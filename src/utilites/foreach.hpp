//////////////////////////////////////////////////////////////////////////
//
// utilites library
// 
// Written by Pavel Amialiushka 
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <iterator>

//////////////////////////////////////////////////////////////////////////

// range

template<typename T>
struct range_traits
{
    static T begin()
    {
        return static_cast<T>(0);
    }

    static T step()
    {
        return static_cast<T>(1);
    }

    static bool up(const T& step)
    {
        return static_cast<T>(0) < step;
    }
};

template<typename T>
class range_container
{
public:
    class iterator
    {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef const T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef const T* pointer;
        typedef const T& reference;

        iterator(T first) :
          m_first(first),
          m_step()
        {
        }

        iterator(T first, T step) :
          m_first(first),
          m_step(step)
        {
        }

        bool operator==(const iterator& right) const
        {
            return !(*this != right);
        }

        bool operator!=(const iterator& right) const
        {
            // Check sign
            return range_traits<T>::up(m_step) ?
                   m_first < right.m_first :
                   right.m_first < m_first;
        }

        iterator& operator++()
        {
            m_first += m_step;
            return *this;
        }

        iterator operator++(int)
        {
            T temp(*this);
            
            operator++();

            return temp;
        }

        reference operator*() const
        {
            return m_first;
        }

    private:
        T m_first;
        T m_step;
    };

    typedef iterator const_iterator;

    range_container(T first, T last, T step) :
        m_first(first, step),
        m_last(last)
    {
    }

    const_iterator begin() const
    {
        return m_first;
    }

    const_iterator end() const
    {
        return m_last;
    }

private:
    iterator m_first;
    iterator m_last;
};

template<typename T>
range_container<T> range(T first, T last, T step)
{
    return range_container<T>(first, last, step);
}

template<typename T>
range_container<T> range(T first, T last)
{
    return range(first, last, range_traits<T>::step());
}

template<typename T>
range_container<T> range(const T& last)
{
    return range(range_traits<T>::begin(), last);
}

//////////////////////////////////////////////////////////////////////////

// keys

template< typename T > struct keys_impl 
{
  template< typename T > struct keys_iterator 
  : std::iterator<std::forward_iterator_tag, const typename T::key_type, typename T::iterator::distance_type>
  {
    typedef const typename T::key_type value_type;
    typedef typename T::const_iterator base_iterator;

    keys_iterator( base_iterator const& iter )
    : iterator(iter)
    {
    }

    //////////////////////////////////////////////////////////////////////////
  
    bool operator==(keys_iterator const& other) const
    {
      return iterator==other.iterator;
    }

    bool operator!=(keys_iterator const& other) const
    {
      return iterator!=other.iterator;
    }

    //////////////////////////////////////////////////////////////////////////
  
    value_type const& operator*() const
    {
      return iterator->first;
    }

    keys_iterator& operator++() 
    {
      ++iterator;
      return *this;
    }


  private:
    base_iterator iterator;
  };

  //////////////////////////////////////////////////////////////////////////
  
  keys_impl(T& cont)
  : container(cont)
  {
  }

  keys_iterator<T> begin() const
  {
    return keys_iterator<T>( container.begin() );
  }

  keys_iterator<T> end() const
  {
    return keys_iterator<T>( container.end() );
  }

private:
  
  T& container;

};

template<typename T> keys_impl<T> keys(T& t) { return keys_impl<T>(t); }

//////////////////////////////////////////////////////////////////////////
// values

template< typename T > struct values_impl 
{
  template< typename T > struct values_iterator 
  : std::iterator<std::forward_iterator_tag, typename T::referent_type, typename T::iterator::distance_type>
  {
    typedef typename T::referent_type  value_type;
    typedef typename T::const_iterator base_iterator;

    values_iterator( base_iterator const& iter )
    : iterator(iter)
    {
    }

    //////////////////////////////////////////////////////////////////////////
  
    bool operator==(values_iterator const& other) const
    {
      return iterator==other.iterator;
    }

    bool operator!=(values_iterator const& other) const
    {
      return iterator!=other.iterator;
    }

    //////////////////////////////////////////////////////////////////////////
  
    value_type const & operator*() const
    {
      return iterator->second;
    }

    values_iterator& operator++() 
    {
      ++iterator; return *this;
    }


  private:
    base_iterator iterator;
  };

  //////////////////////////////////////////////////////////////////////////
  
  values_impl(T& cont)
  : container(cont)
  {
  }

  values_iterator<T> begin() const
  {
    return values_iterator<T>( container.begin() );
  }

  values_iterator<T> end() const
  {
    return values_iterator<T>( container.end() );
  }

private:
  
  T& container;
};

template<typename T> values_impl<T> values(T& t) { return values_impl<T>(t); }

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

// foreach

namespace foreach_detail
{
    struct wrapper_holder{};

    template <typename T>
    struct wrapper
        :wrapper_holder
    {
        mutable T wrapped;
        wrapper(const T& t)
            :wrapped(t) 
        {}
    };

    template <typename T> 
    wrapper<T> wrap(const T& t)
    { 
        return t; 
    }

    template <typename T>
    T& unwrap(const wrapper_holder& b, int, T*)
    { 
        return static_cast<const wrapper<T>&>(b).wrapped; 
    }

    template <typename T>
    T& unwrap(const wrapper_holder& b, const T&)
    { 
        return unwrap<T>(b, 0, 0); 
    }

    template <typename T>
    bool is_wrapped_equal(const wrapper_holder& lhs, const wrapper_holder& rhs, const T&)
    { 
        return unwrap<T>(lhs, 0, 0)==unwrap<T>(rhs, 0, 0);
    }
}

#define foreach_(Decl, First, Last)\
if(bool _fe_break = false){}else \
    for(const foreach_detail::wrapper_holder\
             &_fe_cur=foreach_detail::wrap(First)\
            ,&_fe_end=foreach_detail::wrap(Last)\
        ;!_fe_break && !foreach_detail::is_wrapped_equal(_fe_cur, _fe_end, Last)\
        ;++foreach_detail::unwrap(_fe_cur, Last)\
        )\
        if(!(_fe_break=true)){}else\
        for(Decl=*foreach_detail::unwrap(_fe_cur, Last);_fe_break;_fe_break=false)\
// end macro

#define foreach(Decl, Cont)    foreach_(Decl, (Cont).begin(), (Cont).end())
