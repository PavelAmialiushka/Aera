#pragma once

#include "utilites/strings.h"

namespace serl
{

template<class T, class X>
class indirect_t
{
    T *self;
    typedef void (T::*set_value)(X);
    set_value set;
    typedef X (T::*get_value)(void) const;
    get_value get;
public:
    indirect_t()
    {
    }

    indirect_t(T *f, set_value s, get_value g) : self(f), set(s), get(g)
    {
    }

    void serialization(archiver &ar)
    {
        X x;
        if (ar.is_saving())
        {
            x=(self->*get)();
            ar.serial(x);
        }
        else
        {
            ar.serial(x);
            (self->*set)(x);
        }
    }
};

//////////////////////////////////////////////////////////////////////////

template<class T, class X>
indirect_t<T, X> indirect(T *self, const X &, void (T::*set_value)(X), X(T::*get_value)(void)const)
{
    return indirect_t<T, X>(self, set_value, get_value);
}

/////////////////////////////////////////////////////////////////////////

template<class T>
class makeint_t
{
public:

    makeint_t() : ref(0) {}
    makeint_t(T &a) : ref(&a) {}

    void serialization(serl::archiver &ar)
    {
        int y=ref ? *ref : 0;

        ar.serial(y);

        if (ar.is_loading())
        {
            if (ref) *ref=static_cast<T>(y);
        }
    }

private:
    T *ref;
};

template<class T> makeint_t<T> makeint(T &t)
{
    return makeint_t<T>(t);
}

//////////////////////////////////////////////////////////////////////////

template<class T>
class make_container_t
{
public:
    make_container_t() : ref(0) {}
    make_container_t(T &a) : ref(&a) {}

    void serialization(serl::archiver &ar)
    {
        typedef typename T::iterator       iterator;
        typedef typename T::key_type       key_type;
#if _MSC_VER == 1200
        typedef typename T::referent_type  referent_type;
#else
        typedef typename T::mapped_type  referent_type;
#endif
        if (ar.is_saving())
        {
            unsigned size=ref->size();

            ar.serial("#size", size);
            unsigned index=0;
            for (iterator iter=ref->begin(); iter!=ref->end(); ++iter, ++index)
            {
                key_type key=iter->first;
                ar.serial(strlib::strf("#%d-k", index), key);
                ar.serial(strlib::strf("#%d-r", index), iter->second);
            }
        }
        else
        {
            ref->clear();
            unsigned size=0;
            ar.serial("#size", size);
            for (unsigned index=0; index<size; ++index)
            {
                key_type key;
                referent_type referent;
                ar.serial(strlib::strf("#%d-k", index), key);
                ar.serial(strlib::strf("#%d-r", index), referent);
                (*ref)[key]=referent;
            }
        }
    }
private:
    T *ref;
};

template<class T> make_container_t<T> make_container(T &t)
{
    return make_container_t<T>(t);
}

//////////////////////////////////////////////////////////////////////////


}
