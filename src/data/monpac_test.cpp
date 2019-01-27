#include "stdafx.h"

#include "utilites\TestFile.h"

#include "monpac.h"
#include "html_output.h"
#include "console_out.h"

#include "stage.h"
#include "channel.h"
#include "criterion.h"

#include "nodeFactory.h"
#include "loader.h"

#include "node.h"


/************************************************************************/
/* unittesting                                                          */
/************************************************************************/

using namespace monpac;

class UNITTEST_SUITE(monpac_test)
{
public:

    criterion c_;

    void setUp()
    {
        c_.set_stage(stage("5", 0, 1200));
    }

//  UNITTEST( test_lockout )
//  {
//    criterion c;
//    Processor mp(c);
//
//    mp.append(0.000000, 1, 50, 10, 3);
//    mp.append(0.001000, 2, 45, 1, 3);
//    mp.append(0.001999, 3, 44, 1, 3);
//    mp.append(0.002001, 2, 44, 1, 3);
//    assertEqual(2, mp.results().channels().size());
//
//    mp.append(0.003000, 3, 55, 11, 3);
//    mp.append(0.003100, 3, 40, 11, 3);
//    mp.append(0.003200, 3, 40, 11, 3);
//    assertEqual(3, mp.channels().size());
//  }

    UNITTEST( node_loader_A )
    {
        std::auto_ptr<nodes::loader> loader(new nodes::loader);

        criterion c;
        classifier mp(c);
        mp.set_parent( loader.get() );

        loader->open( get_test_filename(0) );

        pmonpac_result r = process::get_result<monpac_result>(&mp);
        assertEqual(8, r->ch_info_.size());
    }

    UNITTEST( node_loader_B )
    {
        std::auto_ptr<nodes::loader> loader(new nodes::loader);
        loader->open( get_test_filename(0) );

        criterion c;
        classifier mp(c);

        mp.set_parent( loader.get() );

        pmonpac_result r = process::get_result<monpac_result>(&mp);
        assertEqual(8, r->ch_info_.size());
    }

    UNITTEST( html_test1 )
    {
        std::auto_ptr<nodes::loader> loader(new nodes::loader);
        loader->open( get_test_filename(0) );

        criterion c;
        classifier mp(c);

        mp.set_parent( loader.get() );

        html_outputter out;
        out.accept(&mp);
    }

    UNITTEST(console1)
    {
        std::auto_ptr<nodes::loader> loader(new nodes::loader);
        loader->open( get_test_filename(0) );

        criterion c;
        classifier mp(c);

        mp.set_parent( loader.get() );

        console_outputter out;
        out.accept(&mp);
    }

    UNITTEST( criterion_test1 )
    {
        criterion cr;

        stage st;
        st.children_.push_back( shared_ptr<stage>(new stage("60", 10, 20) ));
        st.children_.push_back( shared_ptr<stage>(new stage("50", 30, 40) ));
        cr.set_stage(st);

        assertEqual(2, cr.get_holds().size());
        assertEqual(50, cr.get_threashold());
    }

    static void append(calculator *ch, double e)
    {
        ch->append(hit(150, 1, 55, e, 3));
    }

    UNITTEST( test_channel )
    {
        calculator ch(&c_, 1);

        for (int i=0; i<10; ++i) append(&ch, 10);

        channel hch=ch.calculate();
        assertEqual(zip_0, hch.get_zip());
        assertEqual(1.0, hch.h_index());
    }

    UNITTEST( crit_analyz )
    {
        criterion crit;

        stage st;
        st.children_.push_back( shared_ptr<stage>(new stage("1", 0, 300) ));
        st.children_.push_back( shared_ptr<stage>(new stage("2", 500, 1100) ));
        crit.set_stage(st);

        crit.set_vessel(2);

        calculator ch1(&crit, 1);

        boost::array<double, 16> test1= {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
        std::for_each(STL_II(test1), bind(&monpac_test::append, &ch1, _1));

        channel hch=ch1.calculate();

        assertEqual(48, hch.get_duration());
        assertEqual(0,  hch.get_hits65());
        assertEqual(16, hch.get_total_hits());
        assertEqual(16, hch.get_hold_hits(0));

        ch1.append(hit(400, 1, 70, 1, 100));
        ch1.append(hit(550, 1, 70, 1, 100));
        ch1.append(hit(550, 1, 55, 1, 100));
        ch1.append(hit(650, 1, 55, 1, 100));
        ch1.append(hit(650, 1, 55, 1, 100));

        hch = ch1.calculate();

        assertEqual(548, hch.get_duration());
        assertEqual(  2, hch.get_hits65());
        assertEqual( 21, hch.get_total_hits());
        assertEqual( 16, hch.get_hold_hits(0));
        assertEqual(  2, hch.get_hold_hits(1));
    }

    UNITTEST( test_one_channel_data )
    {
        boost::array<double, 37> test1=
        {
            34, 246, 39, 0, 246, 38, 18, 299, 31, 3,
            11, 105, 125, 110, 95, 68, 93, 81, 75, 92,
            94, 86, 90, 81, 20, 19, 36, 15, 37, 33,
            5, 3, 14, 2, 4, 1, 4
        };

        calculator ch1(&c_, 1);

        std::for_each(STL_II(test1), bind(&monpac_test::append, &ch1, _1));

        channel hch=ch1.calculate();

        assertDoublesEqual(1.042, hch.h_index(), 0.001);
        assertDoublesEqual(150.5, hch.s_index(), 0.1);
        assertEqual(zip_c, hch.get_zip());

    }

    void test_h_index(std::vector<double> const &data, double h_index, debug::SourcePos const &pos)
    {
        calculator ch(&c_, 1);
        std::for_each(STL_II(data), bind(&monpac_test::append, &ch, _1));

        channel h=ch.calculate();
        assertDoublesEqualAt(h_index, h.h_index(), 0.00000001, pos.get_string());

    }

    UNITTEST(test_traced_h_index)
    {
        std::vector<double> data(2000, 1); data[1900]=51;
        test_h_index(data, 1.0*2000/2050*250/200, HERE);
    }

} INSTANCE;
