#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////

#include "LocationSetup.h"
#include "LocationUnit.h"

#include "LinearSetup.h"

#include "Collector.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites/unittest.h"
#include "fake_node.h"

using namespace location;

class UNITTEST_SUITE( linearLocation_test )
{
public:
    UNITTEST( basic_test1 )
    {
        LinearSetup* linear = new LinearSetup;
        std::map<int, double> sensors;
        sensors[1]=11.0;
        sensors[2]=12.0;
        linear->set_sensors(sensors);
        linear->set_unit( m );

        linear->set_velocity( unit_value(3.0, mm_us) );
        linear->set_definition_time( unit_value(2000, usec) );
        linear->set_use_peak_time(false);

        nodes::fake_factory f(linear);
        f.imitate_event()
                .at(1, 0).at(2, 0)
                .ensure(11.5);

        f.imitate_event()
                .at(1, 0).at(2, 333.3)
                .ensure(11.0);

        f.imitate_event()
                .at(1, 0).at(2, 166.7)
                .ensure(11.250);

        f.imitate_event() // incorrect
                .at(1, 0).at(2, 500)
                .ensure_none();

        f.run();
    }

    UNITTEST( basic_test_other_units )
    {
        LinearSetup* linear = new LinearSetup;
        std::map<int, double> sensors;
        sensors[1]=1100;
        sensors[2]=1200;
        linear->set_sensors(sensors);
        linear->set_unit( cm );

        linear->set_velocity( unit_value(3000, m_s) );
        linear->set_definition_time( unit_value(10, m) );
        linear->set_use_peak_time(false);

        nodes::fake_factory f(linear);
        f.imitate_event()
                .at(1, 0).at(2, 0)
                .ensure(1150);

        f.imitate_event()
                .at(1, 0).at(2, 333.333)
                .ensure(1100);

        f.imitate_event()
                .at(1, 0).at(2, 166.667)
                .ensure(1125);

        f.imitate_event() // incorrect
                .at(1, 0).at(2, 500)
                .ensure_none();

        f.run();
    }

    UNITTEST( circular_test1 )
    {
        LinearSetup* linear = new LinearSetup;
        std::map<int, double> sensors;
        sensors[1]=1.0;
        sensors[2]=4.0;
        sensors[3]=7.0;
        linear->set_sensors(sensors);
        linear->set_circular_length(unit_value(9.0, m));
        linear->set_unit( m );

        linear->set_velocity( unit_value(3.0, mm_us) );
        linear->set_definition_time( unit_value(2000, usec) );
        linear->set_use_peak_time(false);

        nodes::fake_factory f(linear);
        f.imitate_event().at(1, 0).at(2, 0)
                .ensure(2.5);

        f.imitate_event().at(1, 0).at(2, 333.333)
                .ensure(2.0);

        f.imitate_event().at(1, 0).at(2, 666.667)
                .ensure(1.5);

        f.imitate_event().at(1, 0).at(2, 1000.0)
                .ensure(1.0);
        //

        f.imitate_event().at(2, 0).at(3, 0)
                .ensure(5.5);

        f.imitate_event().at(2, 0).at(3, 333.333)
                .ensure(5.0);

        //

        f.imitate_event().at(3, 0).at(1, 0)
                .ensure(8.5);

        f.imitate_event().at(3, 0).at(1, 333.333)
                .ensure(8.0);

        f.imitate_event().at(1, 0).at(3, 666.667)
                .ensure(0.5);

        f.run();
    }

} INSTANCE;

