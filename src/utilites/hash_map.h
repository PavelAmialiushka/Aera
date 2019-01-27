#pragma once

#include <iterator>
#include <algorithm>
#include <functional>
#include <cassert>

#include "hash_base.h"

//////////////////////////////////////////////////////////////////////////

#include <map>

template<
         typename T,
         typename U,
         typename _Hash=hash<T>,
         typename _KEq=std::equal_to<T>
         >
class hash_map
{
public:
    typedef std::pair<T, U> value_type;
    typedef T               key_type;
    typedef U               referent_type;
    typedef std::pair<value_type, bool> inner_type;

public:
    //////////////////////////////////////////////////////////////////////////

    class hash_iterator
        : public std::iterator<std::bidirectional_iterator_tag, value_type, ptrdiff_t>
    {
        typedef typename std::pair<value_type, bool> inner_type;

        inner_type  *begin;
        inner_type  *end;
        inner_type  *current;

    public:
        hash_iterator(inner_type *b, inner_type *e, inner_type *c)
            : begin(b)
            , end(e)
            , current(c)
        {}

        hash_iterator(const hash_iterator &other)
            : begin(other.begin)
            , end(other.end)
            , current(other.current)
        {
        }

        void swap(hash_iterator &other)
        {
            std::swap(begin, other.begin);
            std::swap(end, other.end);
            std::swap(current, other.current);
        }

        hash_iterator &operator=(hash_iterator other)
        {
            swap(other);
            return *this;
        }

    protected:

        void increment()
        {
            assert(current!=end);
            for (; ++current, !current->second && current!=end;)
                ;
        }

        void decrement()
        {
            for (; assert(current!=begin), --current, !current->second;)
                ;
        }

    public:

        hash_iterator &operator++()
        {
            increment();
            return *this;
        }

        hash_iterator operator++(int)
        {
            hash_iterator tmp(*this);
            increment();
            return tmp;
        }

        hash_iterator &operator--()
        {
            decrement();
            return *this;
        }

        hash_iterator operator--(int)
        {
            hash_iterator tmp(*this);
            decrement();
            return tmp;
        }

        const value_type &operator*() const
        {
            assert(current!=end);
            return current->first;
        }

        value_type &operator*()
        {
            assert(current!=end);
            return current->first;
        }

        const value_type *operator->() const
        {
            assert(current!=end);
            return &current->first;
        }

        value_type *operator->()
        {
            assert(current!=end);
            return &current->first;
        }

        bool operator==(const hash_iterator &other) const
        {
            return current==other.current;
        }

        bool operator!=(const hash_iterator &other) const
        {
            return current!=other.current;
        }

        friend class hash_map<T, U, _Hash, _KEq>;
    };

    //////////////////////////////////////////////////////////////////////////

    typedef hash_iterator      iterator;
    typedef hash_iterator      const_iterator;

    //////////////////////////////////////////////////////////////////////////

private:

    inner_type *begin_;
    inner_type *end_;
    unsigned    size_;

    _KEq        key_equal;
    _Hash       hasher;

private:

    unsigned _position(inner_type *current) const
    {
        return current-begin_;
    }

    unsigned _hash_pos(inner_type *current) const
    {
        return hasher(current->first.first) % capacity();
    }

    void _increment(inner_type *&current) const
    {
        ++current;
        if (current==end_) current=begin_;
    }

    void _decrement(inner_type *&current) const
    {
        current+=sz;
        while (current>end) current-=(end-begin);
    }

    inner_type *_advance(inner_type *current, size_t sz) const
    {
        if (sz>0)
        {
            current+=sz; while (current>end_) current-=(end_-begin_);
        }
        else
        {
            current-=sz; while (current<begin_) current+=(end_-begin_);
        }
        return current;
    }

    bool _find(const key_type &value, inner_type *&iter) const
    {
        unsigned pos=hasher(value) % capacity();
        iter=_advance(begin_, pos);

        // bottleneck
        for (; iter->second && !key_equal(value, iter->first.first); _increment(iter))
            ;
        return iter->second;
    }

    void _replace_vacant(inner_type *vacant, inner_type *start)
    {
        if (start==end_) start=begin_;
        for (; start->second; _increment(start))
        {
            if ( _hash_pos(start) == _position(vacant) )
            {
                *vacant = *start;
                vacant=start;
            }
        }
        *vacant=std::make_pair(value_type(), false);
    }

public:

    iterator find(const key_type &k)
    {
        inner_type *result;
        return _find(k, result)
               ? iterator(begin_, end_, result)
               : end();
    }

    const_iterator find(const key_type &k) const
    {
        inner_type *result;
        return _find(k, result)
               ? iterator(begin_, end_, result)
               : end();
    }

    referent_type& operator[](const key_type& k)
    {
        inner_type *result;
        if (_find(k, result))
        {
            return result->first.second;
        } else
        {
            return insert(value_type(k,referent_type())).first->second;
        }
    }

    std::pair<iterator, bool> insert(const value_type& v)
    {
        reserve(size_);
        ++size_;

        inner_type *iter;
        bool succ=_find(v.first, iter);
        *iter=std::make_pair(value_type(v.first, v.second), true);
        return std::make_pair(iterator(begin_, end_, iter), succ);
    }

    template<typename II>
    void insert(II first, II last)
    {
        std::map<int,int> s;
        for (; first!=last; ++first)
        {
            assert(first.current);
            insert(*first);
        }
    }

    void erase(const key_type &k)
    {
        inner_type *iter;
        if (_find(k, iter))
            erase(const_iterator(begin_, end_, iter));
    }

    void erase(const_iterator where)
    {
        inner_type *vacant=where.current;
        assert(vacant->second);
        --size_;
        _replace_vacant(vacant, _advance(vacant, 1));
    }

    void clear()
    {
        hash_map<T, U, _Hash, _KEq>(size_).swap(*this);
    }

    void reserve(unsigned size)
    {
        if (capacity() * max_load_factor() < size)
        {
            hash_map<T, U, _Hash, _KEq> tmp( size );
            tmp.insert(begin(), end());
            swap(tmp);
        }
    }

public:

    unsigned size() const
    {
        return size_;
    }

    bool empty() const
    {
        return !size();
    }

    double load_factor() const
    {
        return double(size())/capacity();
    }

    unsigned collisions() const
    {
        unsigned counter=0;
        for (iterator iter=begin(); iter!=end(); ++iter)
        {
            if ( _hash_pos(iter.current) != _position(iter.current))
                ++counter;
        }
        return counter;
    }

    double collision_factor() const
    {
        return !empty() ? static_cast<double>(collisions())/size() : 0.0;
    }

    double max_load_factor() const
    {
        return 0.5;
    }

    unsigned capacity() const
    {
        return end_-begin_;
    }

    iterator begin()
    {
        inner_type *first=begin_;
        for (; first!=end_ && !first->second; ++first)
            ;
        return iterator(begin_, end_, first);
    }

    const_iterator begin() const
    {
        inner_type *first=begin_;
        for (; first!=end_ && !first->second; ++first)
            ;
        return const_iterator(begin_, end_, first);
    }

    iterator end()
    {
        return iterator(begin_, end_, end_);
    }

    const_iterator end() const
    {
        return const_iterator(begin_, end_, end_);
    }

private:

    void realloc(unsigned sz)
    {
        delete[] begin_;
        sz=details::next_prime(sz);
        begin_=new inner_type[sz];
        end_=begin_+sz;
    }

public:
    hash_map(unsigned sz=256)
        : size_(0), begin_(0), end_(0)
    {
        realloc(static_cast<unsigned>( sz/max_load_factor()+1 ));
    }

    ~hash_map()
    {
        delete[] begin_;
    }

    hash_map(const hash_map &other)
        : size_(0), begin_(0), end_(0)
    {
        reserve(other.size());
        for (iterator iter=other.begin(); iter!=other.end(); ++iter)
            insert(*iter);
    }

    hash_map &operator=(hash_map other)
    {
        swap(other);
        return *this;
    }

    void swap(hash_map &other)
    {
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }
};

