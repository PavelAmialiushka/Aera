#include "stdafx.h"

#include "PlanarSetup.h"

#include "planarCollector.h"

#include "LocationUnit.h"

#include "utilites/serl/Archive.h"
#include "utilites/Localizator.h"
#include "data/traits.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

static serl::mark_as_rtti<PlanarSetup> INSTANCE;

PlanarSetup::PlanarSetup()
    : deviation_(1, m)
    , unit_(m)
    , horizontal_diam_(0, m)
    , vertical_diam_(0, m)
{
}


PlanarSetup::PlanarSetup(PlanarSetup const &other)
    : ZonalSetup(other)
    , deviation_(other.deviation_)
    , horizontal_diam_(other.horizontal_diam_)
    , vertical_diam_(other.vertical_diam_)
    , unit_(other.unit_)
    , sensors_(other.sensors_)

{
}

pcollector PlanarSetup::create_collector(unsigned size)
{
    pcollector r( new PlanarCollector(*this) );
    r->setup_and_reserve(size);
    return r;
}


std::auto_ptr<SetupImpl> PlanarSetup::clone() const
{
    return std::auto_ptr<SetupImpl>(new PlanarSetup(*this));
}

//////////////////////////////////////////////////////////////////////////

void PlanarSetup::set_deviation(unit_value ot)
{
    deviation_=ot;
}

double PlanarSetup::get_deviation(Unit ot) const
{
    return deviation_.get(ot);
}

unit_value PlanarSetup::get_deviation() const
{
    return deviation_;
}

void PlanarSetup::set_horizontal_diam(unit_value uv)
{
    horizontal_diam_ = uv;
}

unit_value PlanarSetup::get_horizontal_diam() const
{
    return horizontal_diam_;
}

void PlanarSetup::set_vertical_diam(unit_value uv)
{
    vertical_diam_ = uv;
}

unit_value PlanarSetup::get_vertical_diam() const
{
    return vertical_diam_;
}


void PlanarSetup::set_sensors(PlanarSetup::sensors_map_t const &cn)
{
    sensors_=cn;
}

PlanarSetup::sensors_map_t PlanarSetup::get_sensors() const
{
    return sensors_;
}

void PlanarSetup::get_coordinate_range(int chr, double &result_min, double &result_max)
{
    planar_coords maximum, minimum;
    widen_range_begin(minimum.x, maximum.x);
    widen_range_begin(minimum.y, maximum.y);

    if (chr == aera::C_CoordX || chr == aera::C_CoordY)
    {
        if (!sensors_.empty())
        {
            typedef std::map<int, planar_coords>::value_type value_type;
            foreach(value_type value, sensors_)
            {
                planar_coords c = value.second;

                widen_range(minimum.x, maximum.x, c.x);
                widen_range(minimum.y, maximum.y, c.y);
            }
        } else
            minimum = maximum = planar_coords();

        double vel = get_velocity().get(m_s);
        if (double v = vertical_diam_.to_unit(unit_, vel))
        {
            widen_range(minimum.x, maximum.x, .0);
            widen_range(minimum.x, maximum.x, v * M_PI);
        }

        if (double h = horizontal_diam_.to_unit(unit_, vel))
        {
            widen_range(minimum.y, maximum.y, .0);
            widen_range(minimum.y, maximum.y, h * M_PI);
        }

        widen_range_end(minimum.x, maximum.x);
        widen_range_end(minimum.y, maximum.y);

    }
    if (chr == aera::C_CoordX)
    {
        result_min = minimum.x;
        result_max = maximum.x;
    } else
    {
        result_min = minimum.y;
        result_max = maximum.y;
    }
}

void PlanarSetup::set_unit(Unit ot)
{
    unit_=ot;
}

Unit PlanarSetup::get_unit() const
{
    return unit_;
}

bool PlanarSetup::operator!=(const PlanarSetup &rhs) const
{
    if (deviation_ != rhs.deviation_
            || horizontal_diam_ != rhs.horizontal_diam_
            || vertical_diam_ != rhs.vertical_diam_
            || unit_ != rhs.unit_
            || sensors_ != rhs.sensors_)
    {
        return true;
    }

    return false;
}

void planar_coords::serialization(serl::archiver& a)
{
    a.serial("x", x);
    a.serial("y", y);
}

bool planar_coords::operator==(const planar_coords &rhs) const
{
    return fabs(x - rhs.x) < 1e-6
            && fabs(y - rhs.y) <1e-6;
}

bool planar_coords::operator<(planar_coords const& rhs) const
{
    return x == rhs.x ? y < rhs.y
                      : x < rhs.x;
}


void PlanarSetup::serialization(serl::archiver &ar)
{
    ZonalSetup::serialization(ar);

    ar.serial("units", serl::makeint(unit_));
    ar.serial("channels", serl::make_container(sensors_));

    ar.serial("deviation", deviation_);
    ar.serial("horizontal_diam", horizontal_diam_);
    ar.serial("vertical_diam", vertical_diam_);
}

}
