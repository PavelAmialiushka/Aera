#pragma once

#include "LocationSetup.h"
#include "ZonalSetup.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/serl/Archive.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

//////////////////////////////////////////////////////////////////////////

class LinearSetup : public ZonalSetup
{
public:
    LinearSetup();
    LinearSetup(LinearSetup const &other);

public:
    typedef std::map<int, double> sensors_map_t;

    void       set_deviation(unit_value);
    double     get_deviation(Unit) const;
    unit_value get_deviation() const;

    void set_sensors(sensors_map_t const &);
    sensors_map_t get_sensors() const;

    void       set_circular_length(unit_value);
    unit_value get_circular_length() const;

    void set_unit(Unit);
    Unit get_unit() const;

    bool operator!=(LinearSetup const& rhs) const;

public:
    virtual pcollector create_collector(unsigned size);
    virtual void get_coordinate_range(int chr, double &minimum, double &maximum);

    virtual std::auto_ptr<SetupImpl> clone() const;
    virtual void serialization(serl::archiver &);

private:

    unit_value    deviation_;
    Unit          unit_;
    sensors_map_t sensors_;
    unit_value    circular_length_;
};

}
