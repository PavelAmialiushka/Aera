#include <vector>

#include "CommonIncludes.h"

#pragma once

namespace location
{

struct cluster
{
    struct point
    {
        double x;
        double y;
    };

    double energy;
    int    count;

    void get_rect(double& l, double &t, double &r, double &b) const;
    std::vector<point> points;
};

MAKE_SHARED_STRUCT(clusters);
struct clusters
{
    std::vector<cluster> items;
};

}
