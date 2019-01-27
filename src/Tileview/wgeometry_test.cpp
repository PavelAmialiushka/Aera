#include "stdafx.h"

#include "wgeometry.h"
#include "operators.h"

#include "page_test.h"
#include "utilites/unittest.h"
#include "utilites/foreach.hpp"

#include "utilites/serl/Archive.h"
#include "utilites/serl/datanode.h"

//////////////////////////////////////////////////////////////////////////

enum { m = point::max };

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( Line_test )
{

private:

    Page *page;

public:

    enum {m=point::max};

    void setUp()
    {
        page = Page_test::create_page();
    }

    void tearDown()
    {
        delete page; page=0;
    }

    UNITTEST( constructors )
    {
        Line lineA=Line(page, point(.000,.000), point(.999,.000), bottom);
        assertEqual(line(point(.000,.000),point(.999,.000)), lineA.get_line());

        Line lineB=Line(page, point(.333,.500), point(.667,.500), bottom);
        assertEqual(line(point(.333,.500), point(.667,.500)), lineB.get_line());

        assertEqual(point(.333,.500), lineB.get_points()->at(0));
        assertEqual(point(.667,.500), lineB.get_points()->at(1));

        line ln(.999,.000,.000,.000);
        Line lineC=Line(page, ln.parallel_copy(point(.000,.500)), bottom);
        assertEqual(line(.999,.500,.000,.500), lineC.get_line());
    }

    UNITTEST( copy_construct_exception )
    {
        line ln(m/3, m/2, 2*m/3, m/2);

        assertBlock
        {
            Line ltop=Line(page, ln, top);
            assertEqual(ln, (line)ltop);
        } assertException(Line::creation_error);
    }

    UNITTEST( copy_construct )
    {
        line ln(m/3, m/2, 2*m/3, m/2);
        Line lbottom=Line(page, ln, bottom);
        assertEqual(ln, (line)lbottom);
    }
} INSTANCE;


class UNITTEST_SUITE( SnapLine_test )
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

    UNITTEST( snap3_test )
    {
        rect load_array[]=
        {
            rect(.000,.000,.999,.500), //  ZZZZ
            rect(.000,.500,.500,.999), //  AABB
            rect(.500,.500,.999,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Line lin1=MakeSnapLine(page, point(.250, .500), top);
        assertEqual(line(.999,.500,.000,.500), lin1);

        Line lin2=MakeSnapLine(page, point(.250, .500), bottom);
        assertEqual(line(.000,.500,.999,.500), lin2);
    }

    UNITTEST( snap5_test )
    {
        rect load_array[]=
        {
            rect(.000,.000,.500,.500), //  XB
            rect(.000,.500,.500,.999), //  YC
            rect(.500,.000,.999,.333), //   D
            rect(.500,.333,.999,.666), //
            rect(.500,.666,.999,.999), //
        }; page=Page_test::create_page(STL_AA(load_array));

        Line lin1=MakeSnapLine(page, point(.550, .600), right);
        assertEqual(line(.500,.999,.500,.000), lin1);

        Line lin2=MakeSnapLine(page, point(.550, .600), left);
        assertEqual(line(.500,.000,.500,.999), lin2);
    }

    UNITTEST( minsnap_test )
    {
        rect load_array[]=
        {
            rect(.000,.000,.250,.500),
            rect(.250,.000,.500,.500),
            rect(.500,.000,.750,.500),
            rect(.750,.000,.999,.500),
            rect(.000,.500,.500,.999),
            rect(.500,.500,.999,.999)
        }; page=Page_test::create_page(STL_AA(load_array));

        Line lin1=MakeMinSnapLine(page, point(.550,.125));
        assertEqual(line(.500,.500,.500,.000), lin1);
    }

    UNITTEST( standard_test )
    {
        page=Page_test::create_page();

        Line lin1=MakeMinSnapLine(page, point(.450,.480));
        assertEqual(line(.500,.500,.000,.500), lin1);
    }
} INSTANCE;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( Areal_test )
{

private:

    scoped_ptr<Page> page;
    scoped_ptr<Areal> arealA;
    scoped_ptr<Areal> arealB;
    scoped_ptr<Areal> arealC;
    scoped_ptr<Areal> arealD;

public:

    enum {m=point::max};

    void setUp()
    {
        page.reset(Page_test::create_page());
        arealA.reset(new Areal(&*page,point(0,0),lt));
        arealB.reset(new Areal(&*page,point(m/2,m/2),rb));
        arealC.reset(new Areal(&*page,point(2*m/3,m/2),rt));
        arealD.reset(new Areal(&*page,point(2*m/3,m/2),lt));
    }

    UNITTEST( constructors )
    {
        assertEqual(rect(0,0,m,m),arealA->get_rect());
        assertEqual(2u,arealA->get_window_row(top));
        assertEqual(3u,arealA->get_window_row(left));

        assertEqual(rect(0,0,m/2,m/2),arealB->get_rect());
        assertEqual(1u,arealB->get_window_row(top));
        assertEqual(1u,arealB->get_window_row(left));

        assertEqual(rect(0,m/2,2*m/3,m),arealC->get_rect());
        assertEqual(2u,arealC->get_window_row(right));
        assertEqual(1u,arealC->get_window_row(bottom));

        assertEqual(rect(2*m/3,m/2,m,m),arealD->get_rect());
        assertEqual(1u,arealD->get_window_row(left));
        assertEqual(1u,arealD->get_window_row(bottom));
    }

    UNITTEST( get_window_row )
    {
        page.reset( Page_test::create_page(3) );
        Areal areal(page.get(), point(m, m), rb);
        assertEqual(rect(0, 0, m, m), areal.get_rect());
        assertEqual(3u, areal.get_window_row(left));
        assertEqual(1u, areal.get_window_row(bottom));
    }

    UNITTEST(constructors2)
    {
        assertEqual(rect(0,0,m,m/2),
                    Areal(page.get(),Line(page.get(),point(0,m/2),point(m,m/2),top),lb).get_rect());
        assertEqual(rect(0,0,m/2,m/2),
                    Areal(page.get(),Line(page.get(),point(0,m/2),point(m/2,m/2),top),lb).get_rect());

        Areal a(page.get(),Line(page.get(),point(m/3,m/2),point(2*m/3,m/2),bottom),lt);
        assertEqual(
            rect(m/3,m/2,2*m/3,m), a.get_rect()  );
    }

    UNITTEST(modificators)
    {
        arealC->resize(&*page,rect(0,m/2,3*m/4,m));
        assertEqual(rect(0,m/2,3*m/8,m),page->at(2)->get_rect());
        assertEqual(rect(3*m/8,m/2,3*m/4,m),page->at(3)->get_rect());

        arealD->resize(&*page,rect(3*m/4,m/2,m,m));
        assertEqual(rect(3*m/4,m/2,m,m),page->at(4)->get_rect());
    }
} INSTANCE;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

/*class UNITTEST_SUITE( Selection_test )
{

private:
  Page* page;
  scoped_ptr<SelectionRect> selection;
  scoped_ptr<DoubleSlideLine> mobil;
public:
  enum { m=point::max };

  void setUp()
  {
    page = Page_test::create_page();
  }

  void tearDown()
  {
    delete page; page=0;
  }

  UNITTEST(constructors)
  {
    assertEqual(rect(0,0,m/2,m/2),
      SelectionRect(page,point(m/16,m/32),point(m/2-1,m/2+1)).get_rect());
    assertEqual(rect(0,0,m,m/2),
      SelectionRect(page,point(m/2+1,m/32),point(m/2-1,m/2+1)).get_rect());
    assertEqual(rect(0,0,m,m),
      SelectionRect(page,point(m/16,m/32),point(m-1,m-1)).get_rect());
    assertEqual(rect(0,0,m,m/2),
      SelectionRect(page,point(m/2-1,m/2-1),point(m-1,m-1)).get_rect());
  }

  UNITTEST(line_constructor)
  {
    assertEqual(
      line(.000, .000, .500, .000),
      SelectionLine(page, point(.200,.200), point(.250,.250)).get_line());

    assertEqual(
      line(0, 0, m, 0),
      (line)SelectionLine(page, point(m/4-1, m/4-1), point(3*m/4, m/4)).get_line());

    assertEqual(
      line(m, m/2, 0, m/2),
      (line)SelectionLine(page, point(m/3, m/2-1), point(2*m/3, 1)).get_line());

    assertEqual(
      line(m/3, m/2, m/3, m),
      (line)SelectionLine(page, point(m/3-1, m-2), point(m/3-1, m-2)).get_line());
  }

  UNITTEST(modificators)
  {
    selection.reset(new SelectionRect(page,point(0,0),point(m/2+1,m/2-1)));
    mobil.reset(new DoubleSlideLine(page,point(m/2+1,m/4)));
    assertEqual(rect(0,0,m,m/2),mobil->get_resize_rect());

    mobil->resize_zone(point(m/3,m/4));
    mobil->commit(page);
  }

  UNITTEST(incorrect1)
  {
    modificators();

    assertBlock
    {
      mobil.reset(new DoubleSlideLine(page,point(m/2,m/2),selection.get()));
    } assertException(SingleSlideLine::creation_error);
  }

} INSTANCE;*/

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( Page_serial_test )
{

    void setUp()
    {
        page=0;
        other=0;
    }

    void tearDown()
    {
        delete page; page=0;
        delete other; other=0;
    }

    Page *page;
    Page *other;

    UNITTEST( serial_test )
    {
        page=Page_test::create_propeller();
        other=Page_test::create_page(5);

        std::string container;

        serl::save_archiver(
            new serl::data(container))
        .serial("page", *page);


        serl::load_archiver(
            new serl::data(container))
        .serial("page", *other);

        assertEqual(page->size(), other->size());
        for (unsigned index=0; index<page->size(); ++index)
        {
            assertEqual(*page->at(index), *other->at(index));
        }
    }

} sumka;
