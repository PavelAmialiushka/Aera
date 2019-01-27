#pragma once

#include "Collector.h"
#include "PlanarSetup.h"

#include "ZonalCollector.h"

#include "allocator.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

MAKE_SHARED(planar_clusters);
class planar_clusters;

class PlanarCollector
    : public ZonalCollector
    , public fastmem::allocator
{
public:
    typedef PlanarSetup::sensors_map_t sensors_map_t;

    PlanarCollector(PlanarSetup const &);

public:

    virtual bool calculate_coords(double*&);
    virtual void end_of_file();

    virtual std::vector<aera::chars> get_chars() const;
    virtual std::map<aera::chars, std::string> get_units() const;
    virtual pclusters get_clusters() const;
    virtual void create_minmax_map();


private:

    pplanar_clusters  clusters_;

    double          work_deviation_;
    sensors_map_t   work_channels_;
    Unit            unit_;

    double          vertical_wrap_;
    double          horizontal_wrap_;
};

}
