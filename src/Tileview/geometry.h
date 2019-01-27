#pragma once

#include "utilites\copy_if.h"

DEFINE_ERROR(geometry_error, "common geometry error");

DEFINE_ERROR_OF_TYPE(geometry_error, unexpected_behavior, "geomerty: unexpected behaviour");
DEFINE_ERROR_OF_TYPE(geometry_error, incorrent_param, "geomerty: incorrect parameters");

//////////////////////////////////////////////////////////////////////////
enum _direction {_top=0,
                 _rt,  _right, _rb, _bottom, _lb, _left, _lt, dirmax,
                 zero=15,
                };

enum _rotation { cw=2, ccw=6, back=4, };
enum _shifting { fr=1, br=3, bl=5, fl=7, };

struct has_str_member
{
    const char *str() const
    {
        return "--";
    }
};


struct direction : has_str_member
{

    direction();
    direction(_direction d);

    operator int() const;

    direction rotate(int r) const;
    direction operator+(int t) const;
    direction operator-(int t) const;

    bool is_vertical() const;
    bool is_horizontal() const;
    direction get_vertical() const;
    direction get_horizontal() const;

    direction operator|(direction d) const;

    bool operator&(int d) const;
    direction operator-() const;

    const char *str() const;

protected:

    static int norm(int d);

    int dir_;
};

inline
std::ostream &operator<<(std::ostream &out, const direction &d)
{
    return out << d.str();
}

//////////////////////////////////////////////////////////////////////////

struct border;

struct corner : direction
{
    corner(direction);
    corner(_direction);

    corner operator+(_rotation);
    border operator+(_shifting);
};

static corner lt(_lt);
static corner rt(_rt);
static corner lb(_lb);
static corner rb(_rb);

//////////////////////////////////////////////////////////////////////////

struct border : direction
{
    border(_direction);
    border(direction);

    border operator+(_rotation);
    corner operator+(_shifting);

    border operator-() const;
};

static border left(_left);
static border top(_top);
static border bottom(_bottom);
static border right(_right);

struct point : has_str_member
{

    enum { max = 72 };
    int x, y;

    point() : x(0), y(0) {}
    point(int a, int b) : x(a), y(b) {}
    point(double a, double b) :
        x( static_cast<int>(a*max+0.5)),
        y( static_cast<int>(b*max+0.5))
    {}

    point(const point &p)
    {
        assign(p.x, p.y);
    }
    point(const CPoint &pt, const CSize &sz)
    {
        assign(pt, sz);
    }
    point(const CPoint &pt, const CRect &base)
    {
        assign(pt, base);
    }

    void swap(point &p)
    {
        std::swap(p.x, x);
        std::swap(p.y, y);
    }

    point &operator=(point p)
    {
        swap(p); return *this;
    }

    point &assign(int a, int b)
    {
        x=a;
        y=b;
        return *this;
    }

    point &assign(const CPoint &pt, const CSize &sz)
    {
        x=sz.cx ? pt.x * max / sz.cx : 0;
        y=sz.cy ? pt.y * max / sz.cy : 0;
        return *this;
    }

    point &assign(const CPoint &pt, const CRect &base)
    {
        return assign(pt, base.Size());
    }

    int get_distance(const point &pt) const
    {
        return pt.x==x
               ? abs(pt.y-y)
               : abs(pt.x-x);
    }

    int get_distance(const point &pt, direction dr) const
    {
        if (dr==left)  return pt.x-x;
        if (dr==right) return x-pt.x;
        if (dr==top)   return pt.y-y;
        if (dr==bottom)return y-pt.y;
        return pt.x+pt.y-x-y;
    }


    CPoint transform(const CSize &sz) const
    {
        return CPoint(
                   x * sz.cx / max,
                   y * sz.cy / max
               );
    }
    CPoint transform(const CRect &rc) const
    {
        return transform(rc.Size());
    }

    bool operator==(const point &p) const
    {
        return x==p.x && y==p.y;
    }
    bool operator!=(const point &p) const
    {
        return !operator==(p);
    }

    struct vector operator-(const point &p) const;

    const char *str() const
    {
        char *buffer=(char *)malloc(100);
        sprintf(buffer, "%d, %d", x, y);
        return buffer;
    }
};

typedef std::vector<point> points_t;
typedef shared_ptr<points_t> ppoints_t;

inline
std::ostream &operator<<(std::ostream &str, const point &pt)
{
    return str << strlib::strf("%03d,%03d"
                               , std::min(pt.x*1000/point::max, 999)
                               , std::min(pt.y*1000/point::max, 999));
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

struct vector : has_str_member
{
    enum { max = point::max };
public:

    vector() : dir_(zero), length_(0) {}
    vector(direction d, int len) : dir_(d), length_(len)
    {
        norm();
    }
    vector(const vector &v) : dir_(v.dir_), length_(v.length_)
    {
        norm();
    }

    // MODI

    vector &operator=(vector v)
    {
        swap(v);
        return *this;
    }

    void norm()
    {
        if (get_direction()==zero) length_=0;
        else if (get_length()==0) dir_=zero;
    }

    void swap(vector &v)
    {
        std::swap(dir_, v.dir_);
        std::swap(length_, v.length_);
    }

    // SELE

    int get_length() const
    {
        return length_;
    }
    direction get_direction() const
    {
        return dir_;
    }

    // OPERS

    operator int() const
    {
        return get_length();
    }
    operator direction() const
    {
        return get_direction();
    }

    vector operator-() const
    {
        return vector(-get_direction(), get_length());
    }

    int transform(const CRect &rc)
    {
        direction dr=get_direction();
        if (dr.is_vertical())
        {
            return get_length()*rc.Height()/max;
        }
        else if (dr.is_horizontal())
        {
            return get_length()*rc.Width()/max;
        }
        else
        {
            debug::Assert<>(dr==zero, "cannot tranform diagonal vector.", HERE);
            return 0;
        }
    }

    const char *str() const
    {
        char *buffer=new char[100];
        sprintf(buffer, "%s: %d", dir_.str(), length_);
        return buffer;
    }

private:

    direction dir_;
    int length_;
};

inline point operator+(const point &pt, const vector &v)
{
    return v.get_direction().is_vertical()
           ? point(  pt.x,  v.get_direction()==top
                     ? pt.y - v.get_length()
                     : pt.y + v.get_length() )
               : point(  v.get_direction()==left
                         ? pt.x-v.get_length()
                         : pt.x+v.get_length(), pt.y);
}

inline point operator+(const vector &v, const point &pt)
{
    return operator+(pt, v);
}

inline
std::ostream &operator<<(std::ostream &out, const vector &vec)
{
    return out <<
           strlib::strf("%s,%03d",
                        vec.get_direction().str(),
                        std::min(vec.get_length()*1000/point::max, 999));
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

struct line;
struct corner_point;
struct border_line;

struct rect : has_str_member
{
    DEFINE_NERROR_OF_TYPE(geometry_error, invalid_rect, "rect::invalid_rect");
    enum { max = point::max };

public:

    rect();
    rect(int l, int t, int r, int b);
    rect(point a, point b);
    rect(double l, double t, double r, double b);

    void norm();
    void check_validation();

    rect(const rect &r)
        : left_(r.left_), right_(r.right_), top_(r.top_), bottom_(r.bottom_)
    {
        norm();
    }

    rect &operator=(rect r)
    {
        swap(r);
        return *this;
    }

    rect(const CRect &rc, const CSize &sz)
    {
        assign(rc, sz);
    }

    rect(const CRect &rc, const CRect &base)
    {
        assign(rc, base);
    }

    rect &assign(int l, int t, int r, int b)
    {
        left_=l; top_=t; right_=r; bottom_=b;
        norm();
        return *this;
    }

    void swap(rect &other)
    {
        std::swap(left_, other.left_);
        std::swap(top_, other.top_);
        std::swap(right_, other.right_);
        std::swap(bottom_, other.bottom_);
    }

    rect &assign(const CRect &rc, const CSize &sz)
    {
        left_=sz.cx ? rc.left     *max/sz.cx : 0;
        right_=sz.cx ? rc.right   *max/sz.cx: 0;

        top_=sz.cy ? rc.top       *max/sz.cy : 0;
        bottom_=sz.cy ? rc.bottom *max/sz.cy : 0;

        norm();
        return *this;
    }

    rect &assign(const CRect &rc, const CRect &basic)
    {
        return assign(rc, basic.Size());
    }

    rect &set_corner(direction dir, const point &pt)
    {
        if (dir&left) left_=pt.x;
        if (dir&right) right_=pt.x;
        if (dir&top)   top_=pt.y;
        if (dir&bottom) bottom_=pt.y;
        check_validation();
        return *this;
    }

    rect &shift_border(direction dir, int len)
    {
        if (dir&left)   left_  -=len;
        if (dir&right)  right_ +=len;
        if (dir&top)    top_   -=len;
        if (dir&bottom) bottom_+=len;
        check_validation();
        return *this;
    }

    rect &shift_border(direction dir, point val)
    {
        if (dir&left)   left_  =val.x;
        if (dir&right)  right_ =val.x;
        if (dir&top)    top_   =val.y;
        if (dir&bottom) bottom_=val.y;
        check_validation();
        return *this;
    }

//// selectors ///////////////////////////////////////////////////////////

    int  square() const;
    rect intersect(rect other) const;
    rect operator&(rect) const;

    bool empty() const;
    point get_corner(direction d) const;

    friend corner_point operator*(rect &, corner);
    friend corner_point operator*(corner, rect &);
    friend point operator*(const rect &, corner);
    friend point operator*(corner, const rect &);

    line get_border(direction) const;

    friend border_line operator*(rect &, border);
    friend border_line operator*(border, rect &);
    friend line operator*(const rect &, border);
    friend line operator*(border, const rect &);

    CRect transform(const CSize &sz) const;
    CRect transform(const CRect &rc) const;

    bool operator==(const rect &rc) const;
    bool operator!=(const rect &rc) const;
    const char *str() const;

    void serialization(serl::archiver &ar);

public:

    int left_, top_, right_, bottom_;
};

inline
std::ostream &operator<<(std::ostream &str, const rect &rc)
{
    return str << strlib::strf("%03d,%03d-%03d,%03d"
                               , std::min(rc.left_  *1000/point::max,999)
                               , std::min(rc.top_   *1000/point::max,999)
                               , std::min(rc.right_ *1000/point::max,999)
                               , std::min(rc.bottom_*1000/point::max,999));
}

//////////////////////////////////////////////////////////////////////////

struct corner_point
{
    corner_point(rect &, corner);

    corner_point &operator=(point);
    operator point() const;

    bool operator==(point) const;

private:
    rect  *master;
    corner crn;
};

//////////////////////////////////////////////////////////////////////////

struct line : has_str_member
{

    enum { max = point::max, };

    line();
    line(int a, int b, int c, int d);
    line(double a, double b, double c, double d);
    line(const point &a, const point &b);
    void swap(line &l);
    line &operator=(line l);

    line parallel_copy(const point &pt) const;

    void set_point1(const point &pt);
    void set_point2(const point &pt);

    point get_point1() const;
    point get_point2() const;

    vector get_vector() const;
    direction get_direction() const;
    operator direction() const;
    operator int() const;

    int   get_length() const;
    int   transform(CRect &rc) const;
    bool  empty() const;

    line   operator&(const rect &rc) const;
    vector operator-(const point &pt) const;

    bool contains_projection(point pt) const;

    bool operator==(const line &ln) const;
    bool operator!=(const line &ln) const;

    const char *str() const;
private:
    point point1_;
    point point2_;
};


inline
line operator&(const rect &rc, const line &ln)
{
    return ln&rc;
}

inline
vector operator-(const point &pt, const line &ln)
{
    return -(ln-pt);
}


inline std::ostream &operator << (std::ostream &os, const line &ln)
{
    return os << strlib::strf("%03d,%03d->%03d,%03d"
                              , std::min(ln.get_point1().x  *1000/point::max,999)
                              , std::min(ln.get_point1().y  *1000/point::max,999)
                              , std::min(ln.get_point2().x  *1000/point::max,999)
                              , std::min(ln.get_point2().y  *1000/point::max,999))
           ;
}

//////////////////////////////////////////////////////////////////////////

struct border_line
{
    border_line(rect &rc, border brd);

    border_line &operator=(point);
    operator line() const;
    operator int() const;
    operator direction() const;

    bool operator==(line) const;


    friend vector operator-(border_line, point);
    friend vector operator-(point, border_line);

private:
    rect *master;
    border brd;
};

inline std::ostream &operator<<(std::ostream &str, const border_line &self)
{
    return str << (const line &)self;
}

//////////////////////////////////////////////////////////////////////////
