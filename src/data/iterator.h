#pragma once

#include "data/cursor.h"

namespace data
{

struct cursor_iterator: std::iterator<std::random_access_iterator_tag, double>
{
    double get_value() const
    {
        data::pcursor_t cursor(master_->clone(index));
        if (cursor->is_valid())
        {
            return cursor->get_time();
        }
        return 0.0;
    }

    double operator*() const
    {
        return get_value();
    }

    bool operator<(const cursor_iterator &d) const
    {
        return index < d.index;
    }

    bool operator==(const cursor_iterator &d) const
    {
        return index == d.index;
    }

    cursor_iterator &operator++()
    {
        ++index; return *this;
    }

    cursor_iterator operator++(int)
    {
        cursor_iterator temp(*this); ++temp; return temp;
    }

    cursor_iterator &operator--()
    {
        --index; return *this;
    }

    cursor_iterator operator--(int)
    {
        cursor_iterator temp(*this); --temp; return temp;
    }

    cursor_iterator &operator+=(int z)
    {
        index+=z; return *this;
    }

    cursor_iterator operator+(int z) const
    {
        return cursor_iterator(master_, index+z);
    }

    cursor_iterator operator-(int z) const
    {
        return cursor_iterator(master_, index-z);
    }

    size_t operator-(const cursor_iterator &l) const
    {
        return index-l.index;
    }

    cursor_iterator()
        : master_(),
          index(0)
    {}

    cursor_iterator(const cursor_iterator &l)
        : master_(l.master_),
          index(l.index)
    {}

    cursor_iterator(data::pcursor_t cur, int i=0)
        : master_(cur),
          index(i<0 ? cur->rest() : i)
    {}
private:

    data::pcursor_t master_;
    int        index;

};

}