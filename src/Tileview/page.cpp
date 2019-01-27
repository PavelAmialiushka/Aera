#include "stdafx.h"

#include "math.h"

#include "page.h"
#include "wgeometry.h"

#include "utilites/foreach.hpp"
#include "utilites/serl/Archive.h"

int  Page::append_new_window(HWND hwnd)
{
    windows_t::iterator index=std::max_element(STL_II(windows_), p_cmp_id());
    max_id_=index!=windows_.end() ? (*index)->get_id() + 1 : 0 ;
    pwindow_t nwin=pwindow_t(new Window(hwnd, max_id_));
    windows_.push_back(nwin);
    return max_id_;
}

void  Page::rearrange_windows(double k, bool horz)
{
    unsigned maxim = windows_.size() ;

    unsigned rows=std::max((int)(sqrt((double)maxim)*k), 1);
    rows=std::min(rows, maxim);
    if (maxim==2) rows=1;

    windows_t::iterator index=windows_.begin();
    for (unsigned row=0; row<rows; ++row)
    {
        unsigned cols=(row==rows-1)
                      ? maxim-maxim/rows*(rows-1)
                      : maxim/rows;
        for (unsigned col=0; col<cols; ++col, ++index)
        {
            debug::Assert<out_of_range>(index!=windows_.end(), HERE);
            if (!horz)
                (*index)->set_rect(rect(CRect(col, row, col+1, row+1), CSize(cols, rows)));
            else
                (*index)->set_rect(rect(CRect(row, col, row+1, col+1), CSize(rows, cols)));
        }
    }
    check_validation();
}

void  Page::push_back(Window *w)
{
    windows_.push_back(boost::shared_ptr<Window>(w) );
}

unsigned Page::size() const
{
    return windows_.size();
}

pwindow_t Page::at(unsigned index) const
{
    return windows_.at(index);
}

void Page::erase(unsigned index)
{
    pwindow_t pw=windows_[index];
    if (pw->get_hwnd())
    {
        ::SendMessage(pw->get_hwnd(), WM_CLOSE, 0, 0);
    }
    windows_.erase(windows_.begin()+index);
}

void Page::erase(pwindow_t item)
{
    windows_t::iterator index=std::find(begin(), end(), item);
    debug::Assert<geometry_error>(index!=end(), HERE);
    windows_.erase(index);
}

pwindow_t Page::find(int id)
{
    windows_t::iterator index=
        std::find_if(begin(), end(), id_is(id));
    return index!=end()
           ? *index
           : pwindow_t();
}



//////////////////////////////////////////////////////////////////////////

std::auto_ptr<Page> Page::clone() const
{
    Page *page=new Page();
    for (unsigned index=0; index<size(); ++index)
    {
        page->push_back( new Window( *at(index) ) );
    }
	return std::auto_ptr<Page>(page);
}

void Page::assign_layout(Page *copy)
{
    foreach(pwindow_t other, *copy)
    {
        pwindow_t win=find(other->get_id());
        if (win)
        {
            win->set_rect(other->get_rect());
        }
        else
        {
            push_back( new Window(*other) );
        }
    }
}

void Page::UpdateLayout(const std::vector<CRect> &vctr)
{
    unsigned index=0;
    foreach(pwindow_t win, *this)
    {
        HWND hwnd=win->get_hwnd();
        if (hwnd)
        {
            CRect prev; GetWindowRect(hwnd, prev);
            bool repaint = (prev!=vctr[index]);

            CWindow(hwnd).MoveWindow(vctr[index], true);
            CWindow(hwnd).SendMessage(WM_SHOWWINDOW, 1, 0);
        }
        ++index;
    }
}

void Page::check_validation() const
{
    for (unsigned index=0; index<size(); ++index)
    {
        rect foo=at(index)->get_rect();
        debug::Assert<invalid_page>(!foo.empty(), HERE);
        for (unsigned row=index+1; row<size(); ++row)
        {
            rect bar=at(row)->get_rect();
            debug::Assert<invalid_page>((bar&foo).empty(), HERE);
        }
    }
}

void Page::serialization(serl::archiver &ar)
{
    ar.serial_static_ptr_container("windows", windows_);
    if (ar.is_loading())
    {
        check_validation();
    }
}

static bool compare_windows(pwindow_t a, pwindow_t b)
{
    return a->get_rect().top_ == b->get_rect().top_
           ? a->get_rect().left_ < b->get_rect() . left_
           : a->get_rect().top_ < b->get_rect().top_;
}

void Page::sort_windows()
{
    std::sort(begin(), end(), compare_windows);
}