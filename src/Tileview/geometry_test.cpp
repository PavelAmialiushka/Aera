#include "stdafx.h"

#include "geometry.h"
#include "wgeometry.h"

#include "utilites/unittest.h"

//////////////////////////////////////////////////////////////////////////
// Unit testing

#include "utilites\CmdLine.h"

enum { m=point::max };

class UNITTEST_SUITE( direction_test )
{

public:

    UNITTEST(constructors)
    {
        direction a=left; assertEqual((direction)left,a);
        direction b=lt;   assertEqual((direction)lt,b);
        direction c=zero; assertEqual((direction)zero,c);
    }

    UNITTEST(modificators)
    {
        direction a=left;
        assertEqual(right,-a);
        assertEqual(top,a+cw);
        assertEqual(bottom,a.rotate(ccw));
        assertEqual(lt,a|top);
        assertEqual(rt,-a|top);

        assertTrue(top&lt);
        assertTrue(top&rt);
        assertTrue(top&top);
        assertTrue(!(top&bottom));
        assertTrue(!(top&lb));
        assertTrue(!(top&rb));
        assertTrue(!(top&left));
        assertTrue(!(top&right));

        assertTrue(lt&left);
        assertTrue(lt&top);
    }

    UNITTEST(selectors)
    {
        assertEqual(left,left.get_horizontal());
        assertEqual(direction(),left.get_vertical());
        assertEqual(false,left.is_vertical());
        assertEqual(true,left.is_horizontal());

        assertEqual(bottom,rb.get_vertical());
        assertEqual(right,rb.get_horizontal());
        assertEqual(false,rb.is_vertical());
        assertEqual(false,rb.is_horizontal());

        assertEqual(bottom,lb.get_vertical());
        assertEqual(left,lb.get_horizontal());
    }
} go1;

class UNITTEST_SUITE( point_test )
{
public:

    UNITTEST(ops)
    {
        point zero(5, 5);
        assertEqual(point(10, 5), zero+vector(right, 5));
        assertEqual(point(5, 10), zero+vector(bottom, 5));
        assertEqual(point(5, 0), zero+vector(top, 5));
        assertEqual(point(0, 5), zero+vector(left, 5));
    }

} go2;

//////////////////////////////////////////////////////////////////////////
class UNITTEST_SUITE( line_test )
{

public:

    UNITTEST(constructors)
    {
        line lineA;
        assertTrue(lineA.empty());
        assertEqual(point(0,0),lineA.get_point1());
        assertEqual(point(0,0),lineA.get_point2());

        line lineB(point(0,0),point(20,30));
        assertEqual(point( 0,0),lineB.get_point1());
        assertEqual(point(20,30),lineB.get_point2());

        line lineC(point(40,60),point(10,20));
        assertEqual(point(40,60),lineC.get_point1());
        assertEqual(point(10,20),lineC.get_point2());

        assertTrue(lineA!=lineC);
        assertTrue(lineA!=lineB);
        assertTrue(lineB!=lineC);

        lineA=lineB;
        assertEqual(point( 0,0),lineA.get_point1());
        assertEqual(point(20,30),lineA.get_point2());

        line lineD(lineC);
        assertEqual(point(40,60),lineD.get_point1());
        assertEqual(point(10,20),lineD.get_point2());

        line lineE(0,0,20,30);
        assertEqual(lineB,lineE);
    }

    UNITTEST(selectors)
    {
        line lineA(0,0,50,0);
        assertEqual( right,lineA.get_direction() );
        assertEqual( vector(right,50),lineA.get_vector() );

        line lineB(10,20,10,0);
        assertEqual( top,lineB.get_direction() );
        assertEqual( vector(top,20),lineB.get_vector() );

        assertEqual( 50/2,lineA.transform(CRect(0,0,point::max/2,point::max/2)));
        assertEqual( 20/4,lineB.transform(CRect(0,0,point::max/4,point::max/4)));
    }

    UNITTEST(operators)
    {
        line lineA(0,0,50,0);

        rect cutr(5,-10,15,10);
        assertEqual(line(5,0,15,0),cutr&lineA);

        point pt1(25,10);
        assertEqual(vector(top,10),lineA-pt1);
        assertEqual(vector(bottom,10),pt1-lineA);

        point pt2(0,0);
        line lineB(100,50,100,150);
        assertEqual(vector(left,100),pt2-lineB);
        assertEqual(vector(zero,0),pt2-lineA);
        assertEqual(vector(left,75),pt1-lineB);

        assertEqual(line(0,10,50,10),lineA.parallel_copy(pt1));
        assertEqual(line(0,0,50,00), lineA.parallel_copy(pt2));

        assertEqual(line(25,50,25,150),lineB.parallel_copy(pt1));
        assertEqual(line(0,50,0,150), lineB.parallel_copy(pt2));
    }
} go3;

//////////////////////////////////////////////////////////////////////////


class UNITTEST_SUITE( rect_test )
{

private:

    scoped_ptr<rect> rectA;
    scoped_ptr<rect> rectB;
    scoped_ptr<rect> rectC;

public:

    void setUp()
    {
        rectA.reset(new rect(10,10,90,90));
        rectB.reset(new rect(20,30,40,50));
        rectC.reset(new rect(100,200,0,0));
    }

    UNITTEST(constructors)
    {
        assertEqual(rect(10,10,90,90),*rectA);
        assertEqual(rect(20,30,40,50),*rectB);
        assertEqual(rect(0,0,100,200),*rectC);
        assertTrue(*rectA!=*rectB);
        assertTrue(*rectA==*rectA);
        assertTrue(*rectA!=*rectC);

        rect temp(*rectA);
        assertEqual(*rectA,temp);

        temp=*rectB;
        assertEqual(*rectB,temp);

        assertTrue(!rectA->empty());
        assertTrue(!rectB->empty());
        assertTrue(!rectC->empty());

        temp=rect();
        assertTrue(temp.empty());
    }

    UNITTEST(modification)
    {
        rectA->shift_border(left,10);
        rectA->shift_border(bottom,10);
        assertEqual(rect(0,10,90,100),*rectA);

        rectA->shift_border(right,10);
        rectA->shift_border(top,10);
        assertEqual(rect(0,0,100,100),*rectA);

        rectB->set_corner(left,point(30,60));
        assertEqual(rect(30,30,40,50),*rectB);

        rectB->set_corner(rb,point(60,60));
        assertEqual(rect(30,30,60,60),*rectB);

        // operator
        line cr1=line(point(0,50),point(100,50));
        line temp=cr1 & (*rectA);
        assertEqual(cr1,temp);

        cr1=line(point(50,110),point(50,200));
        assertTrue((cr1& (*rectA)).empty());

        assertEqual(line(point(50,50),point(50,100)),
                    line(point(50,50),point(50,150)) & (*rectA));

    }

    UNITTEST(selectors)
    {
        assertEqual(point(20,30),rectB->get_corner(lt));
        assertEqual(point(40,30),rectB->get_corner(rt));
        assertEqual(point(20,50),rectB->get_corner(lb));
        assertEqual(point(40,50),rectB->get_corner(rb));

        // transform
        enum { m = point::max,} ;
        rect temp(CRect(1,2,3,4),CRect(0,0,5,4));
        // TODO strage error:
        //assertEqual(point(1*m/5,2*m/4),temp.get_corner(lt));
        //assertEqual(point(3*m/5,4*m/4),temp.get_corner(rb));
        //assertEqual(point(1*m/5,4*m/4),temp.get_corner(lb));
        //assertEqual(point(3*m/5,2*m/4),temp.get_corner(rt));

        //CRect r=temp.transform(CRect(0,0,10,8));
        //assertTrue(r==CRect(2,4,4,6));
    }

    UNITTEST(intersect_test)
    {
        rect a=rect(.000,.000,.999,.999);
        rect b=rect(.333,.333,.666,.666);
        rect c=rect(.100,.400,.900,.600);

        assertEqual(b, a&b);
        assertEqual(c, a&c);
        assertEqual(rect(.333,.400,.666,.600), c&b);

        assertEqual(c&b, b&c);
        assertEqual(c&a, a&c);
        assertEqual(b&a, a&b);

        rect x(.250,.500,.500,.999);
        rect y(.750,.500,.500,.999);
        assertEqual(rect(.500,.500,.500,.999), x&y);

    }
} go4;

//////////////////////////////////////////////////////////////////////////

class  UNITTEST_SUITE( algorithms_test )
{
public:

    UNITTEST(_contains_point)
    {
        contains_point pred(point(50,150));
        assertTrue(pred(rect(0,100,100,200)));
        assertTrue(pred(rect(0,150,50,200)));
        assertTrue(pred(rect(50,100,50,200)));
        assertTrue(pred(rect(50,150,100,200)));

        assertTrue(!pred(rect(51,151,100,200)));
        assertTrue(!pred(rect(0,151,100,200)));
        assertTrue(!pred(rect(0,100,49,200)));
        assertTrue(!pred(rect(49,100,51,100)));
    }

    UNITTEST(_is_part_of_rect)
    {
        is_part_of_rect pred(rect(40,140,50,150));
        assertTrue(pred(rect(40,140,50,150)));
        assertTrue(pred(rect(50,140,50,140)));
        assertTrue(pred(rect(50,145,50,145)));
        assertTrue(pred(rect(43,141,44,142)));
    }

    UNITTEST(_has_corner)
    {
        has_corner pred(point(0,0),lt);
        assertTrue(pred(rect(0,0,100,100)));
        assertTrue(pred(rect(0,0,200,500)));

        assertTrue(!pred(rect(0,0,-100,100)));
        assertTrue(!pred(rect(1,0,100,100)));

        has_corner pred2(point(100,100),rb);
        assertTrue(pred2(rect(0,0,100,100)));
        assertTrue(!pred2(rect(0,0,200,500)));
    }

    UNITTEST(closest_border_test)
    {
        rect rc(.000,.000,.500,.500);
        assertEqual(
            left,
            distance_to_border(point(.000,.250)).closest_border(rc));

    }
} go5;

//////////////////////////////////////////////////////////////////////////

class UNITTEST_SUITE( corner_test )
{
public:

    UNITTEST( first )
    {
        rect r(.000,.000,.999,.999);

        assertEqual(point(.000,.000), r * lt);
        assertEqual(line(.000,.000,.000,.999), r * left);

        r *lt=point(.500,.500);
        assertEqual(rect(.500,.500,.999,.999), r);

        r *left=point(.000,.000);
        assertEqual(rect(.000,.500,.999,.999), r);
    }

} go6;
