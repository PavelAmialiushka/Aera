#pragma once

#include "Collector.h"
#include "VesselStructure.h"
#include "VesselSetup.h"

#include "vesselClusters.h"
#include "ZonalCollector.h"

#include "allocator.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

class VesselCollectorCache
        : public CollectorCache
{
public:
    VesselCollectorCache(VesselSetup const &);

    int  init_and_get_size();
    bool process_step();

    PVSensor select_sensor(int);

    std::map<int, PVSensor> getSensors() const;

private:
    VesselStructure vessel;
    VesselSensors   sensors;    
    std::map<int, PVSensor> working_sensors_;
};


class VesselCollector
    : public ZonalCollector
    , public fastmem::allocator
{
public:
    VesselCollector(VesselSetup const &);
    virtual void apply_cache(CollectorCache* cache);

public:
    virtual std::vector<aera::chars> get_chars() const;
    virtual std::map<aera::chars, std::string> get_units() const;
    virtual bool calculate_coords(double*&);
    virtual void create_minmax_map();

    virtual void end_of_file();
    virtual pclusters get_clusters() const;

private:

    pvessel_clusters  clusters_;

    std::map<int, PVSensor> working_sensors_;
    double          work_deviation_;
    VesselStructure vessel_;
    Unit            unit_;

};

}
