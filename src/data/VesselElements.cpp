#include "stdafx.h"

#include "utilites/foreach.hpp"

#include "VesselElements.h"
#include "VesselStructure.h"

namespace location
{

VesselElement::VesselElement(double h)
    : height_z(h)
{
}

PVElement VesselElement::make_cone(double h, double d, double d2)
{
    return PVElement( new VesselElementCone(h, d, d2));
}

PVElement VesselElement::make_sphere(double d1, double d2)
{
    double h = std::max(d1, d2) / 2;
    return PVElement( new VesselElementEllipsoid(h, d1, d2));
}

PVElement VesselElement::make_ellipsoid(double h, double d1, double d2)
{
    return PVElement( new VesselElementEllipsoid(h, d1, d2));
}

PVElement VesselElement::make_ellipsoid(double d1, double d2)
{
    double h = std::max(d1, d2) / 4;
    return PVElement( new VesselElementEllipsoid(h, d1, d2));
}

PVElement VesselElement::make_flat(double d1, double d2)
{
    return PVElement( new VesselElementCone(0, d1, d2));
}

double VesselElement::top_y() const
{
    return bottom_y + height_y;
}

double VesselElement::get_width() const
{
    return 2 * M_PI * std::max(
                get_radius_by_ly(0),
                get_radius_by_ly(height_y));
}

double VesselElement::get_radius_by_ly(double y) const
{
    return get_radius_by_lz( y2z(y) );
}

PVElement VesselElement::make_cyl(double h, double d)
{
    return PVElement( new VesselElementCyl(h, d));
}

VesselElementCyl::VesselElementCyl(double h, double d)
    : VesselElement(h)
    , diameter(d)
{
    height_y = height_z;
}

double VesselElementCyl::get_radius_by_lz(double z) const
{
    return diameter / 2;
}

double VesselElementCyl::z2y(double z) const
{
    return z;
}

double VesselElementCyl::y2z(double y) const
{
    return y;
}

bool VesselElementCyl::geodesic_step(double dl, VesselContext& ctx) const
{
    double &ref_y = ctx.coords.y;
    double dy = dl * cos(ctx.azimuth);

    bool result = false;
    double over = 0;
    if (dy > 0 && ref_y + dy > height_y)
    {
        result = true;
        ref_y = height_y;

        dl = scale( height_y - dy - ref_y, .0, dy, 0.0, dl);

    } else if (dy < 0 && ref_y + dy < 0) {
        result = true;
        ref_y = 0;
        dl = scale( ref_y + dy, .0, dy, 0.0, dl);
    }
    else {
        ref_y += dy;
    }

    double dx = dl * sin(ctx.azimuth);
    double dfi = dx / (diameter / 2);

    // коррекци€ рассто€ни€
    ctx.distance += dl;

    // коррекци€ угла
    double& ref_fi = ctx.coords.fi;
    ref_fi = normal_angle(ref_fi + dfi);
    assert( is_normal_angle(ref_fi) );

    return result;
}

VesselElementCone::VesselElementCone(double h, double d, double d2)
    : VesselElement(h)
    , diameter1(d)
    , diameter2(d2)
{
    height_y = calc_height_y();
}

bool VesselElementCone::is_lower_bottom() const
{
    return diameter2 > diameter1;
}

double VesselElementCone::get_radius_by_lz(double z) const
{
    z = std::max(.0, std::min(height_z, z));
    return scale(z, .0, height_z, diameter1, diameter2) / 2;
}

double VesselElementCone::get_radius_by_ly(double y) const
{
    y = std::max(.0, std::min(height_y, y));
    return scale(y, .0, height_y, diameter1, diameter2) / 2;
}

double VesselElementCone::calc_height_y() const
{
    double r0 = fabs(diameter1 - diameter2)/2;
    return sqrt(r0*r0 + height_z*height_z);
}

double VesselElementCone::z2y(double z) const
{
    assert(0 <= z && z <= height_z);
    return scale(z, .0, height_z, .0, height_y);
}

double VesselElementCone::y2z(double y) const
{
    assert(0 <= y && y <= height_y);
    return scale(y, .0, height_y, .0, height_z);
}

bool VesselElementCone::geodesic_step(double dl, VesselContext &ctx) const
{
    double &y = ctx.coords.y;
    double dy = dl * sin(ctx.azimuth);

    bool result = false;
    if (dy > 0 && y > bottom_y + height_y)
    {
        result = true;
        y = bottom_y + height_y;
    } else if (dy < 0 && y < bottom_y) {
        result = true;
        y = bottom_y;
    }
    else {
        y += dy;
    }

    double diameter = scale(y, bottom_y, bottom_y + height_y, diameter1, diameter2);
    double dx = dl * cos(ctx.azimuth);
    double dfi = dx / diameter;

    double fi = ctx.coords.fi;
    fi = normal_angle(fi + dfi);
    return result;
}

VesselElementEllipsoid::VesselElementEllipsoid(double h, double d, double d2)
    : VesselElementCone(h, d, d2)
{
    a = std::max(d, d2) / 2;
    a2 = std::min(d, d2) / 2;
    b = h;

    e = sqrt(a*a - b*b) / a;   // 0,866025
    e1 = sqrt(a*a - b*b) / b;  // 1,732051
    c = a / sqrt(1 - e*e);     // aЈ2

    generate_local_fs();
}


void VesselElementEllipsoid::generate_local_fs()
{
    // расчет fi2M_cache
    double x0 = a;
    double z0 = 0;
    double y = 0;
    double bsign = 1;
    if (is_lower_bottom())
    {
        x0 = 0;
        bsign = -1;
    }
    double x = 0, z = 0;
    ellipsoid_height_z = height_z;
    height_z = get_height_z_by_diameters();
    for(double EF = 0; EF < M_90; EF += M_90 / 1024)
    {
        double fi = is_lower_bottom() ? M_90 - EF : EF;

        x = a * cos(fi);
        z = b * sin(fi);
        double tgF = z / x;

        if (is_lower_bottom())
        {
            z = height_z - z;

            // участок эллипса, который не входит в днище
            // (нижн€€ часть) игнориуем, однако фиксируем момент
            // когда точка попала на интересующую нас часть
            if (x <= a2)
            {
                x0 = x; z0 = z;
                continue;
            }
        }

        // расчитываем длину дуги
        y += sqrt(pow(x-x0,2)+pow(z-z0,2));
        x0=x; z0=z;

        double tgB = tgF / (1-e*e);
        double B = atan(tgB);

        if (!is_lower_bottom())
        {
            if (z >= height_z)
                break;
        }

        local_z2y_f.addValue(z, y);
        local_y2z_f.addValue(y, z);
        local_y2B_f.addValue(y, B * bsign);
        local_B2y_f.addValue(B * bsign, y);
    }

    height_y = y;

    local_z2y_f.addValue(height_z, height_y);
    local_y2z_f.addValue(height_y, height_z);

    if (a2 == 0)
    {
        if (is_lower_bottom())
        {
            local_y2B_f.addValue(0, -M_90);
            local_B2y_f.addValue(-M_90, 0);
        } else {
            local_y2B_f.addValue(height_y, M_90);
            local_B2y_f.addValue(M_90, height_y);
        }
    }
}

double VesselElementEllipsoid::get_radius_by_lz(double z) const
{
    double r0;
    assert(0 <= z &&
           (z <= height_z
            || fabs(z-height_z) < 1e-3));

    z = std::min(height_z,
                 std::max(0., z));
    if (is_lower_bottom())
    {
        z = height_z - z;
        r0 = diameter2 / 2;
    } else
    {
        // верхнее днище
        r0 = diameter1 / 2;
    }
    double sin = z / ellipsoid_height_z;
    double cos = sqrt(1 - sin*sin);
    return r0 * cos;
}

double VesselElementEllipsoid::get_radius_by_ly(double y) const
{
    return VesselElement::get_radius_by_ly(y);
}

double VesselElementEllipsoid::get_height_z_by_diameters() const
{
    double cos = a2 / a;
    double sin = sqrt(1 - cos*cos);
    double z = b * sin;
    return z;
}

double VesselElementEllipsoid::z2y(double z) const
{
    return local_z2y_f(z);
}

double VesselElementEllipsoid::y2z(double y) const
{
    return local_y2z_f(y);
}


/// ѕодшивалов, страница 57
/// стр. 36, система дифференциальных уравнений геодезической линии
/// изменение широты
///   dB = (cosA / M) Ј dS
///
/// изменение долготы
///   dL = (sinA / r) Ј dS
///
/// изменение азимута
///   dA = (sinA / N) Ј dS
///

bool VesselElementEllipsoid::geodesic_step(double ds, VesselContext &ctx) const
{
    // коррекци€ рассто€ни€
    ctx.distance += ds;

    check_if_polar_mode(ctx);
    if (ctx.polar_mode)
        return geodesic_flat(ds, ctx);
    else
        return geodesic_ellipsoid(ds, ctx);
}

void VesselElementEllipsoid::check_if_polar_mode(VesselContext & ctx) const
{
    // находим широту
    double ly = ctx.coords.y;
    double B = local_y2B_f(ly);

    bool polar_zone;
    const double polar_limit = 0.95;
    if (is_lower_bottom())
        polar_zone = ly < (1 - polar_limit) * height_y;
    else
        polar_zone = ly > height_y * polar_limit;

    if (!ctx.polar_mode && polar_zone)
    {
        // переход в пол€рный режим

        // fi=0 направлено вниз
        // ось u вправо
        // ось v вверх

        // пересчет координат
        double tgF = tan(B) * (1 - e*e);
        double F = atan(tgF);

        double r = (M_90 - fabs(F)) * b;
        ctx.uvcoords.u = r * cos(ctx.coords.fi - M_90);
        ctx.uvcoords.v = r * sin(ctx.coords.fi - M_90);

        ctx.polar_azimuth = is_lower_bottom()
                ? -M_90 - ctx.azimuth - ctx.coords.fi
                :  M_90 - ctx.azimuth - ctx.coords.fi;
        ctx.polar_azimuth = normal_angle(ctx.polar_azimuth);

        ctx.polar_mode = true;

    } else if (ctx.polar_mode && !polar_zone)
    {
        // выход из пол€рного режима
        ctx.polar_mode = false;
    }
}

bool VesselElementEllipsoid::geodesic_flat(double ds, VesselContext & ctx) const
{
    double du = ds * cos(ctx.polar_azimuth);
    double dv = ds * sin(ctx.polar_azimuth);

    ctx.uvcoords.u += du;
    ctx.uvcoords.v += dv;

    double r = sqrt(pow(ctx.uvcoords.u, 2)
                  + pow(ctx.uvcoords.v, 2));

    ctx.coords.y = is_lower_bottom()
            ? r : height_y - r;

    if (r > 1e-12)
    {
        // пересчет из пол€рного режима

        // fi=0 направлено вниз
        // ось u вправо
        // ось v вверх

        double f = atan2(ctx.uvcoords.v,
                         ctx.uvcoords.u);
        ctx.coords.fi = normal_angle(f + M_90);
        ctx.azimuth = is_lower_bottom()
                ? -M_90 - ctx.polar_azimuth - ctx.coords.fi
                :  M_90 - ctx.polar_azimuth - ctx.coords.fi;
        ctx.azimuth = normal_angle(ctx.azimuth);
    }
    else
    {
        // точка полюса значит датчик на полюсе
        // и тупо принимаем некоторые данные
        // касающиес€
        ctx.coords.fi = ctx.azimuth = ctx.polar_azimuth;
    }

    return false;
}

bool VesselElementEllipsoid::geodesic_ellipsoid(double ds, VesselContext & ctx) const
{
    double ly = ctx.coords.y;

    double B = local_y2B_f(ly);            // находим широту

    double W = sqrt(1 - pow(e*sin(B), 2)); // основна€ функци€ широты
    double V = W / sqrt(1 - e*e);          // втора€ основна€ функци€ широты
    // главные радиусы кривизны
    double N = c / V;           // другой способ
    double N1 = a / W;
    double M = c / pow(V, 3);   // другой способ
    double M1 = a * (1 - e*e) / pow(W,3);
    double r = N * cos(B);      // радиус кривизны параллели

    double cosA = cos(ctx.azimuth);
    double sinA = sin(ctx.azimuth);
    double dB = cosA / M * ds;
    double dL = sinA / r * ds;
    double dA = sinA / N * ds * tan(B);

    ctx.coords.fi = normal_angle(ctx.coords.fi + dL);

    ctx.azimuth += dA;  // изменение азимута
    B += dB;  // изменение широты

    ly = local_B2y_f(B);

    bool on_edge = false;
    if (is_lower_bottom())
    {
        // растет вверх
        if (cos(ctx.azimuth) > 0 && ly >= height_y)
        {
            ly = height_y;
            on_edge = true;
        }
    }
    else
    {
        // растет вниз
        if (cos(ctx.azimuth) < 0 && ly <= 0)
        {
            ly = 0;
            on_edge = true;
        }
    }

    ctx.coords.y = ly;
    return on_edge;
}


}
