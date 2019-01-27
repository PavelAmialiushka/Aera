//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "singleton.h"

#include <queue>

namespace utils
{

//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////

struct cmp_pairs
{
    bool operator()(const std::pair<singleton_base *, int> &p1,
                    const std::pair<singleton_base *, int> &p2) const
    {
        return p1.second < p2.second;
    }
};

//////////////////////////////////////////////////////////////////////////

class singleton_container
{
public:
    singleton_container()
    {
    }

    ~singleton_container()
    {
    }

    static singleton_container *instance()
    {
        static volatile LONG mutex_flag=0;
        auto_lock lock(&mutex_flag);

        if (!instance_)
        {
            instance_=new singleton_container;
            atexit(destroy);
        }
        return instance_;
    }

    static void __cdecl destroy()
    {
        static volatile LONG mutex_flag=0;
        auto_lock lock(&mutex_flag);

        if (instance_)
            instance_->destroy_all();
    }

    void attach(singleton_base *ptr, int pri)
    {
        singletons_.push(std::make_pair(ptr, pri));
    }

    void destroy_all()
    {
        std::vector<singleton_base *> buffer;
        while (singletons_.size())
        {
            buffer.insert(buffer.begin(), singletons_.top().first);
            singletons_.pop();
        }
        std::for_each(buffer.begin(), buffer.end(),
                      bind(&singleton_base::destroy, _1));

        instance_=0;
        delete this;
    }

private:

    std::priority_queue<
    std::pair<singleton_base *, int>,
        std::vector<std::pair<singleton_base *, int> >,
        cmp_pairs >
        singletons_;

    static singleton_container *instance_;
};

//////////////////////////////////////////////////////////////////////////

singleton_container *singleton_container::instance_;

//////////////////////////////////////////////////////////////////////////

void destroy_singletons()
{
    singleton_container::instance()->destroy_all();
}

//////////////////////////////////////////////////////////////////////////

singleton_base::singleton_base(int p)
{
    singleton_container::instance()->attach(this, p);
}


}