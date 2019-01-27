//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace utils
{

//////////////////////////////////////////////////////////////////////////

class singleton_base
{
protected:
    singleton_base(int);
    virtual ~singleton_base() {}
public:
    int destroy()
    {
        delete this;
        return 0;
    }
};

void destroy_singletons();

//////////////////////////////////////////////////////////////////////////

#if _MSC_VER < 1300
#define SOMECAST const_cast<LONG*>
#else
#define SOMECAST
#endif

//////////////////////////////////////////////////////////////////////////
// thread safe primitives

inline LONG atom_set(volatile LONG *ptr, LONG value)
{
    return InterlockedExchange(SOMECAST(ptr), value);
}

inline LONG atom_get(volatile LONG *ptr)
{
    return InterlockedExchangeAdd(SOMECAST(ptr), 0);
}

#undef SOMECAST

struct auto_lock
{
    auto_lock(volatile LONG *ptr)
        :ptr_(ptr)
    {
        while (atom_set(ptr_, 1))
            Sleep(1);
    }
    ~auto_lock()
    {
        atom_set(ptr_, 0);
    }
    volatile LONG *ptr_;
};

//////////////////////////////////////////////////////////////////////////
// singleton class template

template<class T, int Priority=1>
class singleton : public singleton_base
{
public:
    singleton()
        : singleton_base(Priority)
    {}

    static T *instance()
    {
        static volatile LONG mutex_flag=0;

        // вход ограничен одним потоком
        auto_lock lock(&mutex_flag);

        if (!instance_pointer())
        {
            instance_pointer()=new T();
        }
        return instance_pointer();
    }

    static T& inst()
    {
        return *instance();
    }

    virtual ~singleton()
    {
        instance_pointer()=0;
    }

private:
    static T *&instance_pointer()
    {
        static T *inst_=0;
        return inst_;
    }
};

}

