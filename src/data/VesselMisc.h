#pragma once

#include "finterpol.h"
#include "kdtree.h"
#include "utilites/strings.h"
#include "utilites/serl/Archive.h"

namespace location {

struct  VesselElement;
typedef shared_ptr<VesselElement> PVElement;

struct VesselSensor;
typedef shared_ptr<VesselSensor> PVSensor;


inline bool is_normal_angle(double a)
{
    return -M_PI < a && a <= M_PI;
}

inline double normal_angle(double a)
{
    return fmod2_half(a, M_PI*2);
}

struct lxy_coords
{
    int     element_index;
    double  x;
    double  y;

    lxy_coords(int index=0, double x=0.0, double y=0.0)
        : element_index(index)
        , x(x), y(y)
    {
    }

    // дл€ юниттестов
    bool operator==(lxy_coords const& rhs) const
    {
        return element_index == rhs.element_index
                && fabs(x - rhs.x) < 1e-6
                && fabs(y - rhs.y) < 1e-6;
    }

    friend std::ostream& operator<<(std::ostream& s, lxy_coords const& self)
    {
        s << "{"
          << self.element_index
          << ", "
          << self.x
          << ", "
          << self.y
          << "}";
        return s;
    }
};

struct lfiy_coords
{
    int    element_index; // номер элемента сосуда
    double fi; // пол€рный угол от центральной оси
    double y; // рассто€ние по наружной поверхности

    explicit lfiy_coords(int index=0, double fi=0.0, double y=0.0)
        : element_index(index)
        , fi(fi), y(y)
    {
    }

    // дл€ юниттестов
    bool operator==(lfiy_coords const& rhs) const
    {
        return element_index == rhs.element_index
                && fabs(fi - rhs.fi) < 1e-6
                && fabs(y - rhs.y) < 1e-6;
    }

    friend std::ostream& operator<<(std::ostream& s, lfiy_coords const& self)
    {
        s << "{"
          << self.element_index
          << ", "
          << self.fi
          << ", "
          << self.y
          << "}";
        return s;
    }
};

struct gfiy_coords
{
    double fi;
    double y;

    explicit gfiy_coords(double fi=0.0, double y=0.0)
        : fi(fi), y(y)
    {}

    // дл€ юниттестов
    bool operator==(gfiy_coords const& rhs) const
    {
        return  fabs(fi - rhs.fi) < 1e-6
                && fabs(y - rhs.y) < 1e-6;
    }

    friend std::ostream& operator<<(std::ostream& s, gfiy_coords const& self)
    {
        s << "{"
          << self.fi
          << ", "
          << self.y
          << "}";
        return s;
    }

    double get_est_delta(gfiy_coords const& rhs, double r = 1, bool ignore_angle=false) const
    {
        double dx = fabs(r * normal_angle(fi - rhs.fi));
        double d = sqrt(
                    (!ignore_angle ? pow(dx, 2) : 0)
                    + pow(fabs(y - rhs.y), 2));
        return d;
    }
};

struct xy_coords
{
};

struct uv_coords
{
    double u;
    double v;
    uv_coords(double u, double v)
        : u(u), v(v)
    {}
};

////////////////////////////////////////////////////////////////////////////////////

struct  VesselContext
{
    bool          polar_mode;

    // polar = false
    lfiy_coords   coords;
    double        azimuth;

    // polar = true
    uv_coords     uvcoords;
    double        polar_azimuth;

    double        distance;
    double        start_azimuth;
    lfiy_coords   start_coords;

    VesselContext(lfiy_coords crds, double az)
        : distance(0)
        , polar_mode(false)
        , uvcoords(0, 0)
        , polar_azimuth(0.0)
        , start_coords(crds)
        , coords(crds)
        , azimuth(az)
        , start_azimuth(az)
    {
    }
};

////////////////////////////////////////////////////////////////////////////////////

struct VesselPosition
{
    gfiy_coords coords;
    double      azimuth;
    double      distance;
    double      start_azimuth;
    int         ray_index;

    VesselPosition()
    {
    }

    VesselPosition(gfiy_coords crds, double a, double d, double sa)
        : coords(crds)
        , azimuth(a)
        , distance(d)
        , start_azimuth(sa)
    {
    }

    // определ€ет угол между азимутом и заданным направлением
    double alpha(double teta) const
    {
        return normal_angle(M_PI/2 - teta - azimuth);
    }

    double get_fi() const { return coords.fi; }
    double get_y() const { return coords.y; }
    double get_distance() const { return distance; }
};

}
