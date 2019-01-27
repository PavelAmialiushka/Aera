#include "stdafx.h"

#include "LinearLocationDlg.h"

#include "wtladdons.h"

#include "utilites/Localizator.h"
#include "utilites/foreach.hpp"
#include "utilites/serl/Archive.h"

#include "data/LinearSetup.h"

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

namespace
{
struct widen_coords
{
    int sensor;
    double x;
    double dx;

    bool operator<(widen_coords const& rhs) const
    {
        if (fabs(x - rhs.x) < 1e-4)
            return sensor < rhs.sensor;
        return x < rhs.x;
    }
};
}


class linear_sensor_map
{
public:
    std::vector<widen_coords> sensors_;

    unsigned size() const
    {
        return sensors_.size();
    }

    bool read_from_widget(int pos, int sensor, double x, double dx)
    {
        assert( pos >=0 && (unsigned)pos < sensors_.size() );

        bool result = false;
        widen_coords &curr = sensors_[pos];

        if (fabs(x - curr.x) > 1e-3)
        {
            curr.x = x;
            curr.dx = x - (pos > 0 ? sensors_[pos-1].x : 0);
            for(unsigned index = pos+1; index < size(); ++index)
            {
                sensors_[index].x = sensors_[index-1].x + sensors_[index].dx;
            }
            result = true;
        } else if (fabs(dx - curr.dx) > 1e-3)
        {
            if (pos == 0)
                curr.dx = curr.x = dx;
            else
                curr.dx = dx;

            for(unsigned index = pos; index < sensors_.size(); ++index)
            {
                sensors_[index].x = sensors_[index-1].x + sensors_[index].dx;
            }
            result = true;
        } else
            curr.sensor = sensor;
        return result;
    }

    std::map<int, double> get_map()
    {
        std::map<int, double> result;

        for(unsigned index=0; index < size(); ++index)
            result[sensors_[index].sensor] = sensors_[index].x;

        return result;
    }

    void insertNewRow(int row)
    {
        assert(row >= 0 && (unsigned)row < sensors_.size()+1);
        widen_coords newby;
        if (row == 0)
        {
            newby.sensor =1;
            newby.x = 0;
            newby.dx = 0;
        }
        else
        {
            widen_coords base = sensors_[row-1];
            newby.sensor = base.sensor + 1;
            newby.x = base.x;
            newby.dx = 0;
        }
        sensors_.insert(sensors_.begin() + row, newby);
    }

    void removeRow(int row)
    {
        assert(row >= 0 && (unsigned)row < sensors_.size());
        if (row == 0)
        {
            if (sensors_.size() > 1)
            {
                sensors_[1].dx = sensors_[1].x;
            }
        } else
        {
            if (row != sensors_.size()-1)
            {
                sensors_[row+1].dx += sensors_[row].dx;
            }
        }

        sensors_.erase( sensors_.begin() + row );
    }

    void import(std::map<int, double> sensors)
    {
        sensors_.clear();

        std::vector<std::pair<double, int> > helper_map;
        foreach(LinearSetup::sensors_map_t::value_type pair, sensors)
        {
            helper_map.push_back( std::make_pair(pair.second, pair.first) );
        }
        std::sort(STL_II(helper_map));

        double prev_x = 0;
        typedef std::pair<double,int> pair_type;
        foreach(pair_type pair, helper_map)
        {
            double x = pair.first;
            int sensor = pair.second;

            double dx = 0;
            if (x != 0)
            {
                dx = x - prev_x;
            }

            widen_coords coords;
            coords.sensor = sensor;
            coords.x = x;
            coords.dx = dx;

            prev_x = x;
            sensors_.push_back(coords);
        }
    }
};

//////////////////////////////////////////////////////////////////////////

CLinearLocationDlg::CLinearLocationDlg(LinearSetup *llc)
    : lock_changes_(false)
    , loc_(llc)
    , sensors_map_(new linear_sensor_map)
{
    init(llc);
}

CLinearLocationDlg::~CLinearLocationDlg()
{
}

//////////////////////////////////////////////////////////////////////////

LRESULT CLinearLocationDlg::OnInitDialog(HWND, LPARAM)
{
    _lw(m_hWnd, IDD_LOCATION_LINEAR);
    CenterWindow();

    // схема расположения датчиков
    CRect rc; ::GetWindowRect( GetDlgItem(IDC_INSERT_POINT), rc);
    ScreenToClient(rc);
    sensor_widget_.Create(m_hWnd, rc, NULL, WS_VISIBLE|WS_CHILD);
    CWindow(GetDlgItem(IDC_INSERT_POINT)).ShowWindow(0);

    ImportSensorWidget();

    sensor_grid_.SubclassWindow( GetDlgItem( IDC_LL_CHLIST ) );
    sensor_grid_.InsertColumn(0, _lcs("Channel"), 0, 60, 0);
    sensor_grid_.InsertColumn(1, _lcs("Delta X"), 0, 80, 1);
    sensor_grid_.InsertColumn(2, _lcs("X"), 0, 80, 1);

    SetupMainGrid();
    ImportControlData();

    DlgResize_Init(true, true, 0);

    return 0;
}

LRESULT CLinearLocationDlg::OnCloseCmd(WORD, WORD wID, HWND, BOOL &)
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

void CLinearLocationDlg::DlgResize_UpdateLayout(int cx, int cy)
{
    CDialogResize<CLinearLocationDlg>::DlgResize_UpdateLayout(cx, cy);

    CRect rc; ::GetWindowRect( GetDlgItem(IDC_INSERT_POINT), rc);
    ScreenToClient(rc);
    sensor_widget_.MoveWindow(rc);
}

void CLinearLocationDlg::SetupMainGrid()
{
    CLocationDlgBase<CLinearLocationDlg>::SetupMainGrid();

    grid_.InsertItem(CLM_HEADER, PropCreateHeaderItem( "", _lcs("LL-GridHeader#Linear location") ) );
    grid_.SetSubItem(CLM_HEADER, 1, PropCreateHeaderItem( "" ) );
    grid_.SetSubItem(CLM_HEADER, 2, PropCreateHeaderItem( "" ) );

    grid_.InsertItem(CLM_UNITS, PropCreateReadOnlyItem( "", _lcs("LL-Units#Coordinate units") ) );
    grid_.SetSubItem(CLM_UNITS, 1, PropCreateList("", d_units_.data(), 0));
    grid_.SetSubItem(CLM_UNITS, 2, PropCreateReadOnlyItem( "", "" ) );

    grid_.InsertItem(CLM_CIRCULAR, PropCreateReadOnlyItem( "", _lcs("LL-Circular#Circular length") ) );
    grid_.SetSubItem(CLM_CIRCULAR, 1, PropCreateSimple("", "0.0"));
    grid_.SetSubItem(CLM_CIRCULAR, 2, PropCreateList("", d_units_.data(), 0) );
}

void CLinearLocationDlg::ImportControlData()
{
    ++lock_changes_;
    CLocationDlgBase<CLinearLocationDlg>::ImportControlData();

    // свойство линейной локации
    setUnitValue(CLM_CIRCULAR, loc_->get_circular_length());

    // unit
    HPROPERTY unitProperty = grid_.GetProperty(CLM_UNITS, 1);
    unitProperty->SetValue( CComVariant(unit_2_index_[loc_->get_unit()] ));

    ImportCoordinateGridData();
    --lock_changes_;
}

void CLinearLocationDlg::ImportCoordinateGridData()
{
    std::map<int, double> sensors = loc_->get_sensors();
    sensor_grid_.DeleteAllItems();

    sensors_map_->import(sensors);

    ImportSensorData(-1);
}

void CLinearLocationDlg::ImportSensorData(int index)
{
    if (index == -1)
    {
        for(unsigned index=0; index < sensors_map_->sensors_.size(); ++index)
            ImportSensorData(index);
        return;
    }

    widen_coords coord = sensors_map_->sensors_[index];
    std::string a = strlib::strf("%d", coord.sensor);
    std::string b = strlib::strff("%+.3f", coord.dx);
    std::string c = strlib::strff("%.3f", coord.x);

    if (index >= sensor_grid_.GetItemCount())
    {
        sensor_grid_.InsertItem(index, PropCreateSimple("", a.c_str()));
        sensor_grid_.SetSubItem(index, 1,PropCreateSimple("", b.c_str()));
        sensor_grid_.SetSubItem(index, 2,PropCreateSimple("", c.c_str()));
    } else
    {
        sensor_grid_.SetItemValue(sensor_grid_.GetProperty(index, 0), &CComVariant(a.c_str()));
        sensor_grid_.SetItemValue(sensor_grid_.GetProperty(index, 1), &CComVariant(b.c_str()));
        sensor_grid_.SetItemValue(sensor_grid_.GetProperty(index, 2), &CComVariant(c.c_str()));
    }
}

void CLinearLocationDlg::ImportSensorWidget()
{
    sensor_widget_.setLocation(loc_);
}


void CLinearLocationDlg::ExportControlData()
{
    CLocationDlgBase<CLinearLocationDlg>::ExportControlData();

    unit_value circ = getUnitValue(CLM_CIRCULAR, 0);
    if (circ.unit != none)
        loc_->set_circular_length(circ);

    HPROPERTY unitProperty = grid_.GetProperty(CLM_UNITS, 1);
    CComVariant var; unitProperty->GetValue( &var );
    Unit unit = getUnitByIndex(var.lVal, 0);
    loc_->set_unit(unit);

    ExportSensorGridData();
}

std::string as_string(HPROPERTY prop)
{
    int size = prop->GetDisplayValueLength();
    std::vector<char> buffer(1+size);

    prop->GetDisplayValue(&buffer[0], buffer.size());
    return std::string(&buffer[0], size);
}

void CLinearLocationDlg::ExportSensorData(int index)
{
    HPROPERTY property=sensor_grid_.GetProperty(index, 0);
    int sensor = strlib::read(property, 0);

    LOG(strlib::strf("%d", sensor));

    property=sensor_grid_.GetProperty(index, 1);
    double dx = strlib::read(property, .0);

    property=sensor_grid_.GetProperty(index, 2);
    double x = strlib::read(property, .0);

    widen_coords crds;
    crds.sensor = sensor;
    crds.x = x;
    crds.dx = dx;

    if (sensors_map_->read_from_widget(index, sensor, x, dx))
        for(unsigned jindex=index; jindex < sensors_map_->size(); ++jindex)
            ImportSensorData(jindex);
    else
        ImportSensorData(index);
}

void CLinearLocationDlg::ExportSensorGridData()
{            
    for (int index=0; index<sensor_grid_.GetItemCount(); ++index)
    {
        ExportSensorData(index);
    }

    LinearSetup::sensors_map_t result = sensors_map_->get_map();
    loc_->set_sensors(result);

    ImportSensorWidget();
}

LRESULT CLinearLocationDlg::OnGridItemChanged(LPNMHDR hdr)
{
    if (!lock_changes_)
    {
        ++lock_changes_;

        ExportControlData();

        --lock_changes_;
    }
    return 0;
}

LRESULT CLinearLocationDlg::OnSensorGridItemChanged(LPNMHDR hdr)
{
    LPNMPROPERTYITEM nm = (LPNMPROPERTYITEM)hdr;

    int pos, col;
    sensor_grid_.FindProperty(nm->prop, pos, col);

    if (!lock_changes_)
    {
        ++lock_changes_;

        ExportSensorData(pos);

        --lock_changes_;
    }
    return 0;
}

LRESULT CLinearLocationDlg::OnAddCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    InsertNewLine();
    sensor_grid_.SetFocus();
    return 0;
}

LRESULT CLinearLocationDlg::OnRemoveCommand(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    DeleteLine();
    sensor_grid_.SetFocus();
    return 0;
}


LRESULT CLinearLocationDlg::OnGridKeyDown(LPNMHDR hdr)
{
    LPNMLVKEYDOWN nm=reinterpret_cast<LPNMLVKEYDOWN>(hdr);
    switch (nm->wVKey)
    {
    case VK_INSERT:
        InsertNewLine();
        break;
    case VK_DELETE:
        DeleteLine();
        break;
    }
    return 0;
}

void CLinearLocationDlg::InsertNewLine()
{
    ++lock_changes_;

    int row=sensor_grid_.GetSelectedIndex();
//    if (row<0)
        row=sensor_grid_.GetItemCount();

    sensor_grid_.InsertItem(row, PropCreateSimple("", ""));
    sensor_grid_.SetSubItem(row, 1,  PropCreateSimple("", ""));
    sensor_grid_.SetSubItem(row, 2,  PropCreateSimple("", ""));
    sensor_grid_.SelectItem(row, 0);

    sensors_map_->insertNewRow(row);
    ImportSensorData(-1);

    --lock_changes_;
}

void CLinearLocationDlg::DeleteLine()
{
    ++lock_changes_;

    int row=sensor_grid_.GetSelectedIndex();
    if (row>=0)
    {
        sensor_grid_.DeleteItem(row);
        sensor_grid_.SelectItem(std::min(row, sensor_grid_.GetItemCount()-1));
        sensors_map_->removeRow(row);
        ImportSensorData(-1);

        ExportSensorGridData();
    }

    --lock_changes_;
}


static BOOL CALLBACK SortGridItems(LPARAM i1, LPARAM i2, LPARAM param)
{
    HPROPERTY hSensor1=*reinterpret_cast<HPROPERTY *>(i1);
    HPROPERTY hSensor2=*reinterpret_cast<HPROPERTY *>(i2);

    CPropertyGridCtrl* grid= reinterpret_cast<CPropertyGridCtrl*>(param);
    int i, si;
    grid->FindProperty(hSensor1, i, si);
    si += 2;
    HPROPERTY hXCoord1 = grid->GetProperty(i, si);

    grid->FindProperty(hSensor2, i, si);
    si += 2;
    HPROPERTY hXCoord2 = grid->GetProperty(i, si);

    std::vector<char> sensor1(1+hSensor1->GetDisplayValueLength());
    hSensor1->GetDisplayValue(&sensor1[0], sensor1.size());

    std::vector<char> sensor2(1+hSensor2->GetDisplayValueLength());
    hSensor2->GetDisplayValue(&sensor2[0], sensor2.size());

    std::vector<char> xcoord1(1+hXCoord1->GetDisplayValueLength());
    hXCoord1->GetDisplayValue(&xcoord1[0], xcoord1.size());

    std::vector<char> xcoord2(1+hXCoord2->GetDisplayValueLength());
    hXCoord2->GetDisplayValue(&xcoord2[0], xcoord2.size());

    double x1=atof(&xcoord1[0]);
    double x2=atof(&xcoord2[0]);
    double s1=atof(&sensor1[0]);
    double s2=atof(&sensor2[0]);

    if (fabs(x1 - x2) < 1e-5)
        return s1 < s2 ? -1 :
               s1 > s2 ? +1 : 0;
    else
        return
           x1 < x2 ? -1 :
           x1 > x2 ? +1 :
           0;
}

void CLinearLocationDlg::SortData()
{
    int row=sensor_grid_.GetSelectedIndex();
    int col=sensor_grid_.GetSelectedColumn();
    HPROPERTY hprop=row >=0 ? sensor_grid_.GetProperty(row, 0) : 0;

    sensor_grid_.SortItems(SortGridItems, reinterpret_cast<LPARAM>(&sensor_grid_));

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


LRESULT CLinearLocationDlg::OnGridRClick(LPNMHDR hdr)
{
    LPNMITEMACTIVATE nm=(LPNMITEMACTIVATE)hdr;

    return 0;
}

LRESULT CLinearLocationDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "linloc");
    return 0;
}


