#pragma once

#include "wgeometry.h"

//////////////////////////////////////////////////////////////////////////

class SlideLine
{
    SlideLine &operator=(const SlideLine &other);
public:
    SlideLine();
    SlideLine(Page *base);
    SlideLine(const SlideLine &other);

    void reset(Page *base);
    SlideLine &assign(SlideLine other);

    void swap(SlideLine &other);
    void track(point mouse);

    Line get_line() const;
    Page *get_page() const;

protected:
    Page     *base_;
    direction side_;
    areals_t  areals_;
    Line      snl_;
};

class SlideLineBase : public SlideLine
{
public:
    SlideLineBase(Page *base);

    void commit();
    void rollback();

    void unsafe_track(point pt);
    bool track(point pt);

    Page *get_page() const;

private:
    SlideLine           copy_;
    std::auto_ptr<Page> copypage_;
};

//////////////////////////////////////////////////////////////////////////

class XSlideLine
    : public SlideLineBase
{
public:
    DEFINE_NERROR_OF_TYPE(geometry_error, creation_error, "XSlideLine::creation_error");

    //typedef boost::function3<Line, Page*, point, border> Snapper;
    typedef Line (*Snapper)(Page *, point, border, bool);

    XSlideLine(Page *, point,
               bool maximize=true,
               bool doublesized=true,
               Snapper=&MakeSnapLine
              );

protected:
    XSlideLine(Page *, Line, direction);

    void areal_grabber(direction, bool);
};

//////////////////////////////////////////////////////////////////////////

class SingleSlideLine
    : public XSlideLine
{
public:
    SingleSlideLine(Page *base, Line ln, direction dr);

    rect get_rect() const;

    static void proceed(Page *base, Line ln, direction dr, point endp);
};

//////////////////////////////////////////////////////////////////////////

class DoubleSlideLine
    : public XSlideLine
{
public:
    DoubleSlideLine(Page *base, point pt)
        : XSlideLine(base, pt, true, true, &MakeSnapLine) {}
};

class SmallDoubleSlideLine
    : public XSlideLine
{
public:
    SmallDoubleSlideLine(Page *base, point pt)
        : XSlideLine(base, pt, false, true, &MakeMinSnapLine) {}
};


//////////////////////////////////////////////////////////////////////////

//class SingleSlideLineImpl;
////////////////////////////////////////////////////////////////////////////
//
//// используется для изменения размеров окон по
//// одну сторону от линии.
//class SingleSlideLine
//{
//  class impl;
//public:
//  DEFINE_NERROR_OF_TYPE(geometry_error, creation_error, "SingleSlideLine: creation error");
//
//  static void execute(Page* base, const Line& snl, direction side, const point& pt);
//
//public:
//
//  SingleSlideLine(Page* base, const Line& snl, direction side);
//  virtual ~SingleSlideLine();
//
//  virtual void rollback();
//  virtual void commit(Page* base);
//
//  virtual void trace(point pt) { resize(pt); }
//  bool resize(const point& pt);
//
//  rect get_resize_rect() const;
//  int  get_window_row() const;
//
//private:
//
//  Page*            orpage_;
//  Page::shared_ptr localpage_;
//
//  scoped_ptr<SingleSlideLineImpl> prcopy_;
//  scoped_ptr<SingleSlideLineImpl> original_;
//};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//class DoubleSlideLineImpl {
//public:
//
//  DoubleSlideLineImpl(Page* base, const Line& snl);
//  DoubleSlideLineImpl(Page* base, const point& pt, SelectionRect* sz) ;
//
//
//  void resize(Page* base, const point& pt);
//  void synchronize(DoubleSlideLineImpl* copy);
//
//  int get_window_row() const;
//  rect get_resize_rect() const;
//
//  areals_t get_areals() const;
//
//	bool is_horz() const;
//
//private:
//  Line          snapl_;
//  direction     face_;
//  shared_ptr<SingleSlideLineImpl> mlface_;
//  shared_ptr<SingleSlideLineImpl> mlback_;
//};

// используется для изменения размеров окон по
// две стороны от линии.
//class DoubleSlideLine {
//public:
//  typedef shared_ptr<SingleSlideLine> ptr;
//
//  DoubleSlideLine(Page* base, const Line& snl);
//  DoubleSlideLine(Page* base, const point& pt, SelectionRect* zone);
//
//public:
//
//  void resize_zone(const point& pt);
//  void rollback();
//  void commit(Page* base);
//
//  rect get_resize_rect() const;
//  Page* get_page();
//  areals_t get_areals() const;
//
//	bool	is_horz() const;
//
//private:
//
//  Page*                     orpage_;
//  shared_ptr<Page>          localpage_;
//  scoped_ptr<DoubleSlideLineImpl> prcopy_;
//  scoped_ptr<DoubleSlideLineImpl> original_;
//	bool											is_horz_;
//};

//////////////////////////////////////////////////////////////////////////

class Inserter
{
public:
    DEFINE_NERROR_OF_TYPE(geometry_error, creation_error, "Inserter: creation_error");
    typedef shared_ptr<Inserter> ptr;

    Inserter(Page *, point);

    Page *get_page() const;
    point get_point() const;
    line  get_selline() const;
    pwindow_t get_newwindow() const;

    void track_point(point pt2);
    void commit(Page *);

private:

    void  rollback();

    point            point1_;
    const Page      *basepage_;
    Page::shared_ptr localpage_;
    Line             selline_;
    pwindow_t        newwindow_;
};

//////////////////////////////////////////////////////////////////////////

class Deleter
{
public:
    DEFINE_NERROR_OF_TYPE(geometry_error, cannot_delete, "Deleter: cannot delete");

    Deleter(Page *, point pt);
    Deleter(Page *, pwindow_t win);

private:

    void commit();

    pwindow_t get_window(point pt) const;
    Line prepare_border(border d) const;

    void one_side_delete(border, Line);
    void two_side_delete(border, Line, Line);
    void try_to_delete_in_direction(border); // throw(cannot_delete)

private:

    Page     *page_;
    pwindow_t window_;
};

//////////////////////////////////////////////////////////////////////////
