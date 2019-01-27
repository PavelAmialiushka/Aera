#include "stdafx.h"

#include "utilites/TestFile.h"

#include "ListModel.h"

#include "utilites\unittest.h"
#include "utilites\singleton.h"

#include "data/loader.h"

class UNITTEST_SUITE(ListMaker_test)
{
public:

    nodes::loader *root;
    list_model    *model;

    void setUp()
    {
        root=new nodes::loader;
        root->open( get_test_filename(0) );

        model=new list_model(root);

        process::suspend_lock lock(model);

        model->set_peak_mode(false);
        model->set_nav_mode(false);
        model->set_rel_mode(false);

        boost::array<aera::chars, 1> f= {aera::C_Time};
        model->set_features(&f[0], 1);
    }

    void tearDown()
    {
        delete model;
        delete root;
    }

    UNITTEST( easy_test )
    {
        bool event;
        {
            model->get_result();

            assertEqual("    0.0000000 Resume", model->line(0, event));
            assertEqual("   12.2352720 ", model->line(1, event));
            assertEqual(338, model->size());
        }

        aera::chars fts[]= {aera::C_Time, aera::C_Amplitude};
        model->set_features(fts, 2);
        {
            model->get_result();

            assertEqual(338, model->size());
            assertEqual("   12.2352720  54 ", model->line(1, event));

            assertDoublesEqual(537.192476, model->get_time(model->search_time(500.0)), 0.01);
        }

        // changing mode

        model->set_nav_mode(true);
        {
            model->get_result();

            assertEqual(13, model->size());
            assertEqual("  907.2435915 Timemark", model->line(1, event));

            assertDoublesEqual(907.2435915, model->get_time(model->search_time(500.0)), 0.01);

            assertEqual("            T   A ", model->get_header());
        }
    }
} INSTANCE;
