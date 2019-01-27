#pragma once

#include "utilites\copy_if.h"

#include "geometry.h"
#include "window.h"
#include "page.h"

//////////////////////////////////////////////////////////////////////////

point step_from_corner(Page *base, point corner, direction dir, direction side);


//////////////////////////////////////////////////////////////////////////

class Line : public line
{
public:
    DEFINE_NERROR_OF_TYPE(geometry_error, creation_error, "Line: no such node found");
public:
    Line();
    Line(Page *base, const line &ln, direction side);
    Line(Page *base, point corner, direction dir, direction side);
    Line(Page *base, const point &a, const point &b, direction side);

    void swap(Line &l);
    Line &operator=(Line ln);

    const line &get_line() const
    {
        return *this;
    }
    ppoints_t get_points() const
    {
        return nodes_;
    }
    direction get_side() const
    {
        return side_;
    }

protected:

    // создает максимально длинный луч из данной точки в данном направлении
    void make_xray(Page *base, point corner, direction dir, direction side);

    // создает линию если существует отрезок между двумя точками
    // устанавливает боковую ориентацию
    void make_2_points(Page *base, const point &a, const point &b, direction side);

private:
    ppoints_t nodes_;
    direction side_;
};


//////////////////////////////////////////////////////////////////////////

Line MakeMinSnapLine(Page *base, point pt, border =direction(), bool=false);
Line MakeSnapLine(Page *base, point pt, border =direction(), bool=false);

//////////////////////////////////////////////////////////////////////////

// class Areal служит для создания и масштабирования
// прямоугольной области из нескольких окон

class Areal
{
public:
    DEFINE_NERROR_OF_TYPE(geometry_error, resizing_error, "Aeral::resizing_error");

    Areal(Page *base, const point &cn, corner dir, bool maximize=true);
    Areal(Page *base, const Line &ln, border dir, bool maximize=true);
    Areal(const Areal &copy);

    Areal &operator=(Areal temp);
    void swap(Areal &copy);

private:

    // modi
    void create_from_corner(Page *, const point &corner, direction dir, bool);
    void create_from_line_max(Page *, const Line &ln, direction dir);
    void create_from_line_min(Page *, const Line &ln, direction dir);

    void collect_windows();

    // SEL
    int  calculate_window_row(direction dir) const;

public:

    void resize(Page *base, const rect &rc);

public:

    // sel

    const rect &get_rect() const
    {
        return rect_;
    }
    bool empty() const
    {
        return rect_.empty();
    }
    unsigned get_window_row(direction dir) const
    {
        return calculate_window_row(dir);
    }

private:
    scoped_ptr<Page> localpage_;
    direction root_;
    rect rect_;
    int width_;
};

typedef std::vector<Areal> areals_t;

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

class Selection
{
public:

    Selection(const point &a, const point &b);

    void track_point(const point &pt );
    void freeze();

    bool  is_freezed() const;
    point get_point1() const;
    point get_point2() const;

private:
    point point1_,
          point2_;
    bool  freezed_;
};

class SelectionRect : public Selection
{
public:
    typedef shared_ptr<SelectionRect> ptr;

    SelectionRect(Page *bz, const point &a, const point &b);

private:

    void create(Page *);

public:

    void track_point(Page *base, const point &pt);

    // SEL

    rect get_rect() const
    {
        return rect_;
    }
    rect get_drag_rect() const
    {
        return rect(get_point1(), get_point2());
    }
    Page *get_subpage(Page *base) const;

private:
    rect  rect_;
};

//////////////////////////////////////////////////////////////////////////

class SelectionLine : public Selection
{
public:
    typedef shared_ptr<SelectionLine> ptr;

    SelectionLine(Page *base, const point &a, const point &b);

    void track_point(Page *base, const point &pt);

    Line  get_line() const;

private:

    void create(Page *base);

private:
    Line line_;
};

class SelectionLineEx
{
public:

    SelectionLineEx(Page *base, const point &a);
    void track_point(const point &pt);

private:

    Page *page_;

    Line  line_;
    point point1_;
};


//////////////////////////////////////////////////////////////////////////
/// ALGORITHMS ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// bool has_corner(point)(rect)
// bool contains_point(point)(rect)

// bool intersects_border(line)(rect)
// bool intersects_rect(rect)(rect)

// bool is_part_of_rect(rect)(rect)

// int  distance_to_border(point)(rect)

struct algorithms
{
    int dummy;
    algorithms()
    {
        dummy=0;
    }
}; // for text-search only

//////////////////////////////////////////////////////////////////////////

line operator*(pwindow_t win, border brd);
line operator*(border brd, pwindow_t win);

point operator*(pwindow_t win, corner crn);
point operator*(corner crn, pwindow_t win);

//////////////////////////////////////////////////////////////////////////

struct contains_point : public std::unary_function<pwindow_t, bool>
{
    const point &pt;
    contains_point(const point &p) : pt(p) {}

    bool operator()(const rect &rc) const
    {
        return
            pt.x >= rc.left_ &&
            pt.x <= rc.right_ &&
            pt.y>=rc.top_ &&
            pt.y<=rc.bottom_ ;
    }
    bool operator()(pwindow_t win) const
    {
        return operator()(win->get_rect());
    }
};

//////////////////////////////////////////////////////////////////////////

struct is_part_of_rect: public std::unary_function<pwindow_t, bool>
{
    const rect &rc;
    is_part_of_rect(const rect &rc1) : rc(rc1) {}

    bool operator()(const rect &r2) const
    {
        return between_e(r2.left_, rc.left_, rc.right_)
               && between_e(r2.right_, rc.left_, rc.right_)
               && between_e(r2.top_, rc.top_, rc.bottom_)
               && between_e(r2.bottom_, rc.top_, rc.bottom_)
               ;
    }
    bool operator()(pwindow_t w) const
    {
        return operator()(w->get_rect());
    }
};

//////////////////////////////////////////////////////////////////////////

struct intersects_border
        : public std::unary_function<pwindow_t, bool>
{
    const line &ln;
    intersects_border(const line &l) : ln(l) {}

    border where(const rect &rc) const
    {
        if (ln.get_direction().is_vertical())
        {
            if (ln.get_point1().x==rc.left_) return left;
            if (ln.get_point1().x==rc.right_) return right;
            return zero;
        }
        else
        {
            if (ln.get_point1().y==rc.top_) return top;
            if (ln.get_point1().y==rc.bottom_) return bottom;
            return zero;
        }
    }

    bool operator()(const rect &rc) const
    {
        return where(rc)!=zero;
    }
    bool operator()(pwindow_t pwin) const
    {
        return operator()(pwin->get_rect());
    }
};

struct distance_to_border : public std::binary_function<pwindow_t, pwindow_t, bool>
{
    const point &pt;

    distance_to_border(const point &p) : pt(p) {}

    int distance(const rect &rc);
    int distance(pwindow_t win);

    direction closest_border(const rect &rc);
    direction closest_border(pwindow_t win);

    bool operator()(pwindow_t a, pwindow_t b);
};


struct has_corner : public std::unary_function<pwindow_t, bool>
{
    const point &pt;
    corner       dir;
    has_corner(const point &p, int d) : pt(p), dir((_direction)d)
    {}

    bool operator()(const rect &rc) const
    {
        return rc * dir == pt;
    }

    bool operator()(pwindow_t w) const
    {
        const rect &rc=w->get_rect();
        return operator()(rc);
    }
};

struct get_corner : public std::unary_function<pwindow_t, bool>
{
    direction dir_;
    get_corner(direction z) : dir_(z) {}
    point operator()(pwindow_t w) const
    {
        return w->get_rect().get_corner(dir_);
    }
};

struct intersects_rect : public std::unary_function<pwindow_t, bool>
{
    const rect &rc_;
    intersects_rect(const rect &rc) : rc_(rc) {}
    bool operator()(const rect &r2) const
    {
        return
            (    between_e(rc_.left_, r2.left_, r2.right_)
                 || between_e(rc_.right_, r2.left_, r2.right_)
                 || between_e(r2.left_, rc_.left_, rc_.right_)
                 || between_e(r2.right_, rc_.left_, rc_.right_)
            ) &&
            (    between_e(rc_.top_, r2.top_, r2.bottom_)
                 || between_e(rc_.bottom_, r2.top_, r2.bottom_)
                 || between_e(r2.top_, rc_.top_, rc_.bottom_)
                 || between_e(r2.bottom_, rc_.top_, rc_.bottom_)
            );
    }
    bool operator()(pwindow_t win) const
    {
        return operator()(win->get_rect());
    }
};

//////////////////////////////////////////////////////////////////////////

inline
int scale(int value, int l1, int r1, int l2, int r2)
{
    return (value-l1)*(r2-l2)/(r1-l1)+l2;
}

