#pragma once

#include "LocationSetup.h"
#include "LocationUnit.h"

#include "Collector.h"

#include "utilites/serl/Archive.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

//////////////////////////////////////////////////////////////////////////

class ZonalSetup : public SetupImpl
{
public:

    ZonalSetup();
    ZonalSetup(ZonalSetup const &other);

public:
    void   set_definition_time(unit_value);
    void   set_lockout(unit_value);
    void   set_velocity(unit_value);
    void   set_use_peak_time(bool);

    double get_definition_time(Unit) const;
    double get_lockout(Unit) const;
    double get_velocity(Unit) const;
    bool   get_use_peak_time() const;

    unit_value get_definition_time() const;
    unit_value get_lockout() const;
    unit_value get_velocity() const;

public:

    bool operator!=(ZonalSetup const& rhs) const;

    virtual pcollector create_collector(unsigned size);

    virtual void  get_coordinate_range(int chr, double &minimum, double &maximum);

    virtual std::auto_ptr<SetupImpl> clone() const;
    virtual void serialization(serl::archiver &);

private:

    bool       use_peak_time_;

    unit_value edtime_;
    unit_value lockout_;
    unit_value velocity_;
};

//////////////////////////////////////////////////////////////////////////

}
