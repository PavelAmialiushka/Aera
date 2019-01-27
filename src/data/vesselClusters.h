#pragma once


#include "locationClusters.h"
#include "VesselStructure.h"

namespace location
{

MAKE_SHARED(vessel_clusters);

class vessel_clusters
{
    VesselStructure &vessel;
    KDTree           tree;
    double           cell_size;

    struct cell
    {
        int count;
        int in_cluster;
        double energy;
        gfiy_coords center;
    };
    std::map<int, cell> values;
    pclusters         result;

public:
    vessel_clusters(VesselStructure& vessel);

    void add(double fi, double y, double energy);
    void calculate();

    pclusters get_data() const;
};

}
