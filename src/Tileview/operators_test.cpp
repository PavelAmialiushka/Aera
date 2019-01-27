#include "stdafx.h"

#include "operators.h"

#include "page_test.h"
#include "utilites/unittest.h"

class UNITTEST_SUITE( inserter_test )
{

    Page *page;

    void setUp()
    {
        page=0;
    }

    void tearDown()
    {
        delete page;
    }

public:

    UNITTEST( test_inserter )
    {
        page=Page_test::create_page(4);

        Inserter ins(page, point(.250,.550));
        ins.track_point(point(.250,.800));

        rect ar[5]=
        {
            rect(.000,.000,.500,.500),
            rect(.500,.000,.999,.500),
            rect(.000,.800,.500,.999), // сжатое
            rect(.500,.500,.999,.999),
            rect(.000,.500,.500,.800), // новое
        }; Page_test::assert_equal(ins.get_page(), STL_AA(ar), HERE);

        ins.track_point(point(.750, .300));
        rect ar2[]=
        {
            rect(.000,.000,.500,.300), // сжатое
            rect(.500,.000,.999,.300), // сжатое
            rect(.000,.500,.500,.999),
            rect(.500,.500,.999,.999),
            rect(.000,.300,.999,.500), // новое окно
        }; Page_test::assert_equal(ins.get_page(), STL_AA(ar2), HERE);

        ins.commit( page );
        Page_test::assert_equal(page, STL_AA(ar2), HERE);
    }

    UNITTEST( test_again )
    {
        rect load_array[]=
        {
            rect(.000,.000,.999,.500), //  ZZZZ
            rect(.000,.500,.500,.999), //  AABB
            rect(.500,.500,.999,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Inserter ins(page, point(.250, .500));
        ins.track_point(point(.250,.250));

        rect test_array[]=
        {
            rect(.000,.000,.999,.250), // ZZZZ
            rect(.000,.500,.500,.999), // XXXX
            rect(.500,.500,.999,.999), // AABB
            rect(.000,.250,.999,.500), // AABB
        };
        Page_test::assert_equal(ins.get_page(), STL_AA(test_array), HERE);
    }

    UNITTEST( another5_test)
    {
        rect load_array[]=
        {
            rect(.000,.000,.500,.500), //  XXB
            rect(.000,.500,.500,.999), //  YYC
            rect(.500,.000,.999,.333), //  YYD
            rect(.500,.333,.999,.666), //
            rect(.500,.666,.999,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Inserter ins(page, point(.450, .600));
        ins.track_point(point(.750,.650));

        rect test_array[]=
        {
            rect(.000,.000,.500,.500), //
            rect(.000,.500,.500,.999), //
            rect(.500,.000,.999,.333), //
            rect(.750,.333,.999,.666), //
            rect(.500,.666,.999,.999), //
            rect(.500,.333,.750,.666), //
        }; Page_test::assert_equal(ins.get_page(), STL_AA(test_array), HERE);

    }

    UNITTEST( incorrect_window_size_test )
    {
        rect load_array[]=
        {
            rect(.000,.000,.500,.500), //  ZZCC
            rect(.500,.000,.999,.999), //  ABCC
            rect(.000,.500,.250,.999), //
            rect(.250,.500,.500,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Inserter ins(page, point(.250, .750));
        ins.track_point(point(.750,.760));

        Page_test::assert_equal(ins.get_page(), STL_AA(load_array), HERE);

        ins.commit(page);
        Page_test::assert_equal(page, STL_AA(load_array), HERE);
    }

    UNITTEST( over_shrinked_window )
    {
        rect load_array[]=
        {
            rect(.000,.000,.050,.999), //  ZZCC
            rect(.050,.000,.950,.999), //  ABCC
            rect(.950,.000,.999,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Inserter ins(page, point(.950, .500));
        ins.track_point(point(.150,.500));
        Page_test::assert_equal(ins.get_page(), STL_AA(load_array), HERE);
    }

    UNITTEST( thelast_test )
    {
        rect load_array[]=
        {
            rect(.000,.000,.333,.500), //  ZZCC
            rect(.333,.000,.666,.250), //  ABCC
            rect(.333,.250,.666,.500), //
            rect(.666,.000,.999,.500), //
            rect(.000,.500,.250,.999), //
            rect(.250,.500,.500,.999), //
            rect(.500,.500,.999,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Inserter ins(page, point(.400, .550));
        ins.track_point(point(.400,.250));

        rect test_array[]=
        {
            rect(.000,.000,.333,.500), //  ZZCC
            rect(.333,.000,.666,.125), //  ABCC
            rect(.333,.125,.666,.250), //
            rect(.666,.000,.999,.500), //
            rect(.000,.500,.250,.999), //
            rect(.250,.500,.500,.999), //
            rect(.500,.500,.999,.999), //
            rect(.333,.250,.666,.500), // 8th
        }; Page_test::assert_equal(ins.get_page(), STL_AA(test_array), HERE);
    }

} go;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( Deleter_test )
{
public:
    Page *page;

    void setUp()
    {
        page=0;
    }
    void tearDown()
    {
        delete page; page=0;
    }

    UNITTEST(edge_delete)
    {
        page=Page_test::create_page(5);

        Deleter del(page, point(.333,.333));

        rect sr[]=
        {
            rect(.000,.000,.999,.500),
            rect(.000,.500,.333,.999),
            rect(.333,.500,.666,.999),
            rect(.666,.500,.999,.999),
        };
        Page_test::assert_equal(page, sr, ENDOF(sr), HERE);
    }

    UNITTEST(inner_delete)
    {
        page=Page_test::create_page(5);

        Deleter del(page, point(.500, .666));

        rect sr[]=
        {
            rect(.000,.000,.500,.500),
            rect(.500,.000,.999,.500),
            rect(.000,.500,.500,.999),
            rect(.500,.500,.999,.999),
        };
        Page_test::assert_equal(page, STL_AA(sr), HERE);
    }

    UNITTEST( vertical_delete )
    {
        rect init[]=
        {
            rect(.000,.000,.999,.500), // AAAA
            rect(.000,.500,.333,.999), // BDDC
            rect(.666,.500,.999,.999), // BEEC
            rect(.333,.500,.666,.666), // BFFC
            rect(.333,.666,.666,.833),
            rect(.333,.833,.666,.999),
        }; page=Page_test::create_page(STL_AA(init));

        Deleter del(page, point(.500, .800));

        rect test[]=
        {
            rect(.000,.000,.999,.500), // AAAA
            rect(.000,.500,.333,.999), // BDDC
            rect(.666,.500,.999,.999), // BFFC
            rect(.333,.500,.666,.750),
            rect(.333,.750,.666,.999),
        }; Page_test::assert_equal(page, STL_AA(test), HERE);
    }

    UNITTEST( delete5_test)
    {
        page=Page_test::create_page(5);

        Deleter(page, point(.250,.250));
        Deleter(page, point(.250,.250));

        Deleter(page, point(.500,.250));
    }

} go1nm ;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( SingleSlideLine_test )
{
private:

    Page *page;
    Line  linea;
    Line  lineb;

public:

    void setUp()
    {
        page=Page_test::create_page();
        linea=Line(page,line(.000,.500,.999,.500),top);
        lineb=Line(page,line(.333,.500,.333,.999),left);
    }

    void tearDown()
    {
        delete page; page=NULL;
    }

    UNITTEST(impl1)
    {
        /*    SingleSlideLineImpl mli(page,linea,top);
            mli.resize(page,point(.500,.333));
            assertEqual( rect(.000,.000,.500,.333),page->at(0)->get_rect() );
            assertEqual( rect(.500,.000,.999,.333),page->at(1)->get_rect() );
            assertEqual( rect(.000,.500,.333,.999),page->at(2)->get_rect() );
            assertEqual( rect(.333,.500,.666,.999),page->at(3)->get_rect() );
            assertEqual( rect(.666,.500,.999,.999),page->at(4)->get_rect() );*/
    }

    UNITTEST(impl2)
    {
        /*SingleSlideLineImpl mli(page,lineb,left);
        mli.resize(page,point(.166,.999));
        assertEqual( rect(.000,.000,.500,.500),page->at(0)->get_rect() );
        assertEqual( rect(.500,.000,.999,.500),page->at(1)->get_rect() );
        assertEqual( rect(.000,.500,.166,.999),page->at(2)->get_rect() );
        assertEqual( rect(.333,.500,.666,.999),page->at(3)->get_rect() );
        assertEqual( rect(.666,.500,.999,.999),page->at(4)->get_rect() );*/
    }

    UNITTEST(rollback1)
    {
        SingleSlideLine ml(page,linea,top);
        ml.track(point(0,0));
        ml.rollback();

        ml.track(point(.000,.333));
        ml.commit();

        assertEqual( rect(.000,.000,.500,.333),page->at(0)->get_rect() );
        assertEqual( rect(.500,.000,.999,.333),page->at(1)->get_rect() );
        assertEqual( rect(.000,.500,.333,.999),page->at(2)->get_rect() );
        assertEqual( rect(.333,.500,.666,.999),page->at(3)->get_rect() );
        assertEqual( rect(.666,.500,.999,.999),page->at(4)->get_rect() );
    }

} gob;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE(XSingleSlideLine_test)
{
    Page *page;
public:
    void setUp()
    {
        page=Page_test::create_page(6);
    }

    void tearDown()
    {
        delete page; page=0;
    }

    UNITTEST( normal_span_test )
    {
        XSlideLine slin1(page, point(.333,.750));
        assertEqual(line(.333,.000,.333,.999), slin1.get_line());

        slin1.track(point(.500,.750));
        slin1.commit();

        rect array[]=
        {
            rect(.000,.000,.500,.500),
            rect(.500,.000,.750,.500),
            rect(.750,.000,.999,.500),
            rect(.000,.500,.500,.999),
            rect(.500,.500,.750,.999),
            rect(.750,.500,.999,.999),
        }; Page_test::assert_equal(page, STL_AA(array), HERE);
    }

    UNITTEST( min_span_test )
    {
        XSlideLine slin3(page, point(.333,.750), true, true, &MakeMinSnapLine);
        assertEqual(line(.333,.500,.333,.999), slin3.get_line());

        slin3.track(point(.500,.750));
        slin3.commit();

        rect array[]=
        {
            rect(.000,.000,.333,.500),
            rect(.333,.000,.666,.500),
            rect(.666,.000,.999,.500),
            rect(.000,.500,.500,.999),
            rect(.500,.500,.750,.999),
            rect(.750,.500,.999,.999),
        }; Page_test::assert_equal(page, STL_AA(array), HERE);
    }

    UNITTEST( standard_test )
    {
        delete page; page=Page_test::create_page();

        XSlideLine slin2(page, point(.450,.500), true, true, &MakeMinSnapLine);
        assertEqual(line(.999,.500,.000,.500), slin2.get_line());

        //yields invalid page

        //sline.track(point(.500,.750));
        //sline.commit();
    }

    UNITTEST( propeller_test )
    {
        delete page; page=Page_test::create_propeller();

        XSlideLine slin1(page, point(.500,.750), false, true, &MakeMinSnapLine);

    }

} INSTANCE;


//////////////////////////////////////////////////////////////////////////

/*class UNITTEST_SUITE( DoubleSlideLine_test )
{
private:
  Page* page;
  Page* page12;
  Line  snapl;
  Line  snaplB;

  scoped_ptr<DoubleSlideLine> zoneA; // horizontal line
  scoped_ptr<DoubleSlideLine> zoneB; // second vertical line
public:

  void setUp()
  {
    scoped_ptr<int> pi;
    pi.reset(new int(1));

    page=Page_test::create_page();
    page12=Page_test::create_page12();
    snapl=MakeSnapLine(page, point(.450,.475));

    zoneA.reset(new DoubleSlideLine(page, snapl));

    snaplB=MakeSnapLine(page, point(.700,.950) );
    zoneB.reset( new DoubleSlideLine(page, snaplB) );
  }

  void tearDown()
  {
    delete page; page=0;
    delete page12; page12=0;
  }

  UNITTEST(constructors)
  {
    //A
    assertEqual(line(point(.999,.500),point(.000,.500)), snapl);
    assertTrue( snapl.get_direction().is_horizontal() );
    assertEqual(rect(.000,.000,.999,.999),zoneA->get_resize_rect());

    assertEqual(rect(.000,.000,.999,.500),zoneA->get_areals()[0].get_rect());
    assertEqual(rect(.000,.500,.999,.999),zoneA->get_areals()[1].get_rect());

    //B
    assertTrue( snaplB.get_direction().is_vertical() );
    assertEqual(line(point(.666,.999),point(.666,.500)), snaplB);
    assertEqual(rect(.000,.500,.999,.999),zoneB->get_resize_rect());

    assertEqual(rect(.666,.500,.999,.999),zoneB->get_areals()[0].get_rect());
    assertEqual(rect(.000,.500,.666,.999),zoneB->get_areals()[1].get_rect());
  }

  UNITTEST(modificators)
  {
    zoneA->resize_zone(point(.333,.333));
    zoneA->commit(page);
    rect sample1[]=
    {
      rect(.000,.000,.500,.333),
      rect(.500,.000,.999,.333),
      rect(.000,.333,.333,.999),
      rect(.333,.333,.666,.999),
      rect(.666,.333,.999,.999),
    };  Page_test::assert_equal(page, sample1, sample1+5, HERE);

    zoneA->resize_zone(point(.200,.200));
    zoneA->commit(page);
    assertEqual(rect(.000,.000,.500,.200),page->at(0)->get_rect());
    assertEqual(rect(.500,.000,.999,.200),page->at(1)->get_rect());
    assertEqual(rect(.000,.200,.333,.999),page->at(2)->get_rect());

    zoneA->resize_zone(point(.500,.500));
    zoneA->commit(page);
    assertEqual(rect(.000,.000,.500,.500),page->at(0)->get_rect());
  }

  UNITTEST(modificators2)
  {
    // not committed
    assertEqual(rect(.666,.500,.999,.999),zoneB->get_areals()[0].get_rect());
    zoneB->resize_zone(point(.000,.000));

    rect sample2[]=
    {
      rect(.000,.000,.500,.500),
      rect(.500,.000,.999,.500),
      rect(.000,.500,.333,.999),
      rect(.333,.500,.666,.999),
      rect(.666,.500,.999,.999)
    }; Page_test::assert_equal(page, sample2, sample2+5, HERE);

    // rollback and resize
    zoneB->resize_zone(point(.333,.333));
    zoneB->commit(page);
    assertEqual(rect(.333,.500,.999,.999),zoneB->get_areals()[0].get_rect());

    rect sample3[]=
    {
      rect(.000,.000,.500,.500),
      rect(.500,.000,.999,.500),
      rect(.000,.500,.166,.999),
      rect(.166,.500,.333,.999),
      rect(.333,.500,.999,.999)
    }; Page_test::assert_equal(page, sample3, sample3+5, HERE);
  }

  UNITTEST(modificators3)
  {
    snapl= MakeSnapLine(page,point(.550,.250));
    zoneA.reset( new DoubleSlideLine(page, snapl) );
    zoneA->resize_zone( point(.333,.250) );
    zoneA->commit( page );
  }

  UNITTEST(exception1)
  {
    modificators3();

    assertBlock
    {
      Line line(page,point(.333,.500),point(.666,.500),bottom);
      zoneB.reset( new DoubleSlideLine(page,line) );
    } assertException(SingleSlideLine::creation_error);
  }

  UNITTEST(exception2)
  {
    modificators3();
    assertBlock
    {
      Line line(page,point(.666,.500),point(.999,.500),bottom);
      zoneB.reset( new DoubleSlideLine(page,line));
    } assertException(SingleSlideLine::creation_error);
  }

  UNITTEST(double_move_12_1)
  {
    DoubleSlideLine mobz(page12,Line(page12,
      line(.750,.000,.750,.999),
      left));
    mobz.resize_zone( point(.250,.000) );
    mobz.commit( page12 );
    {
      rect sample_rects[]=
      {
        rect(.000,.000,.083,.333),
        rect(.083,.000,.166,.333),
        rect(.166,.000,.250,.333),
        rect(.250,.000,.999,.333),

        rect(.000,.333,.083,.666),
        rect(.083,.333,.166,.666),
        rect(.166,.333,.250,.666),
        rect(.250,.333,.999,.666),

        rect(.000,.666,.083,.999),
        rect(.083,.666,.166,.999),
        rect(.166,.666,.250,.999),
        rect(.250,.666,.999,.999),
      };
      Page_test::assert_equal(page12,sample_rects,sample_rects+12,HERE);
    }

    // second
    {
      DoubleSlideLine mobz(page12, Line(page12,
        line(.250,.000,.250,.999),
        left));
      mobz.resize_zone( point(.833,.000) );
      mobz.commit( page12 );
      {
        rect sample_rects[]=
        {
          rect(.000,.000,.278,.333),
          rect(.278,.000,.555,.333),
          rect(.555,.000,.833,.333),
          rect(.833,.000,.999,.333),

          rect(.000,.333,.278,.666),
          rect(.278,.333,.555,.666),
          rect(.555,.333,.833,.666),
          rect(.833,.333,.999,.666),

          rect(.000,.666,.278,.999),
          rect(.278,.666,.555,.999),
          rect(.555,.666,.833,.999),
          rect(.833,.666,.999,.999),
        };
        Page_test::assert_equal(page12,sample_rects,sample_rects+12,HERE);
      }
    }
  }

  UNITTEST(propeller_zone)
  {
    delete page; page=Page_test::create_propeller();

    Line snl=MakeSnapLine(page, point(.450, .500));
    DoubleSlideLine mz(page, snl);

    mz.resize_zone(point(.250,.500));
    mz.commit(page);

    rect sample[]={
      rect(.000,.000,.750,.250),
      rect(.750,.000,.999,.500),
      rect(.000,.250,.250,.999),
      rect(.250,.250,.750,.500),
      rect(.250,.500,.999,.999)
    }; Page_test::assert_equal(page,STL_AA(sample),HERE);
  }
} goa;*/

