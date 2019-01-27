#include "stdafx.h"

#include "PlanarLocationDlg.h"

#include "wtladdons.h"

#include "utilites/Localizator.h"
#include "utilites/foreach.hpp"
#include "utilites/serl/Archive.h"

#include "data/PlanarSetup.h"

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

CPlanarLocationDlg::CPlanarLocationDlg(PlanarSetup *llc)
    : lock_changes_(false)
    , loc_(llc)
{
    init(loc_);
}

CPlanarLocationDlg::~CPlanarLocationDlg()
{
}


//////////////////////////////////////////////////////////////////////////

LRESULT CPlanarLocationDlg::OnInitDialog(HWND, LPARAM)
{
    _lw(m_hWnd, IDD_LOCATION_PLANAR);
    CenterWindow();

    // схема расположения датчиков
    CRect rc; ::GetWindowRect( GetDlgItem(IDC_INSERT_POINT), rc);
    ScreenToClient(rc);
    sensor_widget_.Create(m_hWnd, rc, NULL, WS_VISIBLE|WS_CHILD);
    CWindow(GetDlgItem(IDC_INSERT_POINT)).ShowWindow(0);

    std::map<int, planar_coords> result;
    result=loc_->get_sensors();
    sensor_widget_.setLocation(loc_);

    sensor_grid_.SubclassWindow( GetDlgItem( IDC_LL_CHLIST ) );
    sensor_grid_.InsertColumn(0, _lcs("Channel"), 0, 60, 0);
    sensor_grid_.InsertColumn(1, _lcs("X"), 0, 100, 1);
    sensor_grid_.InsertColumn(2, _lcs("Y"), 0, 100, 1);

    SetupMainGrid();
    ImportControlData();

    DlgResize_Init(true, true, 0);

    return 0;
}

void CPlanarLocationDlg::DlgResize_UpdateLayout(int cx, int cy)
{
    CDialogResize<CPlanarLocationDlg>::DlgResize_UpdateLayout(cx, cy);

    CRect rc; ::GetWindowRect( GetDlgItem(IDC_INSERT_POINT), rc);
    ScreenToClient(rc);
    sensor_widget_.MoveWindow(rc);
}

void CPlanarLocationDlg::SetupMainGrid()
{
    CLocationDlgBase<CPlanarLocationDlg>::SetupMainGrid();

    grid_.InsertItem(CLM_HEADER, PropCreateHeaderItem( "", _lcs("PL-GridHeader#Planar location") ) );
    grid_.SetSubItem(CLM_HEADER, 1, PropCreateHeaderItem( "" ) );
    grid_.SetSubItem(CLM_HEADER, 2, PropCreateHeaderItem( "" ) );

    grid_.InsertItem(CLM_UNITS, PropCreateReadOnlyItem( "", _lcs("PL-Units#Coordinate units") ) );
    grid_.SetSubItem(CLM_UNITS, 1, PropCreateList("", d_units_.data(), 0));
    grid_.SetSubItem(CLM_UNITS, 2, PropCreateReadOnlyItem( "", "" ) );

    grid_.InsertItem(CLM_DEVIATION, PropCreateReadOnlyItem( "", _lcs("PL-Deviaton#Deviation limit") ) );
    grid_.SetSubItem(CLM_DEVIATION, 1, PropCreateSimple("", "0.0"));
    grid_.SetSubItem(CLM_DEVIATION, 2, PropCreateList("", d_units_.data(), 0));

    grid_.InsertItem(CLM_HORVESSEL, PropCreateReadOnlyItem( "", _lcs("PL-HorDiameter#Horizontal vessel diameter") ) );
    grid_.SetSubItem(CLM_HORVESSEL, 1, PropCreateSimple("", "0.0"));
    grid_.SetSubItem(CLM_HORVESSEL, 2, PropCreateList("", d_units_.data(), 0));

    grid_.InsertItem(CLM_VERTVESSEL, PropCreateReadOnlyItem( "", _lcs("PL-VertDiameter#Vertical vessel diameter") ) );
    grid_.SetSubItem(CLM_VERTVESSEL, 1, PropCreateSimple("", "0.0"));
    grid_.SetSubItem(CLM_VERTVESSEL, 2, PropCreateList("", d_units_.data(), 0));
}

void CPlanarLocationDlg::ImportControlData()
{
    // копирует данные из настроек в диалог

    ++lock_changes_;
    CLocationDlgBase<CPlanarLocationDlg>::ImportControlData();

    // свойства планарной локации
    setUnitValue(CLM_DEVIATION, loc_->get_deviation());
    setUnitValue(CLM_HORVESSEL, loc_->get_horizontal_diam());
    setUnitValue(CLM_VERTVESSEL, loc_->get_vertical_diam());

    // unit
    HPROPERTY unitProperty = grid_.GetProperty(CLM_UNITS, 1);
    unitProperty->SetValue( CComVariant(unit_2_index_[loc_->get_unit()] ));

    ImportCoordinateGridData();
    UpdateSensorWidget();
    --lock_changes_;
}

void CPlanarLocationDlg::ImportCoordinateGridData()
{
    std::map<int, planar_coords> sensors = loc_->get_sensors();

    typedef std::map<int, planar_coords>::value_type value_type;
    foreach(value_type pair, sensors)
    {
        std::string ch_value = strlib::strf("%d", pair.first);
        std::string x_value = strlib::strff("%f", pair.second.x);
        std::string y_value = strlib::strff("%f", pair.second.y);

        bool found=false;
        for (int index=0; index<sensor_grid_.GetItemCount(); ++index)
        {
            std::vector<char> buffer(1024);
            sensor_grid_.GetItemText(index, 0, &buffer[0], buffer.size());

            std::string text(&buffer[0]);
            if (atoi(text.c_str())==pair.first)
            {
                found=true;

                sensor_grid_.SetSubItem(index, 1,
                                 PropCreateSimple("", x_value.c_str(), 0));

                sensor_grid_.SetSubItem(index, 2,
                                 PropCreateSimple("", y_value.c_str(), 0));
                break;
            }
        }

        if (!found)
        {
            sensor_grid_.InsertItem(0,
                             PropCreateSimple("", ch_value.c_str()));
            sensor_grid_.SetSubItem(0, 1,
                             PropCreateSimple("", x_value.c_str()));
            sensor_grid_.SetSubItem(0, 2,
                             PropCreateSimple("", y_value.c_str()));
        }
    }

    SortSensorGrid();
}

void CPlanarLocationDlg::ExportControlData()
{
    CLocationDlgBase<CPlanarLocationDlg>::ExportControlData();

    unit_value dev=getUnitValue(CLM_DEVIATION, 0);
    if (dev.unit != none)
        loc_->set_deviation(dev);

    unit_value hor=getUnitValue(CLM_HORVESSEL, 0);
    if (hor.unit != none)
        loc_->set_horizontal_diam(hor);

    unit_value ver=getUnitValue(CLM_VERTVESSEL, 0);
    if (ver.unit != none)
        loc_->set_vertical_diam(ver);

    HPROPERTY unitProperty = grid_.GetProperty(CLM_UNITS, 1);
    CComVariant var; unitProperty->GetValue( &var );
    Unit unit = getUnitByIndex(var.lVal, 0);
    loc_->set_unit(unit);

    ExportCoordinateGridData();
}

void CPlanarLocationDlg::ExportCoordinateGridData()
{
    std::map<int, planar_coords> result;
    for (int index=0; index<sensor_grid_.GetItemCount(); ++index)
    {
        try
        {
            HPROPERTY property=sensor_grid_.GetProperty(index, 0);
            std::vector<char> b1(property->GetDisplayValueLength()+1);
            property->GetDisplayValue(&b1[0], b1.size());
            if (b1[0]==0) continue;
            int channel=_blc<int>(&b1[0]);

            property=sensor_grid_.GetProperty(index, 1);
            std::vector<char> b2(property->GetDisplayValueLength()+1);
            property->GetDisplayValue(&b2[0], b2.size());
            if (b2[0]==0) continue;
            double x=_blc<double>(&b2[0]);

            property=sensor_grid_.GetProperty(index, 2);
            std::vector<char> b3(property->GetDisplayValueLength()+1);
            property->GetDisplayValue(&b3[0], b3.size());
            if (b3[0]==0) continue;
            double y=_blc<double>(&b3[0]);

            result[channel].x=x;
            result[channel].y=y;
        }
        catch (boost::bad_lexical_cast &) {}
    }

    loc_->set_sensors(result);
    UpdateSensorWidget();
}

void CPlanarLocationDlg::UpdateSensorWidget()
{
    sensor_widget_.setLocation(loc_);
    sensor_widget_.Invalidate();
}

LRESULT CPlanarLocationDlg::OnSensorGridItemChanged(LPNMHDR)
{
    if (!lock_changes_)
    {
        ExportControlData();
        SortSensorGrid();
    }
    return 0;
}

LRESULT CPlanarLocationDlg::OnGridItemChanged(LPNMHDR)
{
    if (!lock_changes_)
    {
        ExportControlData();
        UpdateSensorWidget();
    }
    return 0;
}

LRESULT CPlanarLocationDlg::OnAddCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    InsertNewLine();
    sensor_grid_.SetFocus();
    return 0;
}

LRESULT CPlanarLocationDlg::OnRemoveCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    DeleteCurrentLine();
    sensor_grid_.SetFocus();
    return 0;
}

LRESULT CPlanarLocationDlg::OnGridKeyDown(LPNMHDR hdr)
{
    LPNMLVKEYDOWN nm=reinterpret_cast<LPNMLVKEYDOWN>(hdr);
    switch (nm->wVKey)
    {
    case VK_INSERT:
        InsertNewLine();
        break;
    case VK_DELETE:
        DeleteCurrentLine();
        break;
    }
    return 0;
}

void CPlanarLocationDlg::InsertNewLine()
{
    ++lock_changes_;

    int row=sensor_grid_.GetSelectedIndex();
    if (row<0) row=sensor_grid_.GetItemCount();
    sensor_grid_.InsertItem(row, PropCreateSimple("", ""));
    sensor_grid_.SetSubItem(row, 1,  PropCreateSimple("", ""));
    sensor_grid_.SetSubItem(row, 2,  PropCreateSimple("", ""));
    sensor_grid_.SelectItem(row, 0);
    ExportControlData();

    --lock_changes_;
}

void CPlanarLocationDlg::DeleteCurrentLine()
{
    ++lock_changes_;

    int row=sensor_grid_.GetSelectedIndex();
    if (row>=0)
    {
        sensor_grid_.DeleteItem(row);
        sensor_grid_.SelectItem(std::min(row, sensor_grid_.GetItemCount()-1));
        ExportControlData();
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

void CPlanarLocationDlg::SortSensorGrid()
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

LRESULT CPlanarLocationDlg::OnGridRClick(LPNMHDR hdr)
{
    LPNMITEMACTIVATE nm=(LPNMITEMACTIVATE)hdr;

    return 0;
}

LRESULT CPlanarLocationDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL &)
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

LRESULT CPlanarLocationDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "plaloc");
    return 0;
}
