#include "stdafx.h"
#include "VesselSetup.h"

#include "vesselCollector.h"

#include "Traits.h"

namespace location {

static serl::mark_as_rtti<VesselSetup> INSTANCE;

VesselSetup::VesselSetup()
{
}

void VesselSetup::set_vessel(VesselStructure const& s)
{
    vessel_ = s;
}

VesselStructure const & VesselSetup::get_vessel() const
{
    return vessel_;
}

void VesselSetup::set_sensors(const VesselSensors & s)
{
    sensors_ = s;
}

VesselSensors const& VesselSetup::get_sensors() const
{
    return sensors_;
}

void VesselSetup::get_vessel_m(VesselStructure& vessel) const
{
    vessel.clear();

    for(unsigned index=0; index < vessel_.size(); ++index)
    {
        VesselPart p, src = vessel_[index];
        p.type = src.type;
        p.diameter1 = unit_value(src.diameter1, unit_).to_unit(m, get_velocity().get(m_s));
        p.diameter2 = unit_value(src.diameter2, unit_).to_unit(m, get_velocity().get(m_s));
        p.height    = unit_value(src.height,    unit_).to_unit(m, get_velocity().get(m_s));

        vessel.append(p);
    }

    vessel.compile();
}

void VesselSetup::get_sensors_m(VesselSensors & sensors) const
{
    foreach(VesselCoords crds, sensors_)
    {
        crds.x = unit_value(crds.x, unit_).to_unit(m, get_velocity().get(m_s));
        crds.y_or_z = unit_value(crds.y_or_z, unit_).to_unit(m, get_velocity().get(m_s));
        crds.global.y = unit_value(crds.global.y, unit_).to_unit(m, get_velocity().get(m_s));
        crds.lxy.x = unit_value(crds.lxy.x, unit_).to_unit(m, get_velocity().get(m_s));
        crds.lxy.y = unit_value(crds.lxy.y, unit_).to_unit(m, get_velocity().get(m_s));

        sensors[crds.number] = crds;
    }
}

void VesselSetup::get_coordinate_range(int chr, double &minimum, double &maximum)
{
    int cx = aera::C_CoordX;
    int cy = aera::C_CoordY;
    if (!vessel_.get_is_vertical())
        std::swap(cx, cy);

    if (chr == cy)
    {
        minimum = 0;
        maximum = vessel_.get_height_y();
    } else if (chr == cx)
    {
        minimum = 0;
        maximum = vessel_.get_width_x();
    }
}

void VesselSetup::set_deviation(unit_value d)
{
    deviation_ = d;
}

unit_value VesselSetup::get_deviation() const
{
    return deviation_;
}

void VesselSetup::set_unit(Unit u)
{
    unit_ = u;
}

Unit VesselSetup::get_unit() const
{
    return unit_;
}

bool VesselSetup::operator!=(const VesselSetup &rhs) const
{
    return get_vessel() != rhs.get_vessel()
            || get_sensors() != rhs.get_sensors();
}

pcollector VesselSetup::create_collector(unsigned size)
{
    pcollector r( new VesselCollector(*this) );
    r->setup_and_reserve(size);
    return r;

}

std::auto_ptr<SetupImpl> VesselSetup::clone() const
{
    return std::auto_ptr<SetupImpl>(new VesselSetup(*this));
}

void VesselSetup::serialization(serl::archiver & ar)
{
    ZonalSetup::serialization(ar);

    ar.serial("vessel", vessel_);
    ar.serial("sensors", sensors_);

    if (ar.is_loading())
    {
        foreach(VesselCoords& crds, sensors_)
            crds.lxy = vessel_.to_lxy(crds.global);
    }

    ar.serial("units", serl::makeint(unit_));
    ar.serial("deviation", deviation_);
}

void VesselCoords::serialization(serl::archiver &ar)
{
    ar.serial("gfi", global.fi);
    ar.serial("gy", global.y);
    ar.serial("x", x);
    ar.serial("y", y_or_z);
    ar.serial("coords_type", coords_type);
    ar.serial("base_type", base_type);
    ar.serial("base_index", base_index);
    ar.serial("number", number);
}

bool VesselSensors::operator!=(const VesselSensors &rhs) const
{
    if (size() != rhs.size())
        return true;

    foreach(VesselCoords crds1, *this)
    {
        if (!rhs.contains(crds1.number))
            return true;

        VesselCoords crds2 = rhs[crds1.number];
        if (crds1 != crds2) return true;
    }

    return false;
}

} // namespace location
