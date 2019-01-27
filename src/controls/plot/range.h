#ifndef ___plotrange__aera_h___
#define ___plotrange__aera_h___

#include <math.h>

namespace plot
{

class range
{
public:

    //////////////////////////////////////////////////////////////////////////

    class marks
    {
    protected:
        double lo;
        double hi;
        double step;
        bool   log_scale;
    public:
        DEFINE_ERROR(fault, "unexpected error");

        marks(double a, double b, double c, bool log=false)
            : lo(a), hi(b), step(c), log_scale(log)
        {
            if (c<=0)
            {
                c=1;
            }
            //debug::Assert<fault>(c>0, HERE);
        }

        std::string operator[](int index) const
        {
            return get_mark_at(index);
        }

        static int round(double val)
        {
            if (val>0) return static_cast<int>(val+0.5);
            else return static_cast<int>(val-0.5);
        }

        int scale(int index, int l, int r)
        {
            return log_scale
                   ? ::scale(log10(get_double_at(index)), log10(lo), log10(hi), l, r)
                   : ::scale(get_double_at(index), lo, hi, l, r);
        }

        std::string get_all_marks() const
        {
            std::string temp;
            for (unsigned index=0; index<size(); ++index)
            {
                temp += get_mark_at(index)+" ";
            }
            return temp;
        }

        double get_double_at(int index) const
        {
            return log_scale
                   ? pow(10., round(log10(lo))+index)
                   :  (lo+step*index) ;
        }

        std::string get_mark_at(int index) const
        {
            return range::to_string(
                       get_double_at(index),
                       log_scale
                       ?  range(lo, hi, log_scale).calculate_prec(index, size())
                       :  int(-floor(log10(step))));
        }

        unsigned size() const
        {
            if (log_scale)
            {
                return round(log10(hi))-round(log10(lo))+1;
            }
            else
            {
                return static_cast<int>((hi-lo)/step)+1;
            }
        }

        unsigned get_max_length() const
        {
            size_t result=0;
            for (unsigned index=0; index<size(); ++index)
                result=std::max(result, operator[](index).length());
            return result;
        }

        bool operator==(const marks &other) const
        {
            return lo==other.lo
                   && hi==other.hi
                   && step==other.step;
        }

        friend std::ostream &operator<<(std::ostream &str, const marks &self)
        {
            return str<<'('<<self.lo<<','<<self.hi<<','<<self.step<<')';
        }
    };

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    //////////////////////////////////////////////////////////////////////////

    range()
    {
        lo = 0;
        hi = 1;
        log_scale = false;
        init();
    }

    enum testing { nocalc } ;

    range(testing, double l, double r)
    {
        lo = std::min(l,r);
        hi = std::max(l,r);
        log_scale = false;
    }

    range(double l, double r, bool log = false)
    {
        lo = std::min(l, r);
        hi = std::max(l, r);
        log_scale = log;

        init();
    }

    void init()
    {
        if (log_scale)
        {
            lo=lo>0 ? pow(10., ::floor(log10(lo))) : 1;
            hi=hi>0 ? pow(10., ::ceil(log10(hi))) : 1;
            if (lo==hi) hi*=10;
            step=1;
        }
        else
        {
            if (lo==hi) hi=lo+1;
            double delta=hi-lo;

            step=pow(10.0, ceil(log10(delta)));

            double dx, ldot, rdot;
            for ( ; ; step/=10.0)
            {
                ldot=floor(double(lo)/step)*step;
                rdot=ceil(double(hi)/step)*step;
                dx=(lo-ldot+rdot-hi)/delta;
                if (dx<0.2) break;
            };

            lo=ldot;
            hi=rdot;
        }
    }

    double get_closest(double val, double lstep, int round=+1)
    {
        return round >= 0
               ? ceil(val/lstep)*lstep
               : floor(val/lstep)*lstep;
    }

    double get_step_precision(int x)
    {
        if (!log_scale)
        {
            //    -3  -2  -1 0 1 2 3   4
            //  1/10 1/5 1/2 1 2 5 10 20 50 100

            if (x>0)
            {
                return pow(10., x/3)
                       *((x%3)==1
                         ? 2
                         : ((x%3)==2 ? 5 : 1));
            }
            else
            {
                return pow(10., x/3)
                       /((x%3)==-1
                         ? 2
                         : ((x%3)==-2 ? 5:1));
            }
        }
        else
        {
            return 1;
        }
    }

    int get_mark_count(int x)
    {
        return get_marks(x).size();
    }

    marks get_marks(int x)
    {
        return log_scale
               ? marks(lo, hi, step, true)
               : marks(get_closest(lo, step/get_step_precision(x),  +1),
                       get_closest(hi, step/get_step_precision(x), -1),
                       step/get_step_precision(x), false);

    }

    double get_left() const
    {
        return lo;
    }

    double get_right() const
    {
        return hi;
    }

    unsigned map_to(double x, int max) const
    {
        if (log_scale)
        {
            if (x<=0) return 0;
            return ::scale(log10(x), log10(lo), log10(hi), 0, max);
        }
        else
        {
            return ::scale(x, lo, hi, 0, max);
        }
    }

    int calculate_prec(unsigned x, unsigned max) const
    {
        if (log_scale)
        {
            double val=pow(10., scale(x, 0u, max, log10(lo), log10(hi)));
            return 1-(int)floor(log10(val));
        }
        else
        {
            return -(int)floor(log10((hi-lo)/max));
        }
    }

    double unmap_double(int x, int max) const
    {
        if (log_scale)
        {
            return pow(10., scale(x, 0, max, log10(lo), log10(hi)));
        }
        else
        {
            return scale(x, 0, max, lo, hi);
        }
    }

    std::string unmap(int x, int max) const
    {
        return to_string(unmap_double(x, max), calculate_prec(x, max));
    }

    static std::string to_string(double value, int prec)
    {
        std::ostringstream str;

        if (value>1e-30 && (value>1E5 || value<1E-3))
        {
            str.precision( 1 );
            str.setf(std::ios::scientific, std::ios::floatfield);
        }
        else
        {
            str.precision( std::max(0, prec) );
            str.setf(std::ios::fixed, std::ios::floatfield);
        }

        str << value;
        return str.str();
    }

    bool operator==(const range &other) const
    {
        return lo==other.lo && hi==other.hi;
    }

    friend std::ostream &operator<<(std::ostream &str, const range &self)
    {
        return str << '(' << self.lo << ", " << self.hi << ')';
    }

public:
    double lo, hi;
    double step;
    bool   log_scale;
};

}

#endif
