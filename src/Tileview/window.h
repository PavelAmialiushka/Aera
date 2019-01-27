#pragma once

#include "geometry.h"

struct CTVCreator;

class Window
{
public:
    Window(HWND h=0, int id=0, rect r=rect());
    Window(const Window &win);
    ~Window();

    void set_rect(const rect &r);
    void set_hwnd(HWND h);

    void scale(const rect &prev, const rect &rc);

    int         get_id() const;
    const rect &get_rect() const;
    HWND        get_hwnd() const;
    CTVCreator *get_contents() const;

    bool operator==(int id) const;
    bool operator==(Window other) const;

    void serialization(serl::archiver &ar);

private:

    HWND  hwnd_;
    CTVCreator *contents_;

    int   id_;
    rect  r_;
};

inline std::ostream &operator<<(std::ostream &s, const Window &self)
{
    return s << self.get_id()  << ": " << self.get_rect();
}

//////////////////////////////////////////////////////////////////////////

typedef shared_ptr<Window> pwindow_t;
typedef std::vector<pwindow_t > windows_t;

struct id_is : std::unary_function<pwindow_t, bool>
{
    int id_;
    id_is(int a) : id_(a) {}
    bool operator()(pwindow_t win) const
    {
        return win->get_id()==id_;
    }
};

//////////////////////////////////////////////////////////////////////////
