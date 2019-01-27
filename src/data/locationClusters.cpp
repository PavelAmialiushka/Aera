#include "stdafx.h"

#include "locationClusters.h"

#include "utilites/foreach.hpp"

namespace location
{

void cluster::get_rect(double &l, double &t, double &r, double &b) const
{
    if (points.empty()) return;
    l = r = points[0].x;
    t = b = points[0].y;
    foreach(cluster::point const& pt, points)
    {
        l = std::min(l, pt.x);
        r = std::max(r, pt.x);
        t = std::max(t, pt.y);
        b = std::min(b, pt.y);
    }
}

}
