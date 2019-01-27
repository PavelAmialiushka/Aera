#pragma once

#include "utilites/serl/Archive.h"

namespace location
{

enum Unit
{
    mm,
    cm,
    m ,

    usec,
    sec,

    mm_us,
    m_s,

    none,
    dB,
    volt,

    DISTANCE=0, TIME=1, VELOCITY=2,
};

std::string get_short_name(Unit);
Unit unit_from_name(std::string);

struct unit_value : serl::serializable
{
    double        value;
    Unit          unit;

    unit_value()
        : value(0), unit(mm)
    {}

    unit_value(double v, Unit u)
        : value(v), unit(u)
    {}

    static int type(int u)
    {
        switch (Unit(u))
        {
        case mm:
        case cm:
        case m:
            return 0;
        case usec:
        case sec:
            return 1;
        default:
            return 2;
        }
    }

    int type() const
    {
        return type(unit);
    }

    static double factor(Unit u)
    {
        switch (u)
        {
        case mm:   return 1;
        case cm:   return 10;
        case m:    return 1000;
        case usec: return 1;
        case sec:  return 1e6;
        case mm_us:return 1;
        case m_s:  return 0.001;
        case none:
        default:
            return 1;
        }
        assert(!"unknown unit");
        return 0.0;
    }

    double get(Unit ot) const
    {
        assert( (type(unit)==type(ot)) && "incompatible units" );

        return value * factor(unit) / factor(ot);
    }

    // returns [sec]
    // [velocity] = m/s
    double get_time(double velocity)
    {
        unit_value result = *this;
        if (type() == DISTANCE)
        {
            result = velocity
                ? unit_value(get(m) / velocity, sec)
                : unit_value(0, sec);
        }
        return result.get(sec);
    }

    double to_unit(Unit u, double velocity) // [velocity] = m/s
    {
        if (type(u)==type())
            return get(u);
        else if (type() == DISTANCE)
            return unit_value(get(m) / velocity, sec).get(u);
        else
            return unit_value(get(sec) * velocity, m).get(u);
    }

    // returns [m]
    double get_distance(double velocity) // [velocity] = m/s
    {
        unit_value result = *this;
        if (type() == TIME)
        {
            result = unit_value(get(sec) * velocity, m);
        }
        return result.get(m);
    }

    unit_value convert(Unit ot) const
    {
        return unit_value( get(ot), ot );
    }

    void serialization(serl::archiver &ar)
    {
        ar.serial("value", value);
        ar.serial("unit", serl::makeint(unit));
    }

    bool operator==(const unit_value &b) const
    {
        return type()==b.type() && fabs(get( b.unit ) - b.value)<1e-6;
    }

    bool operator!=(const unit_value &b) const
    {
        return !operator==(b);
    }

};

}
