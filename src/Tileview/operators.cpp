#include "stdafx.h"

#include "operators.h"
#include "utilites\foreach.hpp"

//////////////////////////////////////////////////////////////////////////

SlideLine::SlideLine()
    : base_(0)
{
}

SlideLine::SlideLine(Page *base)
    : base_(base)
{
}

SlideLine::SlideLine(const SlideLine &other)
    : side_(other.side_),
      areals_(other.areals_),
      snl_(other.snl_)
{
}

void SlideLine::swap(SlideLine &other)
{
    // do not copy base_
    std::swap(snl_, other.snl_);
    std::swap(side_, other.side_);
    areals_.swap(other.areals_);
}

void SlideLine::reset(Page *base)
{
    base_=base;
}

SlideLine &SlideLine::assign(SlideLine other)
{
    swap(other);
    return *this;
}

static rect calculate_new_rectangle(rect rectangle, line start, point mouse)
{
    direction dir=intersects_border(start).where(rectangle);
    rectangle.shift_border(dir, mouse);

    return rectangle;
}

void SlideLine::track(point mouse)
{
    line new_snl=snl_.parallel_copy(mouse);

    foreach(Areal& areal, areals_)
    {
        areal.resize(base_, calculate_new_rectangle(areal.get_rect(), snl_, mouse));
    }

    snl_=Line(base_, new_snl, snl_.get_side());
}

Line SlideLine::get_line() const
{
    return snl_;
}

Page *SlideLine::get_page() const
{
    return base_;
}

//////////////////////////////////////////////////////////////////////////

SlideLineBase::SlideLineBase(Page *base)
    : SlideLine(base)
{
    rollback();
}

void SlideLineBase::commit()
{
    base_->assign_layout( copypage_.get() );
    assign(copy_);
}

void SlideLineBase::rollback()
{
    copypage_=base_->clone();

    copy_.reset(copypage_.get());
    copy_.assign(*this);
}

void SlideLineBase::unsafe_track(point mouse)
{
    rollback();
    copy_.track(mouse);
}

bool SlideLineBase::track(point mouse)
{
    try
    {

        unsafe_track(mouse);
        return true;

    }
    catch (geometry_error &)
    {
        rollback();
    }
    return false;
}

Page *SlideLineBase::get_page() const
{
    return copy_.get_page();
}

//////////////////////////////////////////////////////////////////////////

XSlideLine::XSlideLine(Page *base, Line ln, direction dr)
    : SlideLineBase(base)
{
    snl_=ln;
    side_=dr;
}

XSlideLine::XSlideLine(Page *base, point pt, bool maximize, bool doublesized, Snapper snap)
    : SlideLineBase(base)
{
    snl_=snap(base_, pt, zero, doublesized);
    side_=snl_.get_side();

    areal_grabber(side_, maximize);
    if (doublesized) areal_grabber(-side_, maximize);

    rollback();
}

void XSlideLine::areal_grabber(direction side, bool maximize)
{
    try
    {
        int width_=point::max;

        // лицевая сторона
        int counter=400;
        point start = snl_.get_point1();
        point end   = snl_.get_point2();
        direction snld=snl_.get_direction();
        debug::Assert<fault>(snld.is_horizontal()!=side.is_horizontal(), HERE);

        do
        {
            debug::Assert<debug::deadlock>(--counter, HERE);

            // append new Areal
            Line basis(base_, start, end, side);
            areals_.push_back(Areal(base_, basis, -side | -snld, maximize));
            rect a_rc=areals_.back().get_rect();
            debug::Assert<creation_error>(!a_rc.empty(), HERE);

            start = a_rc.get_corner( -side | snld );
            width_= std::min( width_, (int)(a_rc * border(-snld)) );
        }
        while (start!=snl_.get_point2());

    }
    catch (geometry_error &)
    {
        throw;
    }
}

//////////////////////////////////////////////////////////////////////////

SingleSlideLine::SingleSlideLine(Page *base, Line ln, direction dr)
    : XSlideLine(base, ln, dr)
{
    areal_grabber(dr, true);
    rollback();
}

rect SingleSlideLine::get_rect() const
{
    rect temp(snl_.get_point1(), snl_.get_point2());

    int width=0;
    foreach(const Areal& areal, areals_)
    {
        rect ar=areal.get_rect();
        point a=ar * corner(side_+fl);
        point b=ar * corner(side_+bl);
        width = std::max(width, (int)(a-b));
    }

    temp.shift_border(side_,  width);
    return temp;
}

void SingleSlideLine::proceed(Page *base, Line ln, direction dr, point endp)
{
    SingleSlideLine lnx(base, ln, dr);
    lnx.unsafe_track(endp);
    lnx.commit();
}


/*
////////////////////////////////////////////////////////////////////////
class SingleSlideLineImpl {
public:

  SingleSlideLineImpl(Page* base, const Line& snl, direction side);
  SingleSlideLineImpl(const SingleSlideLineImpl& other);
  SingleSlideLineImpl& operator=(const SingleSlideLineImpl &other);
  void resize(Page* base, const point& pt);
  direction       get_side() const;
  int             get_width() const;
  const Line&     get_snapline();
  const areals_t& get_areals() const;
  rect            get_resize_rect() const;
  int             get_window_row() const;
private:
  areals_t  areals_;
  direction side_;
  Line      snl_;
  int       width_;
};
////////////////////////////////////////////////////////////////////////
SingleSlideLine::~SingleSlideLine()
{
}
void SingleSlideLineImpl::resize(Page* base, const point& mouse)
{
  Line old_snapl=snl_;
  line new_snapl=snl_.parallel_copy(/ *real* /mouse);

  for(unsigned index=0; index<areals_.size(); ++index) {
    rect nrc=areals_[index].get_rect();
    direction dir=intersects_border(old_snapl).where(nrc);
    nrc.set_corner(dir, / *real* /mouse);
    areals_[index].resize(base, nrc);
  }
  snl_=Line(base, new_snapl, snl_.get_side());
}
SingleSlideLineImpl::SingleSlideLineImpl(const SingleSlideLineImpl& other)
{
  snl_=other.snl_;
  width_=other.width_;
  side_=other.side_;
  areals_.assign(other.areals_.begin(), other.areals_.end());
}
SingleSlideLineImpl& SingleSlideLineImpl::operator=(const SingleSlideLineImpl& other)
{
  snl_=other.snl_;
  width_=other.width_;
  side_=other.side_;
  areals_.assign(other.areals_.begin(), other.areals_.end());
  return *this;
}
SingleSlideLineImpl::SingleSlideLineImpl(Page* base, const Line& snl, direction side)
: width_(0), side_(side)
{
  try {
    snl_=Line(base, snl, side);
    debug::Assert<SingleSlideLine::creation_error>(!snl_.empty(), HERE);
    width_=point::max;
    // лицевая сторона
    int counter=400;
    point start = snl_.get_point1();
    direction snld=snl_.get_direction();
    debug::Assert<fault>(snld.is_horizontal()!=side_.is_horizontal(), HERE);
    do {
      debug::Assert<debug::deadlock>(--counter, HERE);
      Line basis(base, start, snl_.get_point2(), snl_.get_side());
      areals_.push_back(Areal(base, basis, -side_ | -snld));
      rect a_rc=areals_.back().get_rect();
      debug::Assert<SingleSlideLine::creation_error>(!a_rc.empty(), HERE);
      start = a_rc.get_corner( -side_ | snld );
      width_= std::min( width_, (int)(a_rc * border(-snld)) );
    } while(start!=snl_.get_point2());
  } catch(geometry_error& e) { e;
    debug::Assert<SingleSlideLine::creation_error>(0, HERE);
  }
}
rect SingleSlideLineImpl::get_resize_rect() const
{
  rect temp(snl_.get_point1(), snl_.get_point2());
  temp.shift_border(side_,  width_);
  return temp;
}
direction SingleSlideLineImpl::get_side() const
{
  return side_;
}
int  SingleSlideLineImpl::get_width() const
{
  return width_;
}
const Line& SingleSlideLineImpl::get_snapline()
{
  return snl_;
}
const areals_t& SingleSlideLineImpl::get_areals() const
{
  return areals_;
}
int  SingleSlideLineImpl::get_window_row() const
{
  int result=0;
  areals_t::const_iterator index=areals_.begin();
  for(;index!=areals_.end(); ++index) {
    result=std::max(result, (int)index->get_window_row(side_));
    }
  return result;
}
////////////////////////////////////////////////////////////////////////
SingleSlideLine::SingleSlideLine(Page* base, const Line& snl, direction side)
: orpage_(base),
  localpage_( base->clone() ),
  original_( new SingleSlideLineImpl(base, snl, side) ),
  prcopy_( new SingleSlideLineImpl(localpage_.get(), snl, side) )
{
  rollback();
}
void SingleSlideLine::rollback()
{
  localpage_=orpage_->clone();
  *prcopy_=*original_;
}
void SingleSlideLine::commit(Page* base)
{
  base->assign_layout(localpage_.get());
  *original_=*prcopy_;
}
bool SingleSlideLine::resize(const point& pt)
{
  try {
    prcopy_->resize(localpage_.get(), pt);
  } catch (geometry_error&) {
    rollback();
    return false;
  }
  return true;
}
void SingleSlideLine::execute(Page* base, const Line& snl, direction side, const point& pt)
{
  SingleSlideLine ml(base, snl, side);
  ml.resize(pt);
  ml.commit(base);
}
rect SingleSlideLine::get_resize_rect() const
{
  return original_->get_resize_rect();
}
int  SingleSlideLine::get_window_row() const
{
  return prcopy_->get_window_row();
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
DoubleSlideLineImpl::DoubleSlideLineImpl(Page* base, const Line& snl)
{
  try {
    snapl_=snl;
    face_=snl.get_direction();
    mlface_.reset( new SingleSlideLineImpl(base, snl, face_+2) );
    mlback_.reset( new SingleSlideLineImpl(base, snl, face_-2));
  } catch(geometry_error&) {
    throw SingleSlideLine::creation_error(HERE);
  }
}
DoubleSlideLineImpl::DoubleSlideLineImpl(Page* base, const point& pt, SelectionRect* sz)
{
  try {
    snapl_=sz
          ? Line(base, MakeSnapLine(base, pt) & sz->get_rect() ,
                       MakeSnapLine(base, pt).get_side())
          : MakeSnapLine(base, pt);
    face_=snapl_.get_direction();
    mlface_.reset(new SingleSlideLineImpl(base, snapl_, face_+2));
    mlback_.reset(new SingleSlideLineImpl(base, snapl_, face_-2));
  } catch(geometry_error&) {
    throw SingleSlideLine::creation_error(HERE);
  }
}
void DoubleSlideLineImpl::synchronize(DoubleSlideLineImpl* copy)
{
  snapl_=copy->snapl_;
  face_=copy->face_;
  *mlface_=*copy->mlface_;
  *mlback_=*copy->mlback_;
}
void DoubleSlideLineImpl::resize(Page* base, const point& mouse)
{
  rect limits=get_resize_rect();
  point real(ensure_minmax(mouse.x, limits.left_, limits.right_),
          ensure_minmax(mouse.y, limits.top_, limits.bottom_));
  mlface_->resize(base, real);
  mlback_->resize(base, real);
  snapl_=mlface_->get_snapline();
}
rect DoubleSlideLineImpl::get_resize_rect() const
{
  rect temp(snapl_.get_point1(), snapl_.get_point2());
  temp.shift_border(mlface_->get_side(),  mlface_->get_width());
  temp.shift_border(mlback_->get_side(), mlback_->get_width());
  return temp;
}
int DoubleSlideLineImpl::get_window_row() const
{
  return mlface_->get_window_row()
        +mlback_->get_window_row();
}
areals_t DoubleSlideLineImpl::get_areals() const
{
  areals_t temp;
  std::copy(mlface_->get_areals().begin(), mlface_->get_areals().end(), std::back_inserter(temp));
  std::copy(mlback_->get_areals().begin(), mlback_->get_areals().end(), std::back_inserter(temp));
  return temp;
}
bool DoubleSlideLineImpl::is_horz() const
{
	return !face_.is_horizontal();
}
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
DoubleSlideLine::DoubleSlideLine(Page* base, const Line& snl)
: orpage_(base),
  localpage_(base->clone()),
  original_( new DoubleSlideLineImpl(base, snl) ), // original page
  prcopy_( new DoubleSlideLineImpl(localpage_.get(), snl) )
{
  rollback();
	is_horz_=original_->is_horz();
}
DoubleSlideLine::DoubleSlideLine(Page* base, const point& pt, SelectionRect* zone)
: orpage_(zone ? zone->get_subpage(base) : base),
  localpage_(orpage_->clone()),
  original_( new DoubleSlideLineImpl(orpage_, pt, zone) ), // original page
  prcopy_( new DoubleSlideLineImpl(localpage_.get(), pt, zone) )
{
  rollback();
	is_horz_=original_->is_horz();
}
void DoubleSlideLine::resize_zone(const point& pt)
{
  try {
    rollback();
    prcopy_->resize(localpage_.get(), pt);
  } catch(geometry_error&) {
    rollback();
  }
}
rect DoubleSlideLine::get_resize_rect() const
{
  return prcopy_->get_resize_rect();
}
Page* DoubleSlideLine::get_page()
{
  return localpage_.get();
}
areals_t DoubleSlideLine::get_areals() const
{
  return prcopy_->get_areals();
}
void DoubleSlideLine::commit(Page* base)
{
  base->assign_layout(localpage_.get());
  original_->synchronize(prcopy_.get());
  base->check_validation();
}
void DoubleSlideLine::rollback()
{
  localpage_ = shared_ptr<Page>( orpage_->clone() );
  prcopy_->synchronize(original_.get());
}
bool DoubleSlideLine::is_horz() const
{
	return is_horz_;
}

*/
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

Inserter::Inserter(Page *page, point pt1)
    : basepage_(page),
      localpage_(),
      point1_(pt1)
{
    track_point(pt1);
}

void  Inserter::rollback()
{
    localpage_=Page::shared_ptr(basepage_->clone());
    newwindow_=pwindow_t();
}

void Inserter::track_point(point pt2)
{
    rollback();

    try
    {
        SelectionLine sel(localpage_.get(), point1_, pt2);
        selline_=sel.get_line();

        // направление роста окна
        //border side=pt2-selline_;
        //if (side==zero) side=selline_.get_direction()+_left;
        border side=selline_.get_side();

        SingleSlideLine ml(localpage_.get(), selline_, side);

        // новый размер зоны и нового окна
        rect prevzone=ml.get_rect();
        rect zone=prevzone;
        rect newwindow=prevzone;

        zone*(side+back)=pt2;
        newwindow *side=pt2;

        debug::Assert<creation_error>(
            !(prevzone & zone).empty() && !newwindow.empty(),
            HERE);

        point newp=zone*(side+bl);

        ml.unsafe_track( newp );
        ml.commit();
        localpage_->check_validation();

        int id=localpage_->append_new_window();
        newwindow_=localpage_->find(id);
        newwindow_->set_rect(newwindow);

    }
    catch (geometry_error &)
    {

        rollback();

    }
}


Page *Inserter::get_page() const
{
    return localpage_.get();
}

point Inserter::get_point() const
{
    return point1_;
}

line Inserter::get_selline() const
{
    return selline_.get_line();
}

void Inserter::commit(Page *page)
{
    page->assign_layout(localpage_.get());
    page->check_validation();
}

pwindow_t Inserter::get_newwindow() const
{
    return newwindow_;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

Deleter::Deleter(Page *page, point pt)
    : page_(page)
{
    try
    {
        window_=get_window(pt);
        commit();
    }
    catch (cannot_delete) {}
}

Deleter::Deleter(Page *page, pwindow_t win)
    : page_(page),
      window_(win)
{
    try
    {
        commit();
    }
    catch (cannot_delete) {}
}

pwindow_t Deleter::get_window(point pt) const
{
    windows_t::iterator index=std::find_if(
                                  STL_II(*page_), contains_point(pt));

    debug::Assert<cannot_delete>(index!=page_->end());
    return *index;
}

Line Deleter::prepare_border(border brd) const
{
    try
    {
        rect global(.000,.000,.999,.999);

        // если упирается в край экрана
        intersects_border its(global*brd);
        debug::Assert<cannot_delete>(!its(window_), HERE);

        return Line(page_, window_->get_rect() * brd, brd);

    }
    catch (Line::creation_error)
    {
        throw cannot_delete(HERE);
    }
}

void Deleter::two_side_delete(border d, Line l1, Line l2)
{
    debug::Assert<cannot_delete>(l1 && l2, HERE);
    line ln = window_->get_rect() * (d+ccw);

    point middle_point=
        window_->get_rect() * (d+bl)
        +vector(d, ln.get_length()/2);

    window_->set_rect(rect());
    page_->erase(window_);

    SingleSlideLine::proceed(page_, l1, d, middle_point);
    SingleSlideLine::proceed(page_, l2, -d, middle_point);
}

void Deleter::one_side_delete(border d, Line ln)
{
    debug::Assert<cannot_delete>(ln, HERE);
    point corner_point=window_->get_rect() * (d+bl);

    SingleSlideLine::proceed(page_, ln, d, corner_point);
    page_->erase(window_);
}

void Deleter::try_to_delete_in_direction(border d)
{
    Line l1;
    Line l2;

    try
    {
        l1=prepare_border(d);
    }
    catch (cannot_delete) {}
    try
    {
        l2=prepare_border(-d);
    }
    catch (cannot_delete) {}

    try
    {
        if (!l2)
        {
            one_side_delete(d, l1);
        }
        else if (!l1)
        {
            one_side_delete(-d, l2);
        }
        else
        {
            two_side_delete(d, l1, l2);
        }
    }
    catch (geometry_error)
    {
        throw cannot_delete(HERE);
    }
}

void Deleter::commit()
{
    try
    {
        try_to_delete_in_direction(left);
    }
    catch (cannot_delete &)
    {
        try
        {
            try_to_delete_in_direction(bottom);
        }
        catch (cannot_delete &)
        {
        }
    }
    page_->check_validation();
}

