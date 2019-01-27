#pragma once

#include "LocationSetup.h"
#include "ZonalSetup.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/serl/Archive.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

//////////////////////////////////////////////////////////////////////////

struct planar_coords {
    double x;
    double y;

    planar_coords(double x=0, double y=0)
        : x(x), y(y)
    {
    }

    void serialization(serl::archiver &);
    bool operator==(planar_coords const& rhs) const;
    bool operator<(planar_coords const& rhs) const;
};

class PlanarSetup : public ZonalSetup
{
public:

    PlanarSetup();
    PlanarSetup(PlanarSetup const &other);

public:

    typedef std::map<int, planar_coords> sensors_map_t;

    void       set_deviation(unit_value);
    double     get_deviation(Unit) const;
    unit_value get_deviation() const;

    void       set_horizontal_diam(unit_value);
    unit_value get_horizontal_diam() const;

    void       set_vertical_diam(unit_value);
    unit_value get_vertical_diam() const;

    void set_sensors(sensors_map_t const &);
    sensors_map_t get_sensors() const;

    void  get_coordinate_range(int chr, double &minimum, double &maximum);

    void set_unit(Unit);
    Unit get_unit() const;

    bool operator!=(PlanarSetup const& rhs) const;

public:

    virtual pcollector create_collector(unsigned size);
    virtual std::auto_ptr<SetupImpl> clone() const;
    virtual void serialization(serl::archiver &);

private:

    unit_value deviation_;
    unit_value horizontal_diam_;
    unit_value vertical_diam_;

    Unit          unit_;
    sensors_map_t sensors_;
};

}
