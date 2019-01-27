#ifndef FINTERPOL_H
#define FINTERPOL_H

#include <map>
#include <limits>

class finterpol
{
    std::map<double, double> cache;
public:
    finterpol()
    {
    }

    void addValue(double x, double y)
    {
        cache[x] = y;
    }

    double operator()(double fi) const
    {
        std::map<double, double>::const_iterator next,
                iter = cache.lower_bound(fi);

        if (iter == cache.end())
        {
            if (iter == cache.begin())
            {
                assert(!"empty map");
                return 0.0;
            }
            iter = cache.end();
            return (--iter)->second;
        }

        if (iter == cache.end())
        {
            iter = cache.end();
            if (iter == cache.begin())
                return std::numeric_limits<double>::quiet_NaN();
            --iter;
            return iter->second;
        }

        if (iter == cache.begin())
        {
            return iter->second;
        } else
        {
            next = iter; --iter;
            return scale(fi, iter->first, next->first,
                         iter->second, next->second);
        }
    }
};

#endif // FINTERPOL_H
