#pragma once

#include "locationClusters.h"

#include "utilites/hash_map.h"

namespace location
{

MAKE_SHARED(planar_clusters);
class planar_clusters
{
    double minx; double maxx; bool wrapx;
    double miny; double maxy; bool wrapy;

    int rowcount;
    int colcount;

    struct cell {
        int count;
        int in_cluster;
        double energy;


        cell()
            : count(0)
            , in_cluster(0)
            , energy(0)
        {
        }
    };
public:
    struct xy
    {
        int x;
        int y;

        xy() {}
        xy(int x, int y) : x(x), y(y) {}

        bool operator==(xy const& rhs) const
        {
            return x==rhs.x && y==rhs.y;
        }

        size_t hash() const
        {
            return size_t(x * 31 + y);
        }

    };
private:
    typedef hash_map<xy, cell, inner_hash<xy> > map_type;
    map_type map;

    pclusters         result;

public:
    planar_clusters(double minx, double maxx,
                  double miny, double maxy,
                  double horw, double vertv);

    void add(double x, double y, double energy);
    void calculate();

    pclusters get_data() const;

private:
    cell& lvalue(int x, int y);
    cell* rvalue(int x, int y);
};

}
