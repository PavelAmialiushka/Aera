#include "stdafx.h"

#include "LocationSetup.h"

#include "ZonalSetup.h"
#include "LinearSetup.h"
#include "PlanarSetup.h"
#include "VesselSetup.h"

#include "Collector.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/Localizator.h"
#include "utilites/serl/Archive.h"

//////////////////////////////////////////////////////////////////////////
namespace location
{

Setup::Setup()
    : number_(0)
{
}

Setup::Setup(Setup const &other)
    : id_(other.id_)
    , number_(other.number_)
{
    if (other.impl_)
        impl_ = other.impl_->clone();
}

Setup &Setup::operator=(Setup other)
{
    swap(other);
    return *this;
}

void Setup::swap(Setup &other)
{
    impl_.swap(other.impl_);

    std::swap(id_, other.id_);
    std::swap(number_, other.number_);
}

bool Setup::operator==(Setup const &other) const
{
    if (*zonal() != *other.zonal())
        return false;

    if (vessel())
    {
        if (!other.vessel() || *vessel() != *other.vessel())
            return false;
    } else if (planar())
    {
        if (!other.planar() || *planar() != *other.planar())
            return false;
    } else if (linear())
    {
        if (!other.linear() || *linear() != *other.linear())
            return false;
    }
    return true;
}

bool Setup::operator!=(const Setup &other) const
{
    return !operator==(other);
}

void Setup::serialization(serl::archiver &ar)
{
    ar.serial("id", id_);
    ar.serial("no", number_);
    ar.serial_virtual_ptr("impl", impl_);
}

Setup::Setup(SetupImpl *impl)
    : impl_(impl)
    , id_( object_id::create() )
    , number_(0)
{
}

Setup Setup::create(int i)
{
    switch (i)
    {
    case locZonal: return Setup( new ZonalSetup );
    case locLinear: return Setup( new LinearSetup );
    case locPlanar: return Setup( new PlanarSetup );
    case locVessel: return Setup( new VesselSetup );
    default:
        assert(!"Incorrect value");
        throw debug::aera_error("location creation failured", HERE);
    }
}

int Setup::get_number() const
{
    return number_;
}

void Setup::set_number(int n)
{
    number_=n;
}

bool operator==(Setup const &self, object_id const &id)
{
    return self.id_==id;
}

bool operator==(object_id const &id, Setup const &self)
{
    return self.id_==id;
}

object_id Setup::get_id() const
{
    return id_;
}

ZonalSetup *Setup::zonal() const
{
    return dynamic_cast<ZonalSetup*>(impl_.get());
}

LinearSetup *Setup::linear() const
{
    return dynamic_cast<LinearSetup*>(impl_.get());
}

PlanarSetup *Setup::planar() const
{
    return dynamic_cast<PlanarSetup*>(impl_.get());
}

VesselSetup *Setup::vessel() const
{
    return dynamic_cast<VesselSetup*>(impl_.get());
}

pcollector Setup::create_collector(unsigned size)
{
    return impl_->create_collector(size);
}

//////////////////////////////////////////////////////////////////////////

std::string Setup::name() const
{
    std::string result;
    if (VesselSetup* v = vessel())
    {
        result="V%d "+_ls("vessel-location#Vessel location");
    } else if (PlanarSetup* p = planar())
    {
        result="P%d "+_ls("planar-location#Planar location");
    } else if (LinearSetup* p = linear())
    {
        result="L%d "+_ls("linear-location#Linear location");
    } else if (ZonalSetup* p = zonal())
    {
        result="Z%d "+_ls("zonal-location#Zonal location");
    }

    assert(result.find("%d")!=std::string::npos);
    return strlib::strf(result.c_str(), number_);
}

bool Setup::empty() const
{
    return !impl_;
}

}
