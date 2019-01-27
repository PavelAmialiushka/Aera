#include "stdafx.h"

#include "planarCluster.h"

namespace location
{

planar_clusters::planar_clusters(double minx, double maxx,
                             double miny, double maxy,
                             double horw, double vertv)
    : minx(minx), maxx(maxx), miny(miny), maxy(maxy)
    , wrapx(false), wrapy(false)
{
    if (horw)
    {
        wrapy = true;
        miny = 0; maxy = horw;
    }

    if (vertv)
    {
        wrapx = true;
        minx = 0; maxx = vertv;
    }

    colcount = 128; // x
    rowcount = colcount * (maxy - miny) / (maxx - minx);
    map.reserve(rowcount * colcount * 4);
}

void planar_clusters::add(double x, double y, double energy)
{
    int ix = scale(x, minx, maxx, 0, colcount-1);
    int iy = scale(y, miny, maxy, 0, rowcount-1);
    cell& c = lvalue(ix, iy);
    c.count += 1;
    c.energy += energy;
}

pclusters planar_clusters::get_data() const
{
    return result;
}

template<class T>
struct by_energy_t
{
    T *map;
    by_energy_t(T &t) : map(&t) {}
    bool operator()(planar_clusters::xy const& lhs,
                    planar_clusters::xy const& rhs) const
    {
        return (*map)[lhs].energy > (*map)[rhs].energy;
    }
};

template<class T> by_energy_t<T> by_energy(T &map)
{
    return by_energy_t<T>(map);
}

static bool by_cluster_energy(cluster const& lhs, cluster const& rhs)
{
    return lhs.energy > rhs.energy;
}

void planar_clusters::calculate()
{
    result.reset( new clusters );

    std::deque<xy> sorted;
    for(map_type::iterator it = map.begin(); it != map.end(); ++it)
    {
        it->second.in_cluster = 0;
        if (it->second.energy > 1)
            sorted.push_back( it->first );
    }

    std::sort(sorted.begin(), sorted.end(), by_energy(map));


    while(sorted.size())
    {
#if 0
        double c = map.collision_factor();
        int sz = map.size();
        int sz2 = sorted.size();
#endif

        xy it= sorted[0];
        int x = it.x;
        int y = it.y;
        sorted.pop_front();

        if (map[it].in_cluster)
            continue;

        int left   = 8;
        int right  = 8;
        int top    = 8;
        int bottom = 8;

        // определяем суммарную энергию и количество всего региона
        double energy = 0;
        int    count = 0;
        for(int xx = x - left; xx <= x + right; ++xx)
        {
            for(int yy = y - bottom; yy <= y + top; ++yy)
            {
                cell const* c = rvalue(xx, yy);
                if (c && !c->in_cluster)
                {
                    energy += c->energy;
                    count += c->count;
                }
            }
        }

        // отнимаем, если потеря энергии (по площади) не более
        double energy_limit = 0.05;

        // кластер должен содержать не менее этого значения
        double count_limit = 5;

        int step = 0;
        do {
            step = 0;

            double energy_t = 0; double energy_b = 0;
            int count_t = 0; int count_b = 0;
            for(int xx = x - left; xx <= x + right; ++xx)
            {
                cell const* c1 = rvalue(xx, y+top);
                if (c1 && !c1->in_cluster)
                {
                    energy_t += c1->energy;
                    count_t += c1->count;
                }

                cell const* c2 = rvalue(xx, y-bottom);
                if (c2 && !c2->in_cluster)
                {
                    energy_b += c2->energy;
                    count_b += c2->count;
                }
            }

            if (energy_t * (top + bottom + 1) < energy * energy_limit
                    && top)
            {
                top -= 1;
                energy -= energy_t;
                count -= count_t;
                ++step;
            }
            if (energy_b * (top + bottom + 1) < energy * energy_limit
                    && bottom)
            {
                bottom -= 1;
                energy -= energy_b;
                count -= count_b;
                ++step;
            }

            double energy_r = 0; double energy_l = 0;
            int count_r = 0; int count_l = 0;
            for(int yy = y - bottom; yy <= y + top; ++yy)
            {
                cell const* c1 = rvalue(x+right, yy);
                if (c1 && !c1->in_cluster)
                {
                    energy_r += c1->energy;
                    count_r += c1->count;
                }

                cell const* c2 = rvalue(x-left, yy);
                if (c2 && !c2->in_cluster)
                {
                    energy_l += c2->energy;
                    count_l += c2->count;
                }
            }
            if (energy_l * (left + right + 1) < energy * energy_limit
                    && left)
            {
                left -= 1;
                energy -= energy_l;
                count -= count_l;
                ++step;
            }
            if (energy_r * (left + right + 1) < energy * energy_limit
                    && right)
            {
                right -= 1;
                energy -= energy_r;
                count -= count_r;
                ++step;
            }
        } while(step);

        // определяем кластер и записываем его
        for(int xx = x - left; xx <= x + right; ++xx)
        {
            for(int yy = y - bottom; yy <= y + top; ++yy)
            {
                cell* c = rvalue(xx,yy);
                if (c) c->in_cluster = 1;
            }
        }

        if (count > count_limit
                && energy > 100)
        {
            double x1 = scale(x - left, 0, colcount-1, minx, maxx);
            double x2 = scale(x + right + 1, 0, colcount-1, minx, maxx);
            double y1 = scale(y + top + 1, 0, rowcount-1, miny, maxy);
            double y2 = scale(y - bottom, 0, rowcount-1, miny, maxy);

            cluster item;
            cluster::point a = {x1, y1};
            cluster::point b = {x1, y2};
            cluster::point c = {x2, y2};
            cluster::point d = {x2, y1};
            item.points.push_back(a);
            item.points.push_back(b);
            item.points.push_back(c);
            item.points.push_back(d);
            item.energy = energy;
            item.count = count;
            result->items.push_back( item );
        }
    }

    std::sort(result->items.begin(), result->items.end(),
              by_cluster_energy);
    while(result->items.size() > 10)
        result->items.pop_back();
}

planar_clusters::cell &planar_clusters::lvalue(int x, int y)
{
    if (wrapx) x %= colcount;
    if (wrapy) y %= rowcount;

    return map[xy(x,y)];
}

planar_clusters::cell *planar_clusters::rvalue(int x, int y)
{
    if (wrapx) x %= colcount;
    if (wrapy) y %= rowcount;

    map_type::iterator it = map.find(xy(x,y));
    if (it == map.end())
        return 0;
    return &it->second;
}


}
