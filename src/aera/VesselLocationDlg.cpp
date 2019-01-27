#include "stdafx.h"

#include "VesselLocationDlg.h"

#include "wtladdons.h"

#include "utilites/Localizator.h"
#include "utilites/foreach.hpp"
#include "utilites/serl/Archive.h"

#include "data/VesselSetup.h"

#include <boost/lexical_cast.hpp>

#include "utilites/help.h"

#define _blc boost::lexical_cast

using namespace location;

static int cmp_xchg(int &value, int new_value, int cmp_value)
{
    bool result=value;
    if (value==cmp_value)
        value=new_value;
    return result;
}

//////////////////////////////////////////////////////////////////////////

CVesselLocationDlg::CVesselLocationDlg(VesselSetup *llc)
    : lock_changes_(false)
    , loc_(llc)
{
    init(loc_);
}

CVesselLocationDlg::~CVesselLocationDlg()
{
}


//////////////////////////////////////////////////////////////////////////

LRESULT CVesselLocationDlg::OnInitDialog(HWND, LPARAM)
{
    _lw(m_hWnd, IDD_LOCATION_VESSEL);
    CenterWindow();

    // схема расположения датчиков
    CRect rc; ::GetWindowRect( GetDlgItem(IDC_INSERT_POINT), rc);
    ScreenToClient(rc);
    sensor_widget_.Create(m_hWnd, rc, NULL, WS_VISIBLE|WS_CHILD);
    CWindow(GetDlgItem(IDC_INSERT_POINT)).ShowWindow(0);

    sensor_widget_.setLocation(loc_);

    vessel_grid_.SubclassWindow( GetDlgItem(IDC_VL_PARTS ));
    vessel_grid_.InsertColumn(0, _lcs("No"), 0, 20, 0);
    vessel_grid_.InsertColumn(1, _lcs("Type"), 0, 130, 0);
    vessel_grid_.InsertColumn(2, _lcs("Diam1"), 0, 50, 0);
    vessel_grid_.InsertColumn(3, _lcs("Height"), 0, 50, 0);
    vessel_grid_.InsertColumn(4, _lcs("Diam2"), 0, 50, 0);

    sensor_grid_.SubclassWindow( GetDlgItem( IDC_VL_CHLIST ) );
    sensor_grid_.InsertColumn(0, _lcs("Channel"), 0, 20, 0);
    sensor_grid_.InsertColumn(1, _lcs("Base"), 0, 70, 0);
    sensor_grid_.InsertColumn(2, _lcs("N"), 0, 30, 0);
    sensor_grid_.InsertColumn(3, _lcs("X"), 0, 55, 1);
    sensor_grid_.InsertColumn(4, _lcs("Y"), 0, 55, 1);
    sensor_grid_.InsertColumn(5, _lcs("Type"), 0, 80, 0);

    sensor_placement_type_.add(_ls("element"));
    sensor_placement_type_.add(_ls("sensor"));

    vessel_orientation_.add(_ls("vertical"));
    vessel_orientation_.add(_ls("horizontal"));

    yaxe_type_.add(_ls("surf"));
    yaxe_type_.add(_ls("proj"));

    vessel_parts_.add(_ls("Cylinder"));
    vessel_parts_.add(_ls("Ellipsoid"));
    vessel_parts_.add(_ls("Spheric"));
    vessel_parts_.add(_ls("Conic"));
    vessel_parts_.add(_ls("Flat"));

    SetupMainGrid();
    ImportControlData();

    DlgResize_Init(true, true, 0);

    return 0;
}

void CVesselLocationDlg::DlgResize_UpdateLayout(int cx, int cy)
{
    CDialogResize<CVesselLocationDlg>::DlgResize_UpdateLayout(cx, cy);

    CRect rc; ::GetWindowRect( GetDlgItem(IDC_INSERT_POINT), rc);
    ScreenToClient(rc);
    sensor_widget_.MoveWindow(rc);
}

void CVesselLocationDlg::SetupMainGrid()
{
    CLocationDlgBase<CVesselLocationDlg>::SetupMainGrid();

    grid_.InsertItem(CLM_HEADER, PropCreateHeaderItem( "", _lcs("VL-GridHeader#Vessel location") ) );
    grid_.SetSubItem(CLM_HEADER, 1, PropCreateHeaderItem( "" ) );
    grid_.SetSubItem(CLM_HEADER, 2, PropCreateHeaderItem( "" ) );

    grid_.InsertItem(CLM_UNITS, PropCreateReadOnlyItem( "", _lcs("VL-Units#Coordinate units") ) );
    grid_.SetSubItem(CLM_UNITS, 1, PropCreateList("", d_units_.data(), 0));
    grid_.SetSubItem(CLM_UNITS, 2, PropCreateReadOnlyItem( "", "" ) );

    grid_.InsertItem(CLM_DEVIATION, PropCreateReadOnlyItem( "", _lcs("VL-Deviaton#Deviation limit") ) );
    grid_.SetSubItem(CLM_DEVIATION, 1, PropCreateSimple("", "0.0"));
    grid_.SetSubItem(CLM_DEVIATION, 2, PropCreateList("", d_units_.data(), 0));

    grid_.InsertItem(CLM_VESSELORIENTATION, PropCreateReadOnlyItem( "", _lcs("VL-Orientation#Vessel orientation") ) );
    grid_.SetSubItem(CLM_VESSELORIENTATION, 1, PropCreateList("", vessel_orientation_.data(), 0));
    grid_.SetSubItem(CLM_VESSELORIENTATION, 2, PropCreateReadOnlyItem( "", "" ) );
}

void CVesselLocationDlg::ImportControlData()
{
    // копирует данные из настроек в диалог

    ++lock_changes_;
    CLocationDlgBase<CVesselLocationDlg>::ImportControlData();

    // deviation
    setUnitValue(CLM_DEVIATION, loc_->get_deviation());

    // unit
    HPROPERTY unitProperty = grid_.GetProperty(CLM_UNITS, 1);
    unitProperty->SetValue( CComVariant(unit_2_index_[loc_->get_unit()] ));

    // orientation
    HPROPERTY orientProperty = grid_.GetProperty(CLM_VESSELORIENTATION, 1);
    orientProperty->SetValue( CComVariant(loc_->get_vessel().get_is_vertical() ? 0 : 1) );

    ImportVesselGridData();
    ImportSensorGridData();

    UpdateSensorWidget();
    --lock_changes_;
}


void CVesselLocationDlg::ExportControlData()
{
    CLocationDlgBase<CVesselLocationDlg>::ExportControlData();

    unit_value dev=getUnitValue(CLM_DEVIATION, 0);
    if (dev.unit != none)
        loc_->set_deviation(dev);

    int ix = ReadEnumValue(grid_, CLM_UNITS, 1);
    Unit unit = getUnitByIndex(ix, 0);
    loc_->set_unit(unit);

    int ve = ReadEnumValue(grid_, CLM_VESSELORIENTATION, 1);
    vessel_.set_is_vertical(ve == 0);

    ExportVesselGridData();
    ExportSensorGridData();
}


void CVesselLocationDlg::InsertNewPart()
{
    ++lock_changes_;

    int row=1+vessel_grid_.GetSelectedIndex();
    if (row<0) row=vessel_grid_.GetItemCount();
    vessel_grid_.InsertItem(row, PropCreateReadOnlyItem("", ""));
    vessel_grid_.SetSubItem(row, 1,  PropCreateList("", vessel_parts_.data(), 0));
    vessel_grid_.SetSubItem(row, 2,  PropCreateSimple("", ""));
    vessel_grid_.SetSubItem(row, 3,  PropCreateSimple("", ""));
    vessel_grid_.SetSubItem(row, 4,  PropCreateSimple("", ""));

    ExportVesselGridData();
    UpdateVesselGridData();

    --lock_changes_;
}

void CVesselLocationDlg::DeleteCurrentPart()
{
    ++lock_changes_;

    int row=vessel_grid_.GetSelectedIndex();
    if (row>=0)
    {
        vessel_grid_.DeleteItem(row);
        vessel_grid_.SelectItem(std::min(row, sensor_grid_.GetItemCount()-1));
        ExportControlData();
    }

    --lock_changes_;
}


void CVesselLocationDlg::ExportVesselGridData()
{
    location::VesselStructure local;
    local.set_is_vertical(vessel_.get_is_vertical());

    for (int index=vessel_grid_.GetItemCount();
         index-->0; )
    {
        bool ok = true;

        int type = ReadEnumValue(vessel_grid_, index, 1);

        double d1 = ReadGridValueDouble(vessel_grid_, index, 2, &ok);
        if (!ok) d1 = 0;

        double h = ReadGridValueDouble(vessel_grid_, index, 3, &ok);
        if (!ok) h = 0;

        double d2 = ReadGridValueDouble(vessel_grid_, index, 4, &ok);
        if (!ok) d2 = 0;

        VesselPart part;
        part.type = type;
        part.diameter1 = d1;
        part.diameter2 = d2;
        part.height = h;

        local.append( part );
    }
    local.compile();

    if (loc_->get_vessel() != local)
    {
        vessel_ = local;
        loc_->set_vessel( local );

        // перерисовываем табличку
        UpdateVesselGridData();
        UpdateSensorWidget();
    }
}

void CVesselLocationDlg::UpdateVesselGridData()
{
    int index=vessel_.size()-1;
    int no = 0;
    foreach(VesselPart const& part, vessel_.get_parts())
    {
        std::string d1 = part.diameter1 ? strlib::strff("%.3f", part.diameter1) : "";
        std::string h  = part.height    ? strlib::strff("%.3f", part.height) : "";
        std::string d2 = part.diameter2 ? strlib::strff("%.3f", part.diameter2) : "";

        vessel_grid_.GetProperty(index, 0)->SetValue(CComVariant(no));
        vessel_grid_.GetProperty(index, 1)->SetValue(CComVariant(part.type));
        vessel_grid_.GetProperty(index, 2)->SetValue(CComVariant(d1.c_str()));
        vessel_grid_.GetProperty(index, 3)->SetValue(CComVariant(h.c_str()));
        vessel_grid_.GetProperty(index, 4)->SetValue(CComVariant(d2.c_str()));

        vessel_grid_.SetItemEnabled(
                    vessel_grid_.GetProperty(index, 2), part.has_diameter1());
        vessel_grid_.SetItemEnabled(
                    vessel_grid_.GetProperty(index, 3), part.has_height());
        vessel_grid_.SetItemEnabled(
                    vessel_grid_.GetProperty(index, 4), part.has_diameter2());
        --index; ++no;
    }
}

void CVesselLocationDlg::ImportVesselGridData()
{
    vessel_ = loc_->get_vessel();
    vessel_.compile();

    ++lock_changes_;

    for(int index=0; index<vessel_.size(); ++index)
    {
        if (index >= vessel_grid_.GetItemCount())
        {
            vessel_grid_.InsertItem(index, PropCreateReadOnlyItem("", ""));
            vessel_grid_.SetSubItem(index, 1, PropCreateList("", vessel_parts_.data(), 0));
            vessel_grid_.SetSubItem(index, 2, PropCreateSimple("", ""));
            vessel_grid_.SetSubItem(index, 3, PropCreateSimple("", ""));
            vessel_grid_.SetSubItem(index, 4, PropCreateSimple("", ""));
        }
    }

    UpdateVesselGridData();

    UpdateSensorWidget();

    --lock_changes_;
}

void CVesselLocationDlg::ExportSensorGridData()
{
    VesselSensors result;
    for (int index=0; index<sensor_grid_.GetItemCount(); ++index)
    {
        bool ok = true;

        int channel = ReadGridValueInt(sensor_grid_, index, 0, &ok);
        if (!ok) continue;

        int base_type = ReadEnumValue(sensor_grid_, index, 1);

        int base_no = ReadGridValueInt(sensor_grid_, index, 2, &ok);
        if (!ok) continue;

        double x = ReadGridValueDouble(sensor_grid_, index, 3, &ok);
        if (!ok) continue;

        double y_or_z = ReadGridValueDouble(sensor_grid_, index, 4, &ok);
        if (!ok) continue;

        int coord = ReadEnumValue(sensor_grid_, index, 5);

        VesselCoords crds;
        crds.base_type = base_type == 0 ? VesselCoords::ByElement : VesselCoords::BySensor;
        crds.base_index = base_no;
        crds.x = x;
        crds.y_or_z = y_or_z;
        crds.coords_type = coord;
        crds.number = channel;

        if (crds.base_type == VesselCoords::ByElement)
        {            
            if (crds.coords_type == VesselCoords::Coords_XY)
            {
                lxy_coords lxy(crds.base_index, x, y_or_z);
                crds.global = loc_->get_vessel().to_gfiy(lxy);
            }
            else
            {                
                double y = loc_->get_vessel().lz2ly(base_no, y_or_z);
                lxy_coords lxy(crds.base_index, x, y);
                crds.global = loc_->get_vessel().to_gfiy(lxy);
            }

            bool ok = 0;
            crds.lxy = loc_->get_vessel().to_lxy(crds.global, &ok);
            if (!ok) continue;
        } else
        {
            if (result.contains(base_no))
            {
                VesselCoords base = result[base_no];

                // сначала отсчитываем x
                double r = loc_->get_vessel().get_radius_by_gy(base.global.y);
                double fi = r ? normal_angle((crds.x + base.lxy.x) / r)
                              : 0;

                // затем y
                double gy = base.global.y;
                if (crds.coords_type == VesselCoords::Coords_XY)
                    gy += crds.y_or_z;
                else
                {
                    double gz = loc_->get_vessel().y2z(gy);
                    gy = loc_->get_vessel().z2y(gz + crds.y_or_z);
                }

                crds.global = gfiy_coords(fi, gy);
                bool ok = 0;
                crds.lxy = loc_->get_vessel().to_lxy(crds.global, &ok);
                if (!ok) continue;
            }
        }

        result[channel] = crds;
    }

    if (loc_->get_sensors() != result)
    {
        loc_->set_sensors(result);
        UpdateSensorWidget();
    }
}


void CVesselLocationDlg::ImportSensorGridData()
{
    VesselSensors const& sensors = loc_->get_sensors();
    foreach(VesselCoords const& crds, sensors)
    {
        std::string ch_value = strlib::strf("%d", crds.number);
        std::string base_no = strlib::strf("%d", crds.base_index);
        std::string x_value = strlib::strff("%+.3f", crds.x);
        std::string y_value = strlib::strff("%+.3f", crds.y_or_z);

        bool found=false;
        for (int index=0; index<sensor_grid_.GetItemCount(); ++index)
        {
            std::vector<char> buffer(1024);
            sensor_grid_.GetItemText(index, 0, &buffer[0], buffer.size());

            std::string text(&buffer[0]);
            if (atoi(text.c_str())==crds.number)
            {
                found=true;
                sensor_grid_.GetProperty(index, 1)->SetValue(CComVariant(
                                crds.base_type));

                sensor_grid_.GetProperty(index, 2)->SetValue(CComVariant(
                                 base_no.c_str()));

                sensor_grid_.GetProperty(index, 3)->SetValue(CComVariant(
                                 x_value.c_str()));

                sensor_grid_.GetProperty(index, 4)->SetValue(CComVariant(
                                 y_value.c_str()));

                sensor_grid_.GetProperty(index, 5)->SetValue(CComVariant(
                                crds.coords_type));
                break;
            }
        }

        if (!found)
        {
            sensor_grid_.InsertItem(0,
                             PropCreateSimple("", ch_value.c_str()));
            sensor_grid_.SetSubItem(0, 1,
                             PropCreateList("", sensor_placement_type_.data(),
                                            crds.base_type));
            sensor_grid_.SetSubItem(0, 2,
                             PropCreateSimple("", base_no.c_str()));
            sensor_grid_.SetSubItem(0, 3,
                             PropCreateSimple("", x_value.c_str()));
            sensor_grid_.SetSubItem(0, 4,
                             PropCreateSimple("", y_value.c_str()));
            sensor_grid_.SetSubItem(0, 5,
                             PropCreateList("", yaxe_type_.data(),
                                            crds.coords_type));
        }
    }

    SortSensorGrid();
    UpdateSensorWidget();
}

void CVesselLocationDlg::UpdateSensorWidget()
{
    sensor_widget_.setLocation(loc_);
    sensor_widget_.Invalidate();
}

LRESULT CVesselLocationDlg::OnSensorGridItemChanged(LPNMHDR)
{
    if (!lock_changes_)
    {
        SortSensorGrid();
        ExportSensorGridData();
    }
    return 0;
}

LRESULT CVesselLocationDlg::OnVesselGridItemChanged(LPNMHDR)
{
    if (!lock_changes_)
    {
        ExportVesselGridData();
    }
    return 0;
}

LRESULT CVesselLocationDlg::OnGridItemChanged(LPNMHDR)
{
    if (!lock_changes_)
    {
        ExportControlData();
        UpdateSensorWidget();
    }
    return 0;
}

LRESULT CVesselLocationDlg::OnAddCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    InsertNewSensor();
    sensor_grid_.SetFocus();
    return 0;
}

LRESULT CVesselLocationDlg::OnRemoveCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    DeleteCurrentSensor();
    sensor_grid_.SetFocus();
    return 0;
}

LRESULT CVesselLocationDlg::OnAddPart(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    InsertNewPart();
    vessel_grid_.SetFocus();
    return 0;
}

LRESULT CVesselLocationDlg::OnRemovePart(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    DeleteCurrentPart();
    vessel_grid_.SetFocus();
    return 0;
}

LRESULT CVesselLocationDlg::OnGridKeyDown(LPNMHDR hdr)
{
    LPNMLVKEYDOWN nm=reinterpret_cast<LPNMLVKEYDOWN>(hdr);
    switch (nm->wVKey)
    {
    case VK_INSERT:
        InsertNewSensor();
        break;
    case VK_DELETE:
        DeleteCurrentSensor();
        break;
    }
    return 0;
}

void CVesselLocationDlg::InsertNewSensor()
{
    ++lock_changes_;

    // определяем максимальный номер канала
    int max_channel = 0;
    VesselSensors const& sensors = loc_->get_sensors();
    foreach(VesselCoords const& crds, sensors)
        if (max_channel < crds.number)
            max_channel = crds.number;

    std::string num = strlib::strf("%d", max_channel + 1);

    int row=sensor_grid_.GetSelectedIndex();
    if (row<0) row=sensor_grid_.GetItemCount();
    sensor_grid_.InsertItem(row, PropCreateSimple("", num.c_str()));
    sensor_grid_.SetSubItem(row, 1,  PropCreateList("", sensor_placement_type_.data(), 0));
    sensor_grid_.SetSubItem(row, 2,  PropCreateSimple("", "0"));
    sensor_grid_.SetSubItem(row, 3,  PropCreateSimple("", "0"));
    sensor_grid_.SetSubItem(row, 4,  PropCreateSimple("", "0"));
    sensor_grid_.SetSubItem(row, 5,  PropCreateList("", yaxe_type_.data(), 0));
    sensor_grid_.SelectItem(row, 0);
    SortSensorGrid();
    ExportSensorGridData();

    --lock_changes_;
}

void CVesselLocationDlg::DeleteCurrentSensor()
{
    ++lock_changes_;

    int row=sensor_grid_.GetSelectedIndex();
    if (row>=0)
    {
        sensor_grid_.DeleteItem(row);
        sensor_grid_.SelectItem(std::min(row, sensor_grid_.GetItemCount()-1));
        ExportSensorGridData();
    }

    --lock_changes_;
}

static BOOL CALLBACK SortGridItems(LPARAM i1, LPARAM i2, LPARAM)
{
    HPROPERTY hProp1=*reinterpret_cast<HPROPERTY *>(i1);
    HPROPERTY hProp2=*reinterpret_cast<HPROPERTY *>(i2);

    std::vector<char> buffer(1+hProp1->GetDisplayValueLength());
    hProp1->GetDisplayValue(&buffer[0], buffer.size());

    std::vector<char> buffer2(1+hProp2->GetDisplayValueLength());
    hProp2->GetDisplayValue(&buffer2[0], buffer2.size());

    int a1=atoi(&buffer[0]);
    int a2=atoi(&buffer2[0]);

    return a1 < a2 ? -1 :
           a1 > a2 ? +1 :
           0;
}

void CVesselLocationDlg::SortSensorGrid()
{
    int row=sensor_grid_.GetSelectedIndex();
    int col=sensor_grid_.GetSelectedColumn();
    HPROPERTY hprop=row >=0 ? sensor_grid_.GetProperty(row, 0) : 0;

    sensor_grid_.SortItems(SortGridItems, 0);

    for (int index=0; index<sensor_grid_.GetItemCount(); ++index)
    {
        if (sensor_grid_.GetProperty(index, 0)==hprop)
        {
            sensor_grid_.SelectItem(index!=0 ? 0 : std::min(sensor_grid_.GetItemCount()-1, index+1), 0);
            sensor_grid_.SelectItem(index, col);
            break;
        }
    }
}

LRESULT CVesselLocationDlg::OnGridRClick(LPNMHDR hdr)
{
    LPNMITEMACTIVATE nm=(LPNMITEMACTIVATE)hdr;

    return 0;
}

LRESULT CVesselLocationDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL &)
{
    try
    {
        ExportControlData();

        EndDialog(0);
    }
    catch (boost::bad_lexical_cast)
    {
        SetMsgHandled(true);
        return 1;
    }
    return 0;
}

LRESULT CVesselLocationDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "plaloc");
    return 0;
}
