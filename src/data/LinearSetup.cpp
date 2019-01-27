#include "stdafx.h"

#include "LinearSetup.h"

#include "traits.h"
#include "PlanarSetup.h"
#include "LinearCollector.h"

#include "LocationUnit.h"

#include "utilites/serl/Archive.h"
#include "utilites/Localizator.h"
//////////////////////////////////////////////////////////////////////////

namespace location
{

static serl::mark_as_rtti<LinearSetup> INSTANCE;

LinearSetup::LinearSetup()
    : deviation_(0, usec)
    , unit_(m)
{
}


LinearSetup::LinearSetup(LinearSetup const &other)
    : ZonalSetup(other)
    , deviation_(other.deviation_)
    , unit_(other.unit_)
    , sensors_(other.sensors_)
    , circular_length_(other.circular_length_)
{
}

std::auto_ptr<SetupImpl> LinearSetup::clone() const
{
    return std::auto_ptr<SetupImpl>(new LinearSetup(*this));
}

//////////////////////////////////////////////////////////////////////////

void LinearSetup::set_deviation(unit_value ot)
{
    deviation_=ot;
}

double LinearSetup::get_deviation(Unit ot) const
{
    return deviation_.get(ot);
}

unit_value LinearSetup::get_deviation() const
{
    return deviation_;
}

void LinearSetup::set_circular_length(unit_value uv)
{
    circular_length_ = uv;
}

unit_value LinearSetup::get_circular_length() const
{
    return circular_length_;
}

void LinearSetup::set_sensors(std::map<int, double> const &cn)
{
    sensors_=cn;
}

std::map<int, double> LinearSetup::get_sensors() const
{
    return sensors_;
}

void LinearSetup::set_unit(Unit ot)
{
    unit_=ot;
}

Unit LinearSetup::get_unit() const
{
    return unit_;
}

bool LinearSetup::operator!=(const LinearSetup &rhs) const
{
    if (deviation_ != rhs.deviation_
            || unit_ != rhs.unit_
            || circular_length_ != rhs.circular_length_
            || sensors_.size() != rhs.sensors_.size())
    {
        return true;
    }


    typedef std::map<int, double>::value_type value_type;
    foreach(value_type const& pair, sensors_)
    {
        int channel = pair.first;

        if (fabs(sensors_.find(channel)->second
                 - rhs.sensors_.find(channel)->second) > 1e-6)
            return true;
    }

    return false;
}

pcollector LinearSetup::create_collector(unsigned size)
{
    pcollector r( new LinearCollector(*this) );
    r->setup_and_reserve(size);
    return r;
}

void LinearSetup::get_coordinate_range(int ch, double &minimum, double &maximum)
{
    if (ch == aera::C_CoordX)
    {
        widen_range_begin(minimum, maximum);

        if (circular_length_.value != 0)
        {
            double c = circular_length_.to_unit(
                        unit_, get_velocity().get(m_s));
            widen_range(minimum, maximum, c);
        }

        if (!sensors_.empty())
        {
            typedef std::map<int, double>::value_type value_type;
            foreach(value_type value, sensors_)
            {
                double coord = value.second;
                widen_range(minimum, maximum, coord);
            }
        }

        widen_range_end(minimum, maximum);
    }
}

void LinearSetup::serialization(serl::archiver &ar)
{
    ZonalSetup::serialization(ar);

    ar.serial("units", serl::makeint(unit_));
    ar.serial("channels", serl::make_container(sensors_));
    ar.serial("circular", circular_length_);
}

std::string get_short_name(Unit unit)
{
    return
        unit==m  ? _ls("m") :
        unit==mm ? _ls("mm") :
        unit==cm ? _ls("cm") :
        unit==sec ? _ls("sec") :
        unit==usec ? _ls("usec") :
        unit==mm_us ? _ls("mm/usec") :
        unit==m_s ? _ls("m/sec") :
        unit==none ? "" :
        unit==dB ? _ls("dB") :
        unit==volt ? _ls("V") :

        ( assert(!"incorrect unit") ,"");
}

Unit unit_from_name(std::string name)
{
    for (unsigned index=mm; index<m_s; ++index)
    {
        if (get_short_name(static_cast<Unit>(index))==name)
            return static_cast<Unit>(index);
    }
    return assert(!"incorrect unit"), mm;
}

}
