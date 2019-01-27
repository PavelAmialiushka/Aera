#include "stdafx.h"

#include "vesselClusters.h"

namespace location
{

vessel_clusters::vessel_clusters(VesselStructure& vessel)
    : vessel(vessel)
{    
    cell_size= vessel.get_height_y() / 128;
    for(double y = cell_size/2;
        y < vessel.get_height_y() - cell_size/2;
        y += cell_size)
    {
        double c = M_360 * vessel.get_radius_by_gy(y);
        int rays = std::max<int>(1, c / cell_size);

        for(double fi = 0; fi < M_360; fi += M_360 / rays)
        {
            gfiy_coords cx(normal_angle(fi), y);
            int index = values.size();

            cell it;
            it.energy = 0;
            it.count = 0;
            it.in_cluster = 0;
            it.center = cx;
            values[index] = it;

            xyz_coords xx = vessel.to_xyz(cx);
            tree.add(xx, index);
        }
    }
}


void vessel_clusters::add(double fi, double y, double energy)
{
    gfiy_coords cx(normal_angle(fi), y);
    xyz_coords xx = vessel.to_xyz(cx);
    int id = tree.getclosest_id(xx);

    cell& clu = values[id];
    clu.energy += energy;
    clu.count += 1;
}

template<class T>
struct by_energy_t
{
    T *map;
    by_energy_t(T &t) : map(&t) {}
    bool operator()(int lhs, int rhs) const
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

void vessel_clusters::calculate()
{
    result.reset( new clusters );
    std::deque<int> sorted;

    typedef std::map<int, cell>::value_type type;
    foreach(type const& it, values)
    {
        int id = it.first;
        assert(values.count(id));
        if (values[id].count)
        {
            sorted.push_back( it.first );
        }
    }

    std::sort(sorted.begin(), sorted.end(), by_energy(values));

    while(sorted.size())
    {
        cell it = values[sorted[0]];
        sorted.pop_front();
        if (it.in_cluster)
            continue;

        // начинаем сканирование окресностей
        xyz_coords xx0 = vessel.to_xyz(it.center);

        std::vector<int> ids(500);
        int ids_size = tree.getclosest_ids(xx0, &ids[0], ids.size(), cell_size * 10);
        ids.resize(ids_size);

        int left = 8;
        int right = 8;
        int top = 8;
        int bottom = 8;

        // отнимаем, если потеря энергии (по площади) не более
        double energy_limit = 0.05;

        // кластер должен содержать не менее этого значения
        double count_limit = 5;

        double energy = 0;
        int count = 0;

        for(int index=0; index < ids_size; ++index)
        {
            int id = ids[index];
            assert(values.count(id));
            if (values[id].in_cluster || values[id].count==0)
            {
                // занято
                ids.erase(ids.begin() + index);
                --index;
                --ids_size;
            }
            energy += values[id].energy;
            count += values[id].count;
        }

        int progress = 0;
        do {
            progress = 0;
            // посчитываем суммарную энергию в сверху и снизу
            double energy_t = 0; double energy_b = 0;
            int count_t = 0; int count_b = 0;

            for(int index=0; index < ids_size; ++index)
            {
                int id = ids[index];
                xyz_coords xx2 = vessel.to_xyz(values[id].center);

                double teta = 0;
                double d = vessel.local_distance(values[id].center,
                                                 it.center, &teta);

                double y = d * sin(teta);
                int ycell = int(0.5 + y / cell_size);

                if (ycell == -bottom)
                {
                    energy_b += values[id].energy;
                    count_b += values[id].count;
                }
                if (ycell == top)
                {
                    energy_t += values[id].energy;
                    count_t += values[id].count;
                }
            }

            if (energy_t * (top + bottom + 1) < energy * energy_limit
                    && top)
            {
                energy -= energy_t;
                count -= count_t;
                top -= 1;
                ++progress;
            }

            if (energy_b * (top + bottom + 1) < energy * energy_limit
                    && bottom)
            {
                energy -= energy_b;
                count -= count_b;
                bottom -= 1;
                ++progress;
            }

            // посчитываем суммарную энергию в слева и справа

            // энергии слева и справа
            double energy_l = 0; double energy_r = 0;
            int count_l = 0; int count_r = 0;

            for(int index=0; index < ids_size; ++index)
            {
                int id = ids[index];
                xyz_coords xx2 = vessel.to_xyz(values[id].center);

                double teta = 0;
                double d = vessel.local_distance(values[id].center, it.center, &teta);

                double x = d * cos(teta);
                int xcell = int(0.5 + x / cell_size);

                if (xcell == -left)
                {
                    energy_l += values[id].energy;
                    count_l += values[id].count;
                }
                if (xcell == right)
                {
                    energy_r += values[id].energy;
                    count_r += values[id].count;
                }
            }

            if (energy_l * (left + right + 1) < energy * energy_limit
                    && left)
            {
                energy -= energy_l;
                count -= count_l;
                left -= 1;
                ++progress;
            }

            if (energy_r * (left + right + 1) < energy * energy_limit
                    && right)
            {
                energy -= energy_r;
                count -= count_r;
                right -= 1;
                ++progress;
            }

        } while(progress);

        double x0 = vessel.to_nx(it.center);
        double y0 = it.center.y;

        double x1 = x0 - cell_size/2, x2 = x0 + cell_size/2;
        double y1 = y0 - cell_size/2, y2 = y0 + cell_size/2;

        // помечаем все попавшие к нам элементы как принадлежащие кластеру
        count = 0;
        energy = 0;
        for(int index=0; index < ids_size; ++index)
        {
            int id = ids[index];
            double teta = 0;
            double d = vessel.local_distance(values[id].center,
                                             it.center, &teta);

            double x = d * cos(teta);
            double y = d * sin(teta);

            // накидываем на радиус кластера
            x += (x > 0 ? 1 : -1) * cell_size / 2;
            y += (y > 0 ? 1 : -1) * cell_size / 2;

            if ((x < 0
                 ? fabs(x) < cell_size * left
                 : x < cell_size * right )
                &&
                    (y < 0
                     ? fabs(y) < cell_size * bottom
                     : y < cell_size * top))
            {
                values[id].in_cluster = 1;

                count += values[id].count;
                energy += values[id].energy;

                x1 = std::min(x1, x0 + x);
                x2 = std::max(x2, x0 + x);
                y1 = std::min(y1, y0 + y);
                y2 = std::max(y2, y0 + y);

            } else
            {
                // выбрасываем элементы, не попавшие в диапазон
                ids.erase(ids.begin() + index);
                --index;
                --ids_size;
            }
        }

        // определяем размеры кластера
        if (count >= count_limit
                && energy > 100)
        {
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

        std::sort(result->items.begin(), result->items.end(),
                  by_cluster_energy);
        while(result->items.size() > 10)
            result->items.pop_back();
    }
}

pclusters vessel_clusters::get_data() const
{
    return result;
}

}
