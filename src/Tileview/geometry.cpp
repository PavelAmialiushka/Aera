#include "StdAfx.h"

#include "geometry.h"
#include "wgeometry.h"

#include "utilites/tokenizer.h"
#include "utilites/serl/Archive.h"

#include <boost/format.hpp>

//////////////////////////////////////////////////////////////////////////

direction::direction()
    : dir_(zero)
{
}

direction::direction(_direction d)
{
    dir_=norm(d);
}

direction::operator int() const
{
    return dir_;
}

int direction::norm(int d)
{
    if (d==zero) return zero;
    return (d+dirmax*4)%(dirmax);
}

direction direction::rotate(int r) const
{
    if (*this==zero) return *this;
    return direction(static_cast<_direction>(dir_+r));
}

direction direction::operator+(int t) const
{
    return rotate(t);
}

direction direction::operator-(int t) const
{
    return rotate(-t);
}

bool direction::is_vertical() const
{
    return dir_==_top || dir_==_bottom;
}

bool direction::is_horizontal() const
{
    return dir_==_left || dir_==_right;
}

direction direction::get_vertical() const
{
    switch (dir_)
    {
    case _top: case _lt: case _rt: return _top;
    case _bottom: case _rb: case _lb : return _bottom;
    } return zero;
}

direction direction::get_horizontal() const
{
    switch (dir_)
    {
    case _left: case _lt: case _lb: return _left;
    case _right:  case _rt: case _rb : return _right;
    } return zero;
}

direction direction::operator|(direction d) const
{
    if (d==zero) return *this;
    if (*this==zero) return d;
    return
        abs(dir_-d.dir_)<4
        ? direction( static_cast<_direction>((dir_+d.dir_)/2) )
        : direction( static_cast<_direction>((dir_+d.dir_+dirmax)/2) );
}

bool direction::operator&(int d) const
{
    if (d==zero || *this==zero) return false;
    return
        abs(d-dir_)<=4
        ? abs(d-dir_)<2
        : (dirmax-abs(d-dir_)<2);
}

direction direction::operator-() const
{
    return rotate(back);
}

const char *direction::str() const
{
    switch ((int)dir_)
    {
    case _top:    return "top";
    case _rt:     return "rightTop";
    case _right:  return "right";
    case _rb:     return "rightBottom";
    case _bottom: return "bottom";
    case _lb:     return "leftBottom";
    case _left:   return "left";
    case _lt:     return "leftTop";
    }
    return "zero";
}

//////////////////////////////////////////////////////////////////////////

line::line()
{
    set_point1(point()),
               set_point2(point());
}

line::line(int a, int b, int c, int d)
{
    set_point1(point(a, b));
    set_point2(point(c, d));
}

line::line(double a, double b, double c, double d)
{
    set_point1(point(a,b));
    set_point2(point(c,d));
}

line::line(const point &a, const point &b)
{
    set_point1(a);
    set_point2(b);
}

void line::swap(line &l)
{
    point1_.swap(l.point1_);
    point2_.swap(l.point2_);
}

line &line::operator=(line l)
{
    l.swap(*this);
    return *this;
}

line line::parallel_copy(const point &pt) const
{
    line temp;
    if (get_direction().is_vertical())
    {
        temp.set_point1( point(pt.x, get_point1().y) );
        temp.set_point2( point(pt.x, get_point2().y) );
    }
    else
    {
        temp.set_point1( point(get_point1().x, pt.y) );
        temp.set_point2( point(get_point2().x, pt.y) );
    }
    return temp;
}

void line::set_point1(const point &pt)
{
    point1_=pt;
}

void line::set_point2(const point &pt)
{
    point2_=pt;
}

point line::get_point1() const
{
    return point1_;
}

point line::get_point2() const
{
    return point2_;
}

vector line::get_vector() const
{
    debug::Assert<>(point1_.x==point2_.x || point1_.y==point2_.y, "line cannot be diagonal", HERE);
    if (point1_.x == point2_.x)
    {
        return vector(point1_.y<point2_.y ? bottom : top, abs(point1_.y-point2_.y));
    }
    else
    {
        return vector(point1_.x<point2_.x ? right : left, abs(point1_.x-point2_.x));
    }
}

direction line::get_direction() const
{
    return get_vector();
}

line::operator direction() const
{
    return get_direction();
}

line::operator int() const
{
    return get_length();
}

int line::get_length() const
{
    return get_vector();
}

int   line::transform(CRect &rc) const
{
    if (get_direction().is_vertical())
    {
        return abs(get_point2().y-get_point1().y)*rc.Height()/max;
    }
    else
    {
        return abs(get_point2().x-get_point1().x)*rc.Width()/max;
    }
}

bool line::empty() const
{
    return point1_==point2_;
}

// OPERs

line line::operator&(const rect &rc) const
{
    if (get_direction().is_vertical())
    {
        if (between_e(point1_.x, rc.left_, rc.right_))
        {
            int p1=std::max(std::min(point1_.y, point2_.y), rc.top_);
            int p2=std::min(std::max(point1_.y, point2_.y), rc.bottom_);
            if (p1 <= rc.bottom_ && p2>=rc.top_)
            {
                return line(
                           point(point1_.x, p1),
                           point(point1_.x, p2));
            }
        }
    }
    else if (between_e(point1_.y, rc.top_, rc.bottom_))
    {
        int p1=std::max(std::min(point1_.x, point2_.x), rc.left_);
        int p2=std::min(std::max(point1_.x, point2_.x), rc.right_);
        if (p1 <= rc.right_ && p2 >= rc.left_)
        {
            return line(
                       point(p1, point1_.y),
                       point(p2, point2_.y)
                   );
        }
    }
    return line();
}

vector line::operator-(const point &pt) const
{
    return get_direction().is_horizontal()
           ? (point(pt.x, point1_.y)-pt)
           : (point(point1_.x, pt.y)-pt);
}

bool  line::contains_projection(point pt) const
{
    return get_direction().is_horizontal()
           ? point1_.x <= pt.x && pt.x <=point2_.x
           : point1_.y <= pt.y && pt.y <=point2_.y;
}

bool line::operator==(const line &ln) const
{
    return get_point1()==ln.get_point1()
           && get_point2()==ln.get_point2();
}

bool line::operator!=(const line &ln) const
{
    return !operator==(ln);
}


const char *line::str() const
{
    char *buffer=new char[100];
    sprintf(buffer, "(%d, %d), (%d, %d)", point1_.x, point1_.y, point2_.x, point2_.y);
    return buffer;
}

//////////////////////////////////////////////////////////////////////////


corner::corner(_direction d)
    : direction(d)
{
}

corner::corner(direction d)
    : direction(d)
{
}

corner corner::operator+(_rotation r)
{
    return corner(static_cast<_direction>(dir_+r));
}

border corner::operator+(_shifting r)
{
    return border(static_cast<_direction>(dir_+r));
}
//////////////////////////////////////////////////////////////////////////

border::border(_direction d)
    : direction(d)
{
}

border::border(direction d)
    : direction(d)
{
}

border border::operator-() const
{
    return border( direction::operator-() );
}

border border::operator+(_rotation r)
{
    return border(static_cast<_direction>(dir_+r));
}

corner border::operator+(_shifting r)
{
    return corner(static_cast<_direction>(dir_+r));
}

//////////////////////////////////////////////////////////////////////////

vector point::operator-(const point &p) const
{
    direction dy = y<p.y ? top : y==p.y ? zero : bottom ;
    direction dx = x<p.x ? left : x==p.x ? zero : right ;
    return vector(
               dx|dy,
               abs(x-p.x)+
               abs(y-p.y));
}

//////////////////////////////////////////////////////////////////////////

rect::rect() : left_(0), right_(0), top_(0), bottom_(0)
{
}

rect::rect(point a, point b)
    : left_(a.x), right_(b.x),
      top_(a.y), bottom_(b.y)
{
    norm();
}

rect::rect(int l, int t, int r, int b)
    : left_(l), right_(r),top_(t),bottom_(b)
{
    norm();
}

rect::rect(double l, double t, double r, double b)
    : left_(  static_cast<int>(0.5+max *l)),
      right_( static_cast<int>(0.5+max *r)),
      top_(   static_cast<int>(0.5+max *t)),
      bottom_(static_cast<int>(0.5+max *b))
{
    norm();
}


void rect::norm()
{
    if (left_ > right_) std::swap(left_, right_);
    if (top_ > bottom_) std::swap(top_, bottom_);
}

void rect::check_validation()
{
    debug::Assert<invalid_rect>(left_<=right_ && top_<=bottom_, HERE);
}

bool rect::empty() const
{
    return left_==right_ || top_==bottom_;
}

point rect::get_corner(direction d) const
{
    switch (d)
    {
    case _lt: return point(left_, top_);
    case _rt: return point(right_, top_);
    case _lb: return point(left_, bottom_);
    case _rb: return point(right_, bottom_);
    }
    debug::Assert<unexpected_behavior>(false,  HERE);
    return point();
}

rect rect::intersect(rect other) const
{
    const rect &self=*this;
    if (!intersects_rect(self)(other)) return rect();

    int left=other.left_ <= self.left_ && self.left_ <= other.right_ ? self.left_ : other.left_;
    int right=other.left_ <= self.right_ && self.right_ <= other.right_ ? self.right_ : other.right_;
    int top=other.top_ <= self.top_ && self.top_<= other.bottom_? self.top_ : other.top_;
    int bottom=other.top_ <= self.bottom_ && self.bottom_<= other.bottom_? self.bottom_ : other.bottom_;

    return rect(left, top, right, bottom);
}

rect rect::operator&(rect other) const
{
    return intersect(other);
}

int  rect::square() const
{
    return (int)((*this)*top) * (int)((*this)*left);
}

line rect::get_border(direction d) const
{
    switch (d)
    {
    case _left:   return line( point(left_, top_), point(left_, bottom_) );
    case _right:  return line( point(right_, top_), point(right_, bottom_) );
    case _top:    return line( point(left_, top_), point(right_, top_) );
    case _bottom: return line( point(left_, bottom_), point(right_, bottom_) );
    default: debug::Assert<>(0, "incorect direction", HERE);
    };
    return line();
}

corner_point operator*(rect &rc, corner c)
{
    return corner_point(rc, c);
}

corner_point operator*(corner c, rect &rc)
{
    return corner_point(rc, c);
}

point operator*(const rect &rc, corner c)
{
    return rc.get_corner(c);
}

point operator*(corner c, const rect &rc)
{
    return rc.get_corner(c);
}

CRect rect::transform(const CSize &sz) const
{
    CRect rc;
    rc.left=left_    *sz.cx/max;
    rc.right=right_  *sz.cx/max;

    rc.top =top_     *sz.cy/max;
    rc.bottom=bottom_*sz.cy/max;
    return rc;
}

CRect rect::transform(const CRect &rc) const
{
    CRect result=transform(rc.Size());
    result.left+=rc.left;
    result.right+=rc.left;
    result.top+=rc.top;
    result.bottom+=rc.top;
    return result;
}

bool rect::operator==(const rect &rc) const
{
    return left_==rc.left_ && right_==rc.right_ && top_==rc.top_ && bottom_==rc.bottom_;
}

bool rect::operator!=(const rect &rc) const
{
    return !operator==(rc);
}

const char *rect::str() const
{
    char *buffer=new char[100];
    sprintf(buffer, "(%d, %d), (%d, %d)", left_, top_, right_, bottom_);
    return buffer;
}

void rect::serialization(serl::archiver &ar)
{
#if 1
    std::string str;
    if (ar.is_saving())
    {
        boost::format fmt("%03d,%03d,%03d,%03d");
        fmt
        % (left_*1000/max)
        % (top_*1000/max)
        % (right_*1000/max)
        % (bottom_*1000/max);
        str=fmt.str();
    }

    ar.serial(str);

    if (ar.is_loading())
    {
        double a=boost::lexical_cast<double>(::tokenizer::head(str, ","))/1000;
        double b=boost::lexical_cast<double>(::tokenizer::head(str, ","))/1000;
        double c=boost::lexical_cast<double>(::tokenizer::head(str, ","))/1000;
        double d=boost::lexical_cast<double>(::tokenizer::head(str, ","))/1000;
        *this=rect(a, b, c, d);
    }

#else
    ar.serial("left", left_);
    ar.serial("right", right_);
    ar.serial("top", top_);
    ar.serial("bottom", bottom_);
#endif
}


//////////////////////////////////////////////////////////////////////////


border_line operator*(rect &rc, border b)
{
    return border_line(rc, b);
}

border_line operator*(border b, rect &rc)
{
    return border_line(rc, b);
}

line operator*(const rect &rc, border b)
{
    return rc.get_border(b);
}

line operator*(border b, const rect &rc)
{
    return rc.get_border(b);
}

//////////////////////////////////////////////////////////////////////////

corner_point::corner_point(rect &rc, corner cn)
    : master(&rc),
      crn(cn)
{
}

corner_point &corner_point::operator=(point pt)
{
    master->set_corner(crn, pt);
    return *this;
}

corner_point::operator point() const
{
    return master->get_corner(crn);
}

bool corner_point::operator==(point pt) const
{
    return operator point()==pt;
}

//////////////////////////////////////////////////////////////////////////

border_line::border_line(rect &rc, border brd)
    : master(&rc), brd(brd)
{
}

border_line &border_line::operator=(point pt)
{
    master->shift_border(brd, pt);
    return *this;
}

border_line::operator line() const
{
    return master->get_border(brd);
}

border_line::operator int() const
{
    return static_cast<line>(*this).get_length();
}

border_line::operator direction() const
{
    return static_cast<line>(*this).get_direction();
}

bool border_line::operator==(line ln) const
{
    return operator line()==ln;
}

vector operator-(border_line l, point p)
{
    return (line)l-p;
}

vector operator-(point p, border_line l)
{
    return p-(line)l;
}
