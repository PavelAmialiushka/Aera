#include "stdafx.h"

#include "utilites\TestFile.h"

#include "loader.h"
#include "locator.h"

//////////////////////////////////////////////////////////////////////////

#include "LocationSetup.h"
#include "LocationUnit.h"

#include "ZonalSetup.h"
#include "PlanarSetup.h"

#include "Collector.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/unittest.h"

using namespace location;
using namespace data;
using nodes::presult;

#include "fake_node.h"

class UNITTEST_SUITE( location_test )
{
public:
    shared_ptr<nodes::loader> loader;
    shared_ptr<nodes::locator> locator;
    shared_ptr<nodes::fake_node> fakenode;

    void setUp()
    {
        loader.reset( new nodes::loader() );
        loader->open( get_test_filename(0) );

        locator.reset( new nodes::locator );
        locator->set_parent(loader.get());
        locator->restart();
    }

    void tearDown()
    {
        loader.reset();
        locator.reset();
        fakenode.reset();
    }

    void activateLocation(location::Setup loc, std::vector<double> &data)
    {
        fakenode.reset ( new nodes::fake_node(&data[0], data.size()) );

        process::suspend_lock lock(locator);
        locator->set_location(loc);
        locator->set_parent(fakenode.get());
    }

    UNITTEST( zone_location_test )
    {
        pslice slice;
        pslice sh;

        std::vector<double> data;

        nodes::fake_node::append_dt(data, 1, 0, 0,    10);
        nodes::fake_node::append_dt(data, 2, 0, 1000, 20);
        nodes::fake_node::append_dt(data, 3, 0, 1900, 30);
        nodes::fake_node::append_dt(data, 4, 0, 2900, 40);

        nodes::fake_node::append_dt(data, 5, 0, 3900, 50); // next hit
        nodes::fake_node::append_dt(data, 6, 0, 4900, 60);

        nodes::fake_node::append_dt(data, 11, 1,    0, 70); // 000`000 + 4`000
        nodes::fake_node::append_dt(data, 12, 1, 1000, 80); // 000`001 + 3`000
        nodes::fake_node::append_dt(data, 13, 1, 2000, 90); // 000`002 + 0`050

        ZonalSetup* zonal = new ZonalSetup;
        zonal->set_definition_time( unit_value(2000, usec) );
        zonal->set_lockout( unit_value(3000, usec) );
        zonal->set_use_peak_time(true);
        activateLocation(Setup(zonal), data);

        {
            nodes::presult r = process::get_result<nodes::result>(locator);
            slice = r->ae;
            sh = slice->merge(r->ae_sub);
            assertEqual(9, sh->size());

            assertEqual(3, slice->get_subhit_count(0));
            assertDoublesEqual(0, slice->get_value(0), 1e-6);

            assertEqual(1, slice->get_subhit_count(1));
            assertDoublesEqual(0.003900, slice->get_value(1), 1e-6);

            assertEqual(2, slice->get_subhit_count(2));
            assertDoublesEqual(1.000000, slice->get_value(2), 1e-6);
        }

        // TODO разобраться что здесь проверялось и обновить
//        channel_sheme sheme;
//        grader->get_channel_sheme(sheme);
//        sheme.setactive(1, 0);
//        grader->set_channel_sheme(sheme);
//        {
//            nodes::presult r = process::get_result<nodes::result>(locator);

//            slice=r->ae;
//            assertDoublesEqual(0.003900, slice->get_value(0), 1e-6);
//            assertDoublesEqual(5.0, slice->get_value(0, aera::C_Channel), 1e-6);
//            assertEqual(1, slice->get_subhit_count(0));
//            assertEqual(2, slice->get_subhit_count(1));

//            sh = slice->merge(r->ae_sub);
//            assertEqual(5, sh->size());
//            assertDoublesEqual(0.004960, sh->get_value(1, aera::C_PeakTime), 1e-6);
//            assertDoublesEqual(1.001080, sh->get_value(3, aera::C_PeakTime), 1e-6);
//        }
    }


    UNITTEST( planar_test )
    {
        std::vector<double> data;

        nodes::fake_node::append_dt(data, 1, 0, 0);   // центр треугольника
        nodes::fake_node::append_dt(data, 2, 0, 0);
        nodes::fake_node::append_dt(data, 3, 0, 0);

        nodes::fake_node::append_dt(data, 3, 1, 0);    // третий датчик
        nodes::fake_node::append_dt(data, 1, 1, 333.3);
        nodes::fake_node::append_dt(data, 2, 1, 333.3);

        nodes::fake_node::append_dt(data, 1, 2, 0); // несогласованные данные
        nodes::fake_node::append_dt(data, 2, 2, 0);
        nodes::fake_node::append_dt(data, 3, 2, 0);
        nodes::fake_node::append_dt(data, 4, 2, 0);

        nodes::fake_node::append_dt(data, 3, 3, 0); // извне треуголоьника
        nodes::fake_node::append_dt(data, 2, 3, 500);
        nodes::fake_node::append_dt(data, 1, 3, 500);

        PlanarSetup* planar = new PlanarSetup;
        PlanarSetup::sensors_map_t sensors;
        sensors[1].x=0;
        sensors[1].y=0;

        sensors[2].x=1;
        sensors[2].y=0;

        sensors[3].x=0.5;
        sensors[3].y=0.866025;

        sensors[4].x=0.5;
        sensors[4].y=-0.866025;

        planar->set_sensors(sensors);
        planar->set_unit( m );

        planar->set_velocity( unit_value(3.0, mm_us) );
        planar->set_definition_time( unit_value(2000, usec) );
        planar->set_use_peak_time(false);

        activateLocation(Setup(planar), data);

        {
            nodes::presult r = process::get_result<nodes::result>(locator);
            pslice slice=r->ae;
            assertEqual(4, slice->size());

            assertDoublesEqual(0, slice->get_value(0u), 0.0001);
            assertDoublesEqual(0.500, slice->get_value(0u, aera::C_CoordX), 0.001); assertDoublesEqual(0.289, slice->get_value(0u, aera::C_CoordY), 0.001);

            assertDoublesEqual(1, slice->get_value(1u), 0.0001);
            assertDoublesEqual(0.500, slice->get_value(1u, aera::C_CoordX), 0.001);
            assertDoublesEqual(0.866, slice->get_value(1u, aera::C_CoordY), 0.001);

            assertDoublesEqual(2, slice->get_value(2u), 0.0001);
            assertDoublesEqual(0.613, slice->get_value(2u, aera::C_CoordX), 0.001);
            assertDoublesEqual(0.000, slice->get_value(2u, aera::C_CoordY), 0.001);

            assertDoublesEqual(3, slice->get_value(3u), 0.0001);
            assertDoublesEqual(0.500, slice->get_value(3u, aera::C_CoordX), 0.001);
            assertDoublesEqual(0.866, slice->get_value(3u, aera::C_CoordY), 0.001);
        }
    }

} INSTANCE;
