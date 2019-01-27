#pragma once

#include "window.h"

class Line;

class Page
{
public:
    typedef windows_t::iterator iterator;
    typedef shared_ptr<Page> shared_ptr;

    DEFINE_ERROR_OF_TYPE(geometry_error, incorrect_copies, "Page: incorrect copies of page");
    DEFINE_ERROR_OF_TYPE(geometry_error, out_of_range, "Page: index is out of range");
    DEFINE_NERROR_OF_TYPE(geometry_error, invalid_page, "Page: invalid page");

    Page() : max_id_(0) {}
    virtual ~Page() {}

    template<typename _II>
    Page(_II start, _II finish)
    {
        windows_.assign(start, finish);
    }

    template<typename _II, typename _Pr>
    Page(_II start, _II finish, _Pr pr)
    {
        windows_.clear();
        stdex::copy_if(start, finish, std::back_inserter(windows_), pr);
    }

    windows_t::iterator begin()
    {
        return windows_.begin();
    }
    windows_t::iterator end()
    {
        return windows_.end();
    }

    windows_t::const_iterator begin() const
    {
        return windows_.begin();
    }
    windows_t::const_iterator end() const
    {
        return windows_.end();
    }

    struct p_cmp_id
    {
        bool operator()(pwindow_t a, pwindow_t b) const
        {
            return a->get_id() < b->get_id();
        }
    };

    int  append_new_window(HWND=0);
    void sort_windows();
    void push_back(Window *w);

    std::auto_ptr<Page> clone() const;
    void  assign_layout(Page *copy);

    unsigned size() const;
    pwindow_t at(unsigned index) const;

    pwindow_t find(int id);

    void erase(unsigned index);
    void erase(pwindow_t item);

    void rearrange_windows(double=1.0, bool horz=false);

    void UpdateLayout(const std::vector<CRect> &vctr);

    void check_validation() const;

    void serialization(serl::archiver &ar);

protected:

    windows_t windows_;

private:
    int max_id_;
};

