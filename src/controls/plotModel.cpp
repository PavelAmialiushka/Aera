#include "stdafx.h"

#include "plotModel.h"

#include "data/grader.h"
#include "data/locator.h"

plot_model::plot_model(bool m)
    : plot_maker(m)
    , bitmap_maker_(this)
    , valid_(false)
{
    bitmap_maker_.attach_listener(this);
}

plot_model::~plot_model()
{
    bitmap_maker_.detach_listener(this);
}

bool plot_model::valid() const
{
    return valid_;
}

void plot_model::set_dimensions(int cx, int cy)
{
    bitmap_maker_.set_dimensions(CSize(cx, cy));
}

void plot_model::attach_listener(process::host_listener *ls)
{
    bitmap_maker_.attach_listener(ls);
}

void plot_model::detach_listener(process::host_listener *ls)
{
    bitmap_maker_.detach_listener(ls);
}

std::vector<double> plot_model::get_timemarks() const
{
    if (valid())
        return result_->timemarks;

    return std::vector<double>();
}

shared_ptr<CBitmap> plot_model::get_bitmap() const
{
    if (valid())
        return result_->bitmap_;

    CDC desktop_dc=::GetWindowDC( 0 );
    CSize s = bitmap_maker_.get_dimensions();
    HBITMAP bitmap = ::CreateCompatibleBitmap( desktop_dc, s.cx, s.cy);

    CDC dc2( ::CreateCompatibleDC(desktop_dc) );
    dc2.SelectBitmap( bitmap );
    dc2.FillSolidRect(0, 0, s.cx, s.cy, 0x3f3f3f);
    dc2.DeleteDC();

    return shared_ptr<CBitmap>( new CBitmap( bitmap ));
}

plot::range plot_model::get_xrange() const
{
    if (valid())
        return result_->screen->get_xrange();

    return plot_maker::get_xrange();
}


plot::range plot_model::get_yrange() const
{
    if (valid())
        return result_->screen->get_yrange();

    return plot_maker::get_yrange();
}

axe_info plot_model::get_xaxe_info() const
{
    axe_info info = plot_maker::get_xaxe_info();
    if (valid())
    {
        plot::range range = result_->screen->get_xrange();
        info.lo = range.lo;
        info.hi = range.hi;
    }
    return info;
}

axe_info plot_model::get_yaxe_info() const
{
    axe_info info = plot_maker::get_yaxe_info();
    if (valid())
    {
        plot::range range = result_->screen->get_yrange();
        info.lo = range.lo;
        info.hi = range.hi;
    }
    return info;
}

location::pclusters plot_model::get_clusters() const
{
    shared_ptr<nodes::locator_result> r=
        process::get_parent_result<
            nodes::locator_result>(this, false);
    if (!r) return location::pclusters();

    return r->get_clusters();
}

bool plot_model::get_working_location_setup(location::Setup & result) const
{
    shared_ptr<nodes::grader_result> r=
        process::get_parent_result<
            nodes::grader_result>(this, false);
    if (!r) return false;

    if (r->location.empty()) return false;

    result = r->location;
    return true;
}

void plot_model::get_xminmax(bool &a, double &z, double &x) const
{
    plot_maker::get_xminmax(a, z, x);
    if (valid())
    {
        plot::range range = result_->screen->get_xrange();
        z = range.lo;
        x = range.hi;
    }
}

void plot_model::get_yminmax(bool &a, double &z, double &x) const
{
    plot_maker::get_yminmax(a, z, x);
    if (valid())
    {
        plot::range range = result_->screen->get_yrange();
        z = range.lo;
        x = range.hi;
    }
}

void plot_model::safe_on_restart()
{
    valid_ = false;
}

void plot_model::safe_on_finish(process::prslt rsl)
{
    valid_ = true;
    result_ = boost::dynamic_pointer_cast<bitmap_result>(rsl);
    assert(result_);
}

