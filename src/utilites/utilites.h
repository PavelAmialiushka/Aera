//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <limits>
#include <math.h>

//////////////////////////////////////////////////////////////////////////

#undef SIZEOF
#define SIZEOF(buf) (sizeof(buf)/sizeof(*buf))
#define ENDOF(buf) ((buf)+(sizeof(buf)/sizeof(*buf)))

#define STL_II(c) (c).begin(), (c).end()
#define STL_AA(c) (c), ENDOF(c)

//////////////////////////////////////////////////////////////////////////

template<class T, class U>
T scale(U v, U l, U r, T t, T b)
{
    return static_cast<T>( t+(v-l)*(b-t)/(r-l) );
}

template<class T, class U>
T scale(U v, U r, T t)
{
    return static_cast<T>( v * t / r );
}

template<class T>
T nonull(T t, T t2)
{
    return t==0 ? t2 : t;
}

template<class T, class U> bool between_ne(T t, U l, U r)
{
    return l < t && t < r;
}

template<class T, class U> bool between_e(T t, U l, U r)
{
    return l <= t && t <= r;
}

// гарантированно возвращает число из диапазона [0 .. d)
inline int mod2(int a, int d)
{
    a = a % d;
    if (a < 0) return mod2(a + d, d);
    return a;
}

// гарантированно возвращает число из диапазона [-d/2, d/2)
inline int mod2_half(int a, int d)
{
    a = a % d;
    if (a <= -d/2) return a + d;
    else if (a > d/2) return a - d;
    return a;
}

// гарантированно возвращает число из диапазона [0 .. d)
inline double fmod2(double a, double d)
{
    a = fmod(a, d);
    if (a < 0) return fmod2(a + d, d); // охватывает случай для a = -eps
    return a;
}

// гарантированно возвращает число из диапазона [-d/2, d/2)
inline double fmod2_half(double a, double d)
{
    a = fmod(a, d);
    if (a <= -d/2) return a + d;
    else if (a > d/2) return a - d;
    else return a;
}

template<typename T>
inline T ensure_minmax(T x, T mi, T ma)
{
    return std::max(std::min(x, ma), mi);
}


template<class T, class U> class tied
{
public:
    tied(T &t, U &u) : first(t), second(u) {}
    tied<T, U> &operator=(const std::pair<T, U> &pair)
    {
        first=pair.first; second=pair.second;
        return *this;
    }

    tied<T, U> &operator=(std::pair<T, U> &pair)
    {
        first=pair.first; second=pair.second;
        return *this;
    }

public:
    T &first;
    U &second;
};

template<class T, class U> tied<T, U> tie(T &t, U &u)
{
    return tied<T, U>(t, u);
}

template<class T>
void widen_range_begin(T& mini, T& maxi)
{
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        maxi = mini = std::numeric_limits<T>::quiet_NaN();
    } else
    {
        mini = maxi = std::numeric_limits<T>::max()-1;
    }
}

template<class T>
void widen_range(T& mini, T& maxi, T value)
{
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        if (_isnan(value)) return;
        if (_isnan(mini)) mini = value;
        if (_isnan(maxi)) maxi = value;
    } else
    {
        if (mini == maxi &&
                mini == std::numeric_limits<T>::max()-1)
        {
            mini = maxi = value;
            return;
        }
    }

    if (value < mini) mini = value;
    if (value > maxi) maxi = value;
}

template<class T>
void widen_range_end(T& mini, T& maxi)
{
    if (std::numeric_limits<T>::has_quiet_NaN)
    {
        if (_isnan(mini))
        {
            mini = 0; maxi = 1;
        }
    } else
    {
        if (mini == maxi && mini == std::numeric_limits<T>::max()-1)
        {
            mini = 0; maxi = 1;
        }
    }
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
class atomic
{
public:
    atomic() : value_() {}
    atomic(T t) : value_(t) {}

    atomic<T> &operator=(T t)
    {
        boost::mutex::scoped_lock lk(mutex_);
        value_=t;
        return *this;
    }

    T &swap(T &t)
    {
        boost::mutex::scoped_lock lk(mutex_);
        std::swap(value_, t);
        return t;
    }

    T swap(T t)
    {
        boost::mutex::scoped_lock lk(mutex_);
        std::swap(value_, t);
        return t;
    }

    T get() const
    {
        boost::mutex::scoped_lock lk(mutex_);
        return value_;
    }

    void operator==(const T &);
    void operator!=(const T &);
    void operator<(const T &);
    void operator<=(const T &);
    void operator>(const T &);
    void operator>=(const T &);

private:
    mutable boost::mutex mutex_;
    T value_;
};

//////////////////////////////////////////////////////////////////////////

namespace utils
{

class array2
{
    unsigned width_;
    unsigned size_;
    double *array_;
public:
    array2(double *array, unsigned width, unsigned size=0)
        : width_(width), array_(array), size_(size/width)
    {}

    array2(std::vector<double> &array, unsigned width)
        : width_(width), array_(&array[0]), size_(array.size()/width) {}

    double *operator[](unsigned index) const
    {
        return array_+index*width_;
    }
    unsigned size() const
    {
        return size_;
    }
    unsigned width() const
    {
        return width_;
    }
};

}
