#include "stdafx.h"

#include "ZonalSetup.h"
#include "LinearSetup.h"
#include "PlanarSetup.h"

#include "ZonalCollector.h"

#include "utilites/serl/Archive.h"

namespace location
{

static serl::mark_as_rtti<ZonalSetup> INSTANCE;

//////////////////////////////////////////////////////////////////////////

ZonalSetup::ZonalSetup()
    : edtime_(3000, usec)
    , lockout_(0, usec)
    , velocity_(3.0, mm_us)
    , use_peak_time_(true)
{
}

ZonalSetup::ZonalSetup(ZonalSetup const &other)
    : edtime_(other.edtime_)
    , lockout_(other.lockout_)
    , velocity_(other.velocity_)
    , use_peak_time_(other.use_peak_time_)
{
}

void   ZonalSetup::set_definition_time(unit_value ot)
{
    edtime_=ot;
}

void   ZonalSetup::set_lockout(unit_value ot)
{
    lockout_=ot;
}

void   ZonalSetup::set_velocity(unit_value ot)
{
    velocity_=ot;
}

double ZonalSetup::get_definition_time(Unit ot) const
{
    return edtime_.get(ot);
}

double ZonalSetup::get_lockout(Unit ot) const
{
    return lockout_.get(ot);
}

double ZonalSetup::get_velocity(Unit ot) const
{
    return velocity_.get(ot);
}

unit_value ZonalSetup::get_definition_time() const
{
    return edtime_;
}

unit_value ZonalSetup::get_lockout() const
{
    return lockout_;
}

unit_value ZonalSetup::get_velocity() const
{
    return velocity_;
}

bool ZonalSetup::operator!=(const ZonalSetup &rhs) const
{
    return use_peak_time_ != rhs.use_peak_time_
        || edtime_ != rhs.edtime_
        || lockout_ != rhs.lockout_
        || velocity_ != rhs.velocity_;
}

pcollector ZonalSetup::create_collector(unsigned size)
{
    pcollector r( new ZonalCollector(*this) );
    r->setup_and_reserve(size);
    return r;
}

void ZonalSetup::get_coordinate_range(int /*chr*/, double &minimum, double &maximum)
{
    widen_range_begin(minimum, maximum);
}

void ZonalSetup::set_use_peak_time(bool b)
{
    use_peak_time_=b;
}

bool ZonalSetup::get_use_peak_time() const
{
    return use_peak_time_;
}


void ZonalSetup::serialization(serl::archiver &ar)
{
    ar.serial("2edt",  edtime_);
    ar.serial("2loc",  lockout_);
    ar.serial("2vel",  velocity_);

    ar.serial("peak_time",use_peak_time_);
}

std::auto_ptr<SetupImpl> ZonalSetup::clone() const
{
    return std::auto_ptr<SetupImpl>(new ZonalSetup(*this));
}
}

