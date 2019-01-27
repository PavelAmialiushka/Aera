#include "stdafx.h"

#include "window.h"
#include "TVView.h"
#include "TVCreator.h"

#include "utilites/serl/Archive.h"

Window::Window(HWND h, int id, rect r)
    : hwnd_(h), id_(id), r_(r), contents_(0)
{
}

Window::Window(const Window &win)
    : id_(win.id_), r_(win.r_), hwnd_(win.hwnd_), contents_(0)
{
}

Window::~Window()
{
}

void Window::set_rect(const rect &r)
{
    r_=r;
}

void Window::set_hwnd(HWND h)
{
    hwnd_=h;
};

void Window::scale(const rect &prev, const rect &rc)
{
    r_.left_ =::scale(r_.left_, prev.left_, prev.right_, rc.left_, rc.right_);
    r_.right_=::scale(r_.right_, prev.left_, prev.right_, rc.left_, rc.right_);
    r_.top_   =::scale(r_.top_,   prev.top_, prev.bottom_, rc.top_, rc.bottom_);
    r_.bottom_=::scale(r_.bottom_, prev.top_, prev.bottom_, rc.top_, rc.bottom_);
}

int Window::get_id() const
{
    return id_;
}

const rect &Window::get_rect() const
{
    return r_;
}

HWND Window::get_hwnd() const
{
    return hwnd_;
}

bool Window::operator==(int id) const
{
    return id_==id;
}

bool Window::operator==(Window other) const
{
    return get_rect()==other.get_rect();
}

CTVCreator *Window::get_contents() const
{
    return contents_;
}

void Window::serialization(serl::archiver &ar)
{
    bool has_window=hwnd_;

    ar.serial("has_window", has_window);
    ar.serial("rect", r_);
    ar.serial("id", id_);

    if (has_window)
    {
        if (ar.is_saving())
        {
            ::SendMessage(hwnd_, TVVM_GETCLASSPTR, (WPARAM)&contents_, 0);
        }
        ar.serial_virtual_ptr("contents", contents_);
    }
}
