#include "stdafx.h"

#include "utilites/TestFile.h"

#include "VesselSetup.h"

#include "loader.h"
#include "locator.h"
#include "grader.h"


//////////////////////////////////////////////////////////////////////////

#include "LocationSetup.h"
#include "LocationUnit.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/unittest.h"

using namespace location;
using namespace data;


std::string cmp_points(VesselStructure& vs,
                    double delta,
                    gfiy_coords const& lhs,
                    gfiy_coords const& rhs)
{
    double d = vs.local_distance(lhs,rhs);
    if (d < delta)
        return std::string();

    return strlib::strf("%s != %s (d=%s)", lhs, rhs, d);
}



class UNITTEST_SUITE( vessel_test )
{
public:

    UNITTEST(elements_cyl)
    {
        PVElement a = VesselElement::make_cyl(6, 3);
        assertDoublesEqual(6, a->height_y, 1e-3);
        assertDoublesEqual(6, a->height_z, 1e-3);

        assertDoublesEqual(1.5, a->get_radius_by_lz(3), 1e-3);
        assertDoublesEqual(1.5, a->get_radius_by_lz(0), 1e-3);
    }

    UNITTEST(elements_cone)
    {
        PVElement b = VesselElement::make_cone(5, 4, 2);
        assertDoublesEqual(5.099, b->height_y, 1e-3);
        assertDoublesEqual(5, b->height_z, 1e-3);

        assertDoublesEqual(0,   b->z2y(0), 1e-3);
        assertDoublesEqual(5.0, b->y2z(5.099), 1e-3);
        assertDoublesEqual(1.0, b->y2z(1.020), 1e-3);

        assertDoublesEqual(5.099, b->z2y(5), 1e-3);
        assertDoublesEqual(1.020, b->z2y(1), 1e-3);

        assertDoublesEqual(2,     b->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.5,   b->get_radius_by_lz(2.5), 1e-3);
        assertDoublesEqual(1,     b->get_radius_by_lz(5), 1e-3);
    }

    UNITTEST(elements_flat)
    {
        PVElement b = VesselElement::make_flat(6, 2);
        assertDoublesEqual(2, b->height_y, 1e-3);
        assertDoublesEqual(0, b->height_z, 1e-3);

        assertDoublesEqual(0, b->y2z(2), 1e-3);
        assertDoublesEqual(0, b->y2z(1), 1e-3);
        assertDoublesEqual(0, b->y2z(0), 1e-3);

        assertDoublesEqual(3.0, b->get_radius_by_ly(0), 1e-3);
        assertDoublesEqual(2.0, b->get_radius_by_ly(1), 1e-3);
        assertDoublesEqual(1.0, b->get_radius_by_ly(2), 1e-3);
    }

    UNITTEST(elements_ellipse)
    {
        // полусфера, верхнее днище
        PVElement c = VesselElement::make_ellipsoid(2, 4, 0);
        assertDoublesEqual(3.1415,c->height_y, 1e-3);
        assertDoublesEqual(2,     c->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.323, c->get_radius_by_lz(1.5), 1e-3);
        assertDoublesEqual(0,     c->get_radius_by_lz(2), 1e-3);

        // полусфера, нижнее днище
        PVElement d = VesselElement::make_ellipsoid(2, 0, 4);
        assertDoublesEqual(3.1415,d->height_y, 1e-3);
        assertDoublesEqual(0,     d->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.937, d->get_radius_by_lz(1.5), 1e-3);
        assertDoublesEqual(2,     d->get_radius_by_lz(2), 1e-3);

        // эллиптичское верхнее днище
        PVElement e = VesselElement::make_ellipsoid(1, 4, 0);
        assertDoublesEqual(2.4221,e->height_y, 1e-3);
        assertDoublesEqual(2,     e->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.323, e->get_radius_by_lz(0.75), 1e-3);
        assertDoublesEqual(0,     e->get_radius_by_lz(1), 1e-3);

        // эллиптичское нижнее днище
        PVElement f = VesselElement::make_ellipsoid(1, 0, 4);
        assertDoublesEqual(2.4221,f->height_y, 1e-3);
        assertDoublesEqual(0,     f->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.937, f->get_radius_by_lz(0.75), 1e-3);
        assertDoublesEqual(2,     f->get_radius_by_lz(1), 1e-3);

        // неполное днище
        // эллиптичское верхнее днище
        PVElement g = VesselElement::make_ellipsoid(1, 4, 2);
        assertDoublesEqual(1.410, g->height_y, 1e-3);
        assertDoublesEqual(2,     g->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.323, g->get_radius_by_lz(0.75), 1e-3);
        assertDoublesEqual(1,     g->get_radius_by_lz(0.866), 1e-3);

        // неполное днище
        // эллиптичское нижнее днище
        PVElement eh = VesselElement::make_ellipsoid(1, 2, 4);
        assertDoublesEqual(1.410, eh->height_y, 1e-3);
        assertDoublesEqual(1,     eh->get_radius_by_lz(0), 1e-3);
        assertDoublesEqual(1.987, eh->get_radius_by_lz(0.75), 1e-3);
        assertDoublesEqual(2,     eh->get_radius_by_lz(0.866), 1e-3);
    }

    UNITTEST(elements_ellipse_2)
    {
        // верхнее днище
        VesselElementEllipsoid el1(1, 4, 0);
        assertDoublesEqual(el1.local_y2B_f(0), 0, 1e-6);
        assertDoublesEqual(el1.local_y2B_f(1), 1.12419074, 1e-6);
        assertDoublesEqual(el1.local_y2B_f(el1.height_y), M_90, 1e-6);

        assertDoublesEqual(el1.local_B2y_f(0), 0, 1e-6);
        assertDoublesEqual(el1.local_B2y_f(1.12419074), 1.000000, 1e-6);
        assertDoublesEqual(el1.local_B2y_f(M_90), el1.height_y, 1e-6);

        assertDoublesEqual(el1.local_y2z_f(0), 0, 1e-6);
        assertDoublesEqual(el1.local_y2z_f(1), 0.72219831, 1e-6);
        assertDoublesEqual(el1.local_y2z_f(el1.height_y), 1, 1e-6);

        assertDoublesEqual(el1.local_z2y_f(0), 0, 1e-6);
        assertDoublesEqual(el1.local_z2y_f(0.72219831), 1.0, 1e-6);
        assertDoublesEqual(el1.local_z2y_f(2), el1.height_y, 1e-6);

        // нижнее днище
        VesselElementEllipsoid el2(1, 0, 4);
        assertDoublesEqual(el2.local_y2B_f(0), -M_90, 1e-6);
        assertDoublesEqual(el2.local_y2B_f(1), -1.29390375, 1e-6);
        assertDoublesEqual(el2.local_y2B_f(el2.height_y), 0, 1e-6);

        assertDoublesEqual(el2.local_B2y_f(0), el2.height_y, 1e-6);
        assertDoublesEqual(el2.local_B2y_f(-1.29390375), 1, 1e-6);
        assertDoublesEqual(el2.local_B2y_f(-M_90), 0, 1e-6);

        assertDoublesEqual(el2.local_y2z_f(0), 0, 1e-6);
        assertDoublesEqual(el2.local_y2z_f(1), 0.13061970, 1e-6);
        assertDoublesEqual(el2.local_y2z_f(el2.height_y), 1, 1e-6);

        assertDoublesEqual(el2.local_z2y_f(0), 0, 1e-6);
        assertDoublesEqual(el2.local_z2y_f(0.13061970), 1, 1e-6);
        assertDoublesEqual(el2.local_z2y_f(1), el2.height_y, 1e-6);
    }

    UNITTEST( element_structure_cylinder )
    {
        VesselStructure vs;
        vs.append( VesselPart::Cylinder, 4, 4, 4);
        vs.compile();

        gfiy_coords s1 = vs * lfiy_coords(0, 0.0, 2.0);

        VesselSensor sens1(&vs, s1, true);

        gfiy_coords p1 = vs * lfiy_coords(0, M_PI, 2.0);
        double d11 = sens1.distance_to(p1);
        assertDoublesEqual(M_PI * 2,
                          d11, 0.05);

        gfiy_coords p2 = vs * lfiy_coords(0, M_PI, 0.0);
        double d21 = sens1.distance_to(p2);
//        assertDoublesEqual(sqrt( pow(M_PI * 2, 2) + 2*2),
//                          d21, 0.20);
    }

#if 0
    UNITTEST(examine_geodesic_lines)
#else
    void examine_geodesic_lines()
#endif
    {
        VesselStructure vs;
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.append( VesselPart::Cylinder, 4, 4, 4);
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.compile();

        // точка на нижнем кольцевом шве
        gfiy_coords start = vs * lfiy_coords(1, 0.0, 1);

        double r = 2;
        double distance = 10;
        int count = 64;

        double precs[] = {0.001, 0.002, 0.005, 0.0075, 0.010, 0.015, 0.020};
        double dxs_avr[SIZEOF(precs)] = {0.0};
        double dxs_max[SIZEOF(precs)] = {0.0};
        int dxs_count = 0;

        for(double a = 0; a < M_360; a += M_360 / count )
        {
            gfiy_coords end0 = vs.geodesic_line(start, a, distance, 0.0001);

            for(unsigned index=0; index < SIZEOF(precs); ++index)
            {
                gfiy_coords end1 = vs.geodesic_line(start, a, distance,
                                                    precs[index]);
                double dx = end1.get_est_delta(end0);
                dxs_avr[index] += dx;
                dxs_max[index] = std::max(dxs_max[index], dx);
            }
            ++dxs_count;
        }
        for(unsigned index=0; index<SIZEOF(precs); ++index)
        {
            LOG(strlib::strf("For prec=%.3f : deltas : avr=%.6f, max=%.6f",
                             precs[index],
                             dxs_avr[index] / dxs_count,
                             dxs_max[index]
                             ));
        }
    }

#if 0
    UNITTEST( examine_distance_calculation_precision )
#else
    void examine_distance_calculation_precision()
#endif
    {
        VesselStructure vs;
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.append( VesselPart::Cylinder, 4, 4, 4);
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.compile();

        // датчик поставлен на грани нижнего днища
        gfiy_coords s = vs * lfiy_coords(1, 0.0, 0);

        process::tick_timer timer;
        VesselSensor sens(&vs, s, true);
        int elapsed = timer.elapsed();

        std::vector<double> az, ds;

        for(int x = 1; x < 10; x+=2)
        {
            for(int f=0; f<32; ++f)
            {
                az.push_back( normal_angle(scale(f, 32, M_360)));
                ds.push_back( 1.0 * x + 0.5 );
            }
        }

//        // высота цилиндра
//        double ch = 4.000000;
//        // высота элипсоида
//        double eh = 2.422111;

        int failure1_count = 0;
        int failure2_count = 0;
        double max_delta1 = 0;
        double max_delta2 = 0;
        double avr_delta = 0;
        for(unsigned index=0; index < az.size(); ++index)
        {
            double a0 = az[index];
            double d0 = ds[index];
            gfiy_coords t = vs.geodesic_line(s, a0, d0, vs.get_default_step()/10);

            double a = 0;
            double d = sens.distance_to(t, &a);

            gfiy_coords t1 = vs.geodesic_line(s, a, d, vs.get_default_step()/10);
            double delta1 = vs.local_distance(t1, t);

            double d2 = sens.distance_to(t1);
            double delta2 = fabs(d-d2);

            std::string text;
            text += strlib::strf("deltas for index: ---------------- %d: %.3f %.3f\n",
                                 index, delta1, delta2);
            text += strlib::strf("  a %.3f d %.3f => f %.3f y %.3f\n",
                                 a0, d0, t.fi, t.y);
            text += strlib::strf("  a %.3f d %.3f => f %.3f y %.3f\n",
                                 a, d, t1.fi, t1.y);
            text += strlib::strf("  a %.3f d %.3f\n",
                                 a, d2);

            if (delta2 < 0.5)
            {
                max_delta1 = std::max(max_delta1, delta1);
                max_delta2 = std::max(max_delta2, delta2);
                avr_delta += delta2;
            }

            if (delta2 > 0.005)
            {
                if (delta2 > 0.5)
                {
                    failure2_count += 1;
                    text = "FAILURE: " + text;
                } else
                {
                }

                failure1_count += 1;
                LOG(text);
            }
        }
        if (failure1_count)
        {
            LOG(strlib::strf("deltas %d[ms] count= %d / %d (%.1f%% / %.1f%%), "
                             "\nmax_est=%.3fm max_dis=%.3f, avr_dis=%.3f",
                             elapsed,
                             failure1_count, failure2_count,
                             100.0 * failure1_count / az.size(),
                             100.0 * failure2_count / az.size(),
                             max_delta1, max_delta2,
                             avr_delta / (az.size() - failure2_count)));
        }
    }

    UNITTEST( vessel_geodesic_lines_precision )
    {
        VesselStructure vs;
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.append( VesselPart::Cylinder, 4, 4, 4);
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.compile();

        // высота эллипсоида
        double eh = vs.element(0)->height_y;

        double r = 2;

        // движемся вдоль кольцевого шва в области цилиндра
        gfiy_coords p1 = vs.geodesic_line(vs * lfiy_coords(1, 0.0, 1),
                                          M_90, M_PI * r,
                                          0.001);
        assertMessage(cmp_points(vs, 0.001,
                                 p1, vs * lfiy_coords(1, M_PI, 1)));

        // движемся вдоль кольцевого шва в области эллипсоида
        gfiy_coords p2 = vs.geodesic_line(vs * lfiy_coords(1, 0.0, 0),
                                          M_90, M_PI * r,
                                          0.001);
        assertMessage(cmp_points(vs, 0.001,
                                 p2, vs.to_gfiy(lfiy_coords(1, M_PI, 0))));

        // движемся от кромки верхнего днища на 1 метр вверх
        gfiy_coords p3 = vs.geodesic_line(vs * lfiy_coords(2, 0.0, 0),
                                          0, 1,
                                          0.0001);
        assertMessage(cmp_points(vs, 0.001,
                                 p3, vs.to_gfiy(lfiy_coords(2, 0, 1))));

        // движемся от кромки верхнего днища к верхнему полюсу
        gfiy_coords p4 = vs.geodesic_line(vs * lfiy_coords(2, 0.0, 0),
                                          0, eh,
                                          0.001);
        assertMessage(cmp_points(vs, 0.004,
                                 p4, vs.to_gfiy(lfiy_coords(2, M_PI, eh))));

        // движемся от кромки нижнего днища к нижнему полюсу
        gfiy_coords p5 = vs.geodesic_line(vs * lfiy_coords(0, 0.0, eh),
                                          -M_180, eh,
                                          0.001);
        assertMessage(cmp_points(vs, 0.003,
                                 p5, vs * lfiy_coords(0, M_PI, 0)));

        // движемся от нижнего полюса до кромки нижнего днища
        gfiy_coords p6 = vs.geodesic_line(vs * lfiy_coords(0, 0.0, 0),
                                          0, eh,
                                          0.001);
        assertMessage(cmp_points(vs, 0.002,
                                 p6, vs * lfiy_coords(0, 0, eh)));

        // движемся от верхнего полюса до кромки верхнего днища
        gfiy_coords p7 = vs.geodesic_line(vs * lfiy_coords(2, 0.0, eh),
                                          0, eh,
                                          0.001);
        assertMessage(cmp_points(vs, 0.002,
                                 p7, vs.to_gfiy(lfiy_coords(2, M_PI, 0))));

        // движемся от верхнего полюса до нижнего полюса
        gfiy_coords p8 = vs.geodesic_line(vs * lfiy_coords(2, 0.0, eh),
                                          0, 2*eh + 4,
                                          0.001);
        assertMessage(cmp_points(vs, 0.002,
                                 p8, vs.to_gfiy(lfiy_coords(0, 0, 0))));
    }

    UNITTEST( element_simple_vessel_distance )
    {
        VesselStructure vs;
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.append( VesselPart::Cylinder, 4, 4, 4);
        vs.append( VesselPart::Ellipsoid, 4, 0, 0);
        vs.compile();

        // радиус
        double r = 2;
        // высота цилиндра
        double ch = 4.000000;
        // высота элипсоида
        double eh = 2.422111;

        // датчик поставлен на нижний полюс
        gfiy_coords s1 = vs * lfiy_coords(0, 0.0, 0);
        VesselSensor sens1(&vs, s1, true);

        // датчик поставлен на границе нижнего днища
        gfiy_coords s2 = vs * lfiy_coords(1, 0.0, 0);
        VesselSensor sens2(&vs, s2, true);

        // точка на грани нижнего днища
        gfiy_coords p1 = vs * lfiy_coords(1, 0.0, 0);
        double d11 = sens1.distance_to(p1);
        assertDoublesEqual(eh, d11, 0.20); // 0.20
        double d12 = sens2.distance_to(p1);
        assertDoublesEqual(0.0, d12, 0.01);

        // точка на осевом шве верхнего днища
        gfiy_coords p2 = vs * lfiy_coords(2, 45*M_PI/180, 0);
        double d21 = sens1.distance_to(p2);
//        assertDoublesEqual(eh+ch, d21, 0.12);   // 0.12
        double d22 = sens2.distance_to(p2);
        assertDoublesEqual(sqrt(ch*ch+r*r*M_PI*M_PI/4/4), d22, 0.012);

        // произвольная точка на верхнем днище
        gfiy_coords p4 = vs * lfiy_coords(2, M_PI/4, eh/2);
        double d41 = sens1.distance_to(p4);
        assertDoublesEqual(7.727, d41, 0.013); // 0.013
        double d42 = sens2.distance_to(p4);
        assertDoublesEqual(5.410, d42, 0.01);

        // противоположный полюс
        gfiy_coords p3 = vs * lfiy_coords(2, 2.12, eh);
        double d31 = sens1.distance_to(p3);
        assertDoublesEqual(2*eh+ch, d31, 0.01);
        double d32 = sens2.distance_to(p3);
        assertDoublesEqual(eh+ch, d32, 0.01);
    }

} INSTANCE;

