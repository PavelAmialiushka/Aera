#ifndef KDTREE_H
#define KDTREE_H

#include "kdtree_c.h"

namespace location
{

struct xyz_coords
{
    double x, y, z;

    double distance(xyz_coords const& r)
    {
        return sqrt(pow(x-r.x,2)
                    + pow(y-r.y,2)
                    + pow(z-r.z,2));
    }
};

class KDTree
{
    kdtree* self_;

    // запретить копирование
    KDTree(KDTree const&);
    void operator=(KDTree const&);

public:
    KDTree()
    {
        self_ = kd_create(3);
    }

    ~KDTree()
    {
        kd_free(self_);
    }

    void clear()
    {
        kd_clear(self_);
    }

    void add(xyz_coords const& p, int id)
    {
        kd_insert3(self_, p.x, p.y, p.z, (void*)id);
    }

    int getclosest_ids(xyz_coords const& p, int *ids, int count, double range)
    {
        kdres *res = kd_nearest_range3(self_, p.x, p.y, p.z, range);
        if (!res) return 0;

        std::vector<std::pair<double, int> > vec;
        for(unsigned index=0;; ++index)
        {
            if (kd_res_end(res))
            {
                kd_res_free(res);
                break;
            }

            // получаем данные точки
            double pos[3];
            int id = (int)kd_res_item(res, pos);
            kd_res_next(res);

            xyz_coords q = {pos[0], pos[1], pos[2]};
            vec.push_back( std::make_pair(q.distance(p), id));
        }

        std::sort(vec.begin(), vec.end());

        count = std::min<unsigned>(count, vec.size());
        for(unsigned index=0; index < count; ++index)
        {
            ids[index] = vec[index].second;
        }
        return count;
    }

    int getclosest_id(xyz_coords const& p)
    {
        kdres *res = kd_nearest3(self_, p.x, p.y, p.z);
        if (!res) return -1;

        if (!kd_res_end(res))
        {
            return (int)kd_res_item(res, 0);
        }

        kd_res_free(res);
        return -1;
    }

};

}

#endif // KDTREE_H
