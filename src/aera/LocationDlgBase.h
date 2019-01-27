#ifndef LocationDlgBase_H
#define LocationDlgBase_H

#include "resource.h"

#include "others/EditNumber.h"
#include "others/viskoe/PropertyGrid.h"

#include "utilites/Localizator.h"

#include "data/ZonalSetup.h"

#include <map>

using namespace location;

template<class T>
class CLocationDlgBase
{
public:
    struct combo_list
    {
        std::vector<std::string> strings;
        mutable std::vector<const char*> pointers;

        void add(std::string n)
        {
            strings.push_back( n );
        }

        const char** data() const
        {
            pointers.clear();
            foreach(std::string const& item, strings)
                pointers.push_back(item.c_str());
            pointers.push_back(0);
            return &pointers[0];
        }
    };
private:
    combo_list prepare_unit_list(Unit *punit, Unit *pend)
    {
        unsigned index=0;
        combo_list combo;
        for(; punit < pend; ++punit )
        {
            combo.add( get_short_name(*punit) );
            unit_2_index_[*punit] = index++;
        }
        return combo;
    }

protected:
    enum
    {
        CLM_HITTIMEMODE = 0,
        CLM_VELOCITY = 1,
        CLM_DEFINITION = 2,
        CLM_LOCKOUT = 3,

        CLM_NEXT
    };

    CPropertyGridCtrl grid_;

    std::map<Unit, int> unit_2_index_;

    combo_list d_units_;
    combo_list v_units_;

    location::ZonalSetup *loc_;

    void init(location::ZonalSetup * loc)
    {
        loc_ = loc;
    }

    void RegisterUnits()
    {
        //////////////////////////////////////////////////////////////////////////

        Unit distance_units[] = {mm, cm, m, usec, sec};
        d_units_ = prepare_unit_list(STL_AA(distance_units));

        Unit velocity_units[] = {mm_us, m_s};
        v_units_ = prepare_unit_list(STL_AA(velocity_units));

        //////////////////////////////////////////////////////////////////////////
    }

    void SetupMainGrid()
    {
        RegisterUnits();

        grid_.SubclassWindow( static_cast<T*>(this)->GetDlgItem(IDC_ZL_GRID) );

        // настройка таблицы

        CRect grc; grid_.GetWindowRect(&grc);
        unsigned width=grc.Width()-4;

        // колонки
        grid_.InsertColumn(0, _lcs("ZL-name#Name"), LVCFMT_LEFT, width-120, 0);
        grid_.InsertColumn(1, _lcs("ZL-value#Value"), LVCFMT_RIGHT, 65, 1);
        grid_.InsertColumn(2, _lcs("ZL-unit#Unit"), LVCFMT_LEFT, 50, 2);

        // строчки
        grid_.InsertItem(CLM_HITTIMEMODE, PropCreateReadOnlyItem( "", _lcs("ZL-HitTimeMode#Hit time mode") ) );

        std::string buf;
        combo_list timeMode;
        timeMode.add(_lcs("ZL-HitTimeModeFront#front mode"));
        timeMode.add(_lcs("ZL-HitTimeModePeak#peak mode"));
        grid_.SetSubItem(CLM_HITTIMEMODE, 1, PropCreateList("", timeMode.data(), 0));
        grid_.SetSubItem(CLM_HITTIMEMODE, 2, PropCreateReadOnlyItem( "", "" ) );

        grid_.InsertItem(CLM_VELOCITY, PropCreateReadOnlyItem( "", _lcs("ZL-veloc#Velocity") ) );
        grid_.SetSubItem(CLM_VELOCITY, 1, PropCreateSimple("", "0.0"));
        grid_.SetSubItem(CLM_VELOCITY, 2, PropCreateList("", v_units_.data(), 0));

        grid_.InsertItem(CLM_DEFINITION, PropCreateReadOnlyItem( "", _lcs("ZL-deftime#Event definition value") ) );
        grid_.SetSubItem(CLM_DEFINITION, 1, PropCreateSimple("", "0.0"));
        grid_.SetSubItem(CLM_DEFINITION, 2, PropCreateList("", d_units_.data(), 0));

        grid_.InsertItem(CLM_LOCKOUT, PropCreateReadOnlyItem( "", _lcs("ZL-lockout#Event lockout") ) );
        grid_.SetSubItem(CLM_LOCKOUT, 1, PropCreateSimple("", "0.0"));
        grid_.SetSubItem(CLM_LOCKOUT, 2, PropCreateList("", d_units_.data(), 0));
    }

    void setUnitValue(int line_index, unit_value uv)
    {
        HPROPERTY property = grid_.GetProperty(line_index, 1);
        HPROPERTY units = grid_.GetProperty(line_index, 2);
        property->SetValue( CComVariant(strlib::strf("%g", uv.value).c_str()) );
        units->SetValue( CComVariant(unit_2_index_[uv.unit]) );
    }

    void ImportControlData()
    {
        HPROPERTY peak_time_mode = grid_.GetProperty(CLM_HITTIMEMODE, 1);
        peak_time_mode->SetValue( CComVariant(loc_->get_use_peak_time() ? 1 : 0) );

        setUnitValue(CLM_VELOCITY, loc_->get_velocity());
        setUnitValue(CLM_DEFINITION, loc_->get_definition_time());
        setUnitValue(CLM_LOCKOUT, loc_->get_lockout());
    }

    Unit getUnitByIndex(int pos_index, int is_veloc_unit)
    {
        std::vector<Unit> test;
        typedef std::pair<Unit, int> value_type;
        foreach(value_type val, unit_2_index_)
        {
            if (val.second == pos_index )
            {
                test.push_back( val.first );
            }
        }

        Unit result=none;
        foreach(Unit unit, test)
        {
            if (is_veloc_unit && unit_value::type(unit)==VELOCITY)
                result=unit;

            if (!is_veloc_unit && unit_value::type(unit)!=VELOCITY)
                result=unit;
        }
        return result;
    }

    unit_value getUnitValue(int value_index, int is_veloc_unit)
    {
        HPROPERTY property = grid_.GetProperty(value_index, 1);
        HPROPERTY unitProp = grid_.GetProperty(value_index, 2);

        std::vector<char> buffer( property->GetDisplayValueLength()+1 );
        property->GetDisplayValue( &buffer[0], buffer.size() );

        double value;
        try
        {
            value = boost::lexical_cast<double>( std::string(&buffer[0]) );
        }
        catch(boost::bad_lexical_cast&)
        {
            return unit_value(0, none);
        }

        CComVariant var; unitProp->GetValue( &var );
        Unit unit = getUnitByIndex(var.lVal, is_veloc_unit);

        return unit_value(value, unit);
    }

    template<typename U>
    U ReadGridValue(CPropertyGridCtrl &grid, int row, int col, bool* ok = 0, U *u=0 /*signature*/)
    {
        bool success = true;
        U value = U();

        HPROPERTY property=grid.GetProperty(row, col);
        std::vector<char> buffer(property->GetDisplayValueLength()+1);
        property->GetDisplayValue(&buffer[0], buffer.size());

        if (buffer[0]==0) success = false;
        else
        {
            try
            {
                value = boost::lexical_cast<U>(&buffer[0]);
            } catch(boost::bad_lexical_cast&)
            {
                value = U();
                success = false;
            }
        }

        if (ok) *ok = success;
        return value;
    }
    int ReadGridValueInt(CPropertyGridCtrl &grid, int row, int col, bool* ok = 0)
    { return ReadGridValue(grid, row, col, ok, (int*)0); }
    double ReadGridValueDouble(CPropertyGridCtrl &grid, int row, int col, bool* ok = 0)
    { return ReadGridValue(grid, row, col, ok, (double*)0); }

    int ReadEnumValue(CPropertyGridCtrl &grid, int row, int col)
    {
        HPROPERTY unitProp = grid.GetProperty(row, col);
        CComVariant var; unitProp->GetValue( &var );
        return var.lVal;
    }

    void ExportControlData()
    {
        HPROPERTY hprop = grid_.GetProperty(CLM_HITTIMEMODE, 1);
        CComVariant var; hprop->GetValue(&var);
        bool peak = var.lVal == 1;
        loc_->set_use_peak_time( peak );

        unit_value vel=getUnitValue(CLM_VELOCITY, true);
        if (vel.unit != none)
            loc_->set_velocity( vel );

        unit_value def=getUnitValue(CLM_DEFINITION, false);
        if (def.unit != none)
            loc_->set_definition_time( def );

        unit_value loc=getUnitValue(CLM_LOCKOUT, false);
        if (loc.unit != none)
            loc_->set_lockout( loc );

    }

};


#endif // LocationDlgBase_H
