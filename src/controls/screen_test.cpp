#include "stdafx.h"

#include "controls/plot/screen.h"

#include "utilites/unittest.h"

using namespace plot;

class UNITTEST_SUITE(plotscreen_test)
{

public:

    UNITTEST( single_test )
    {
        screen scr;
        scr.set_buffer(500, 500);
        scr.set_xrange(range(0, 1000));
        scr.set_yrange(range(40, 99));
        scr.set_dot(900., 90., colors::normal);
        scr.set_dot(800., 95., colors::normal);

    }

    UNITTEST( more_complicated_test )
    {
        screen scr;
        scr.set_buffer(512, 512);
        scr.set_xrange(range(0, 4000));
        scr.set_yrange(range(40, 100));
        scr.set_dot(2000., 70., colors::normal);

        const darray *da=scr.get_buffer();

        assertTrue(da->at(255, 255)!=0);

    }

    UNITTEST( line_test )
    {
        screen scr;
        scr.set_buffer(10, 10);
        scr.set_xrange(range(0, 1000));
        scr.set_yrange(range(40, 100));
        scr.move_to(0., 40.);
        scr.line_to(1000., 100., colors::normal);

    }
} INSTANCE;
