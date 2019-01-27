#include "StdAfx.h"

#include "wgeometry.h"

#include "utilites/foreach.hpp"
#include "math.h"


//////////////////////////////////////////////////////////////////////////

line operator*(pwindow_t win, border brd)
{
    return win->get_rect() * brd;
}

line operator*(border brd, pwindow_t win)
{
    return win->get_rect() * brd;
}

point operator*(pwindow_t win, corner crn)
{
    return win->get_rect() * crn;
}

point operator*(corner crn, pwindow_t win)
{
    return win->get_rect() * crn;
}


//////////////////////////////////////////////////////////////////////////

Line::Line()
{
}

Line::Line(Page *base, const line &ln, direction side)
    : side_(side)
{
    debug::Assert<creation_error>( side!=zero );
    make_2_points(base, ln.get_point1(), ln.get_point2(), side);
}

Line::Line(Page *base, point corner, direction dir, direction side)
    : side_(side)
{
    make_xray(base, corner, dir, side);
}

Line::Line(Page *base, const point &a, const point &b, direction side)
    : side_(side)
{
    make_2_points(base, a, b, side);
}

void Line::swap(Line &l)
{
    line::swap(l);
    nodes_.swap(l.nodes_);
    std::swap(side_, l.side_);
}

Line &Line::operator=(Line ln)
{
    swap(ln); return *this;
}


void Line::make_xray(Page *base, point corner, direction dir, direction side)
{
    side_=side;
    set_point1(corner);
    nodes_.reset( new points_t );
    nodes_->push_back(corner);
    point temp= step_from_corner(base, corner, dir, side);

    for (; temp!=corner;)
    {
        corner=temp;
        temp=step_from_corner(base, temp, dir, side);
        nodes_->push_back(corner);
    }
    set_point2(corner);
}

void Line::make_2_points(Page *base, const point &a, const point &b, direction side)
{
    side_=side;
    nodes_.reset(new points_t);
    direction dir=b-a;

    debug::Assert<creation_error>(dir!=zero, HERE);
    debug::Assert<creation_error>(a.x==b.x || a.y==b.y, HERE);
    debug::Assert<creation_error>(
        std::count_if(
            base->begin(), base->end(),
            has_corner(a, -(side|dir) )), HERE );

    set_point1(a);

    point corner=a;

    nodes_->push_back(corner);
    point temp=corner; //= step_from_corner(base, corner, dir, side);

    if (dir!=zero)
    {
        do
        {
            corner=temp;
            temp=step_from_corner(base, temp, dir, side);
            nodes_->push_back(temp);
        }
        while (temp!=b && temp!=corner);
    }
    set_point2(temp);
    debug::Assert<creation_error>( temp==b, HERE );
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////


point step_from_corner(Page *base, point corner, direction dir, direction side)
{
    // находит следующий узел, лежащий на линии
    // в направлении dir от corner со стороны side
    // начальный узел должен существовать на этой стороне

    direction cd=-(dir|side);

    pwindow_t best;
    foreach(pwindow_t win, *base)
    {
        if (has_corner(corner, cd)(win))
        {
            if (!best)
            {
                best=win;
            }
            else
            {
                if (
                    (win->get_rect().get_corner(dir|-side) - corner).get_length() <
                    (best->get_rect().get_corner(dir|-side) - corner).get_length())
                {
                    best=win;
                }
            }
        }
    }
    if (best)
    {
        corner=best->get_rect().get_corner(dir|-side);
    };
    return corner;
}

point extend_line(Page *base, point corner, direction dir)
{
    // находит максимально возможную линию от corner
    // в направлении dir.
    // линия может не существовать на одной из сторон

    direction side;
    point temp=
        step_from_corner(base, corner, dir, side=dir.rotate(cw));
    if (temp==corner)
    {
        temp=step_from_corner(base, corner, dir, side=dir.rotate(ccw));
    }

    for (; temp!=corner;)
    {
        corner=temp;
        temp=step_from_corner(base, temp, dir, side);
    }
    return corner;
}

point land_point(Page *base, point corner, direction dir)
{
    //
    point next_cw=step_from_corner(base, corner, dir, dir+cw);
    point next_ccw=step_from_corner(base, corner, dir, dir+ccw);

    if (next_cw!=corner && next_ccw!=corner)
    {
        // с обоих сторон перемещение удалось,
        // значить узел существует
        return corner;
    }
    else
    {

        // предел
        point end=extend_line(base, corner, dir);

        // движемся в обратном направлении
        point back_cw=step_from_corner(base, end, -dir, dir+cw);
        point back_ccw=step_from_corner(base, end, -dir, dir+ccw);

        if (back_cw==end || back_ccw==end)
        {
            // если линия существует только
            // с одной стороны
            return corner;
        }

        point prev=end;
        for (; back_cw - corner == -dir; )
        {
            back_cw=back_ccw=prev;
            back_cw=step_from_corner(base, prev, -dir, dir+cw);
            back_ccw=step_from_corner(base, prev, -dir, dir+ccw);

            while (back_cw!=back_ccw)
            {
                if (back_cw - end < back_ccw - end)
                {
                    back_cw=step_from_corner(base, back_cw, -dir, dir+cw);
                }
                else
                {
                    back_ccw=step_from_corner(base, back_ccw, -dir, dir+ccw);
                }
            }
        }
        return prev;
    }
}

Line MakeMinSnapLine(Page *base, point pt, border dr, bool doublesized)
{
    contains_point contains(pt);
    distance_to_border distance(pt);

    std::vector<pwindow_t> vector;
    foreach(pwindow_t win, *base)
    {
        if (dr==zero)
        {
            if (contains(win))
                vector.push_back(win);
        }
        else
        {
            if ( line(win->get_rect() * (dr+back)).contains_projection(pt))
                vector.push_back(win);
        }
    }

    if (vector.empty())
    {
        return Line(base, point(), point(), zero);
        debug::Assert<Line::creation_error>(!vector.empty(), HERE);
    }

    std::sort(STL_II(vector), distance);

    pwindow_t window=vector.front();
    foreach(pwindow_t win, vector)
    {
        direction side=distance.closest_border(win);
        if (side==dr+back)
        {
            window=win;
            break;
        }
    }
    direction side=distance.closest_border(window);
    point pt1=( window->get_rect().get_corner( side.rotate(fl) ));
    point pt2=( window->get_rect().get_corner( side.rotate(fr) ));

    if (doublesized)
    {
        pt1=land_point(base, pt1, side+ccw);
        pt2=land_point(base, pt2, side+cw);
    }

    return Line(base, pt1, pt2, -side);
}

Line MakeSnapLine(Page *base, point pt, border bdr, bool doublesized)
{
    Line result=MakeMinSnapLine(base, pt, bdr, doublesized);

    if (!result.empty())
    {
        direction side=result.get_side();
        point a=extend_line(base, result.get_point1(), -result.get_direction());
        point b=extend_line(base, result.get_point2(), result.get_direction());
        result=Line(base, a, b, side);
    }
    return result;
}

//////////////////////////////////////////////////////////////////////////

Areal::Areal(Page *base, const point &cnr, corner dir, bool maxim)
    : localpage_( base->clone() )
{
    create_from_corner(base, cnr, dir, maxim);
}

Areal::Areal(Page *base, const Line &ln, border dir, bool maxim)
    : localpage_( base->clone() )
{
    maxim
    ? create_from_line_max(base, ln, dir)
    : create_from_line_min(base, ln, dir);
}

Areal::Areal(const Areal &copy)
    : localpage_( copy.localpage_->clone() )
{
    root_=copy.root_;
    rect_=copy.rect_;
    width_=copy.width_;
}

Areal &Areal::operator=(Areal temp)
{
    swap(temp); return *this;
}

void Areal::swap(Areal &copy)
{
    localpage_.swap(copy.localpage_);
    std::swap(root_, copy.root_);
    std::swap(rect_, copy.rect_);
    std::swap(width_, copy.width_);
}


void Areal::create_from_corner(Page *base, const point &corner, direction dir, bool maxim)
{
    maxim
    ? Areal::create_from_line_max(base, Line(base, corner, (-dir).rotate(1),(-dir).rotate(-1)), dir)
    : Areal::create_from_line_min(base, Line(base, corner, (-dir).rotate(1),(-dir).rotate(-1)), dir);
}

void Areal::create_from_line_min(Page *base, const Line &ln, direction dir)
{
    root_=dir;
    point corner=ln.get_point1();
    windows_t::iterator index=
        std::find_if(
            base->begin(),
            base->end(),
            has_corner(corner, dir)
        );
    if (index==base->end())
    {
        rect_=rect(0,0,0,0);
        return;
    }
    rect_=(*index)->get_rect();
    collect_windows();
}

void Areal::create_from_line_max(Page *base, const Line &ln, direction dir)
{
    root_=dir;
    point corner=ln.get_point1();
    windows_t::iterator index=
        std::find_if(
            base->begin(),
            base->end(),
            has_corner(corner, dir)
        );
    if (index==base->end())
    {
        rect_=rect(0,0,0,0);
        return;
    }
    rect_=rect(corner, corner);
    direction v=(-dir).rotate( 1);
    direction h=(-dir).rotate(-1);
    if (ln.get_direction()==h)
    {
        std::swap(v, h);
    }
    debug::Assert<>(ln.get_direction()==v, "Areal: creation error", HERE);
    ppoints_t primary=ln.get_points();

    ppoints_t secondary=Line(base, corner, h, v) . get_points();
    for (int _i2=secondary->size()-1; _i2>0; --_i2)
    {
        point &_v2=secondary->at(_i2);

        ppoints_t thirdary=Line(base, secondary->at(_i2), v, -h) . get_points();
        for (int _i3=thirdary->size()-1; _i3>0; --_i3)
        {
            point &_v3=thirdary->at(_i3);

            ppoints_t forthary=Line(base, thirdary->at(_i3), -h, -v) . get_points();
            for (int _i4=forthary->size()-1; _i4>0; --_i4)
            {
                point &_v4=forthary->at(_i4);

                points_t::iterator index=
                    std::find(primary->begin(), primary->end(),
                              forthary->at(_i4));
                if (index!=primary->end())
                {
                    rect_.set_corner(-dir, thirdary->at(_i3));
                    _i2=_i3=0;
                    break;
                }
            }
        }
    }
    collect_windows();
}

void Areal::collect_windows()
{
    localpage_.reset( new Page(localpage_->begin(), localpage_->end(), is_part_of_rect(get_rect())) );
}

int Areal::calculate_window_row(direction basic) const
{
    int width=0;
    std::vector<pwindow_t> vassals;
    stdex::copy_if(
        localpage_->begin(), localpage_->end(),
        std::back_inserter(vassals),
        is_part_of_rect(get_rect())
    );

    direction corner_dir=basic.rotate(br);

    std::vector<point> points;
    std::transform(
        vassals.begin(), vassals.end(),
        std::back_inserter(points),
        get_corner(corner_dir));

    for (
        std::vector<point>::iterator index=points.begin();
        index!=points.end();
        ++index)
    {
        line cross=
            line(
                get_rect().get_corner(basic+fl),
                get_rect().get_corner(basic+bl)
            ).parallel_copy(*index);

        int counter=0;
        for (
            std::vector<pwindow_t>::iterator window=vassals.begin();
            window!=vassals.end();
            ++window)
        {
            line intersection=cross & (*window)->get_rect();
            line test_line=(*window)->get_rect().get_border(basic.rotate(-2));
            if (!intersection.empty() && intersection!=test_line)
            {
                ++counter;
            }
        }
        width=max(width, counter);
    }
    return width;
}

void Areal::resize(Page *base, const rect &rc)
{
    rect prevrc=get_rect();
    rect_=rc;

    debug::Assert<resizing_error>(!(prevrc & rc).empty(), HERE);

    unsigned size=localpage_->end()-localpage_->begin();
    for (unsigned index=0; index<size; ++index)
    {
        is_part_of_rect part_of(prevrc);
        if (part_of(localpage_->at(index)))
        {
            pwindow_t win=localpage_->at(index);

            rect prevr=win->get_rect();
            win->scale(prevrc, rc);
            rect currr=win->get_rect();

            debug::Assert<resizing_error>(!currr.empty(), HERE);
        }
    }

    base->assign_layout(localpage_.get());
}

//////////////////////////////////////////////////////////////////////////

SelectionRect::SelectionRect(Page *bz, const point &a, const point &b)
    : Selection(a, b)
{
    create(bz);
}

void SelectionRect::create(Page *base)
{
    Page smaller(base->begin(), base->end(), intersects_rect(rect(get_point1(), get_point2())));

    windows_t::iterator
    primary=std::find_if(
                smaller.begin(), smaller.end(),
                contains_point(get_point1()));

    debug::Assert<>(primary!=smaller.end(), "SelectionRect: error while selecting windows", HERE);

    corner dir=(corner)((get_point1()-get_point2()));
    if (dir==zero) dir=lt;
    if (dir.is_horizontal() || dir.is_vertical()) dir=dir.rotate(1);

    point crn = (*primary)->get_rect().get_corner(dir);
    Areal areal(&smaller, crn, (corner)dir);

    rect_=areal.get_rect();
}


void SelectionRect::track_point(Page *base, const point &pt)
{
    Selection::track_point(pt);
    create(base);
}

Page *SelectionRect::get_subpage(Page *base) const
{
    return new Page(base->begin(), base->end(), is_part_of_rect(get_rect()));
}


//////////////////////////////////////////////////////////////////////////

Selection::Selection(const point &a, const point &b)
    : point1_(a),
      point2_(b),
      freezed_(false)
{ }

void Selection::track_point(const point &pt )
{
    debug::Assert<>(!is_freezed(), "Selection is freezed", HERE);
    point2_=pt;
}

void Selection::freeze()
{
    freezed_=true;
}

bool  Selection::is_freezed() const
{
    return freezed_;
}

point Selection::get_point1() const
{
    return point1_;
}

point Selection::get_point2() const
{
    return point2_;
}

//////////////////////////////////////////////////////////////////////////

SelectionLine::SelectionLine(Page *base, const point &a, const point &b)
    : Selection(a, b)
{
    create(base);
}

void SelectionLine::create(Page *base)
{
    Page vassals(
        base->begin(), base->end(),
        intersects_rect(rect(get_point1(), get_point2())));

    direction dr=MakeSnapLine(&vassals, get_point1()).get_direction();

    dr = get_point1().get_distance(get_point2(), dr+cw) > 0
         ? dr+ccw
         : dr+cw;


    line_=MakeSnapLine(&vassals, get_point1(), dr);
}

Line SelectionLine::get_line() const
{
    return line_;
}

void SelectionLine::track_point(Page *base, const point &pt)
{
    Selection::track_point(pt);
    create(base);
}

//////////////////////////////////////////////////////////////////////////

SelectionLineEx::SelectionLineEx(Page *base, const point &a)
    : point1_(a)
{
    track_point(a);
}

void SelectionLineEx::track_point(const point &pt2)
{
    Page local(STL_II(*page_), intersects_rect(rect(point1_, pt2)));

    direction dr=MakeSnapLine(&local, point1_).get_side();

    dr = point1_.get_distance(pt2, dr) > 0
         ? dr
         : dr+back;

    line_=MakeSnapLine(&local, point1_, dr);
}

//////////////////////////////////////////////////////////////////////////

int distance_to_border::distance(const rect &rc)
{
    return
        std::min(
            rc.left_<=pt.x && pt.x < rc.right_
            ? std::min( abs( pt.y - rc.top_ ), abs( pt.y - rc.bottom_ ) )
            : rect::max,
            rc.top_ <= pt.y && pt.y < rc.bottom_
            ? std::min( abs( pt.x - rc.left_ ), abs( pt.x - rc.right_ ) )
            : rect::max
        );
}

int distance_to_border::distance(pwindow_t win)
{
    const rect &rc=win->get_rect();
    return distance(rc);
}

direction distance_to_border::closest_border(const rect &rc)
{
    int d=distance(rc);
    double yy=1.*(pt.y-rc.top_)/(rc.bottom_-rc.top_)-0.5;
    double xx=1.*(pt.x-rc.left_)/(rc.right_-rc.left_)-0.5;

    return fabs(xx) > fabs(yy)
           ? ( xx<0 ? left : right )
               : ( yy<0 ? top : bottom );
}

direction distance_to_border::closest_border(pwindow_t win)
{
    const rect &rc=win->get_rect();
    return closest_border(rc);
}

bool distance_to_border::operator()(pwindow_t a, pwindow_t b)
{
    return distance(a)<distance(b);
}


