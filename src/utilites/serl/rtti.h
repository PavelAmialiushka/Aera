#pragma once

#include "utilites\Exception.h"
#include "utilites\singleton.h"

#include "utilites/serl/exception.h"

namespace serl
{

//////////////////////////////////////////////////////////////////////////

class archiver;

//////////////////////////////////////////////////////////////////////////

class rtti: public utils::singleton<rtti>
{
public:
    DEFINE_ERROR_OF_TYPE(error, type_not_found, "type is not registered");
protected:
    friend class utils::singleton<rtti>;

    typedef boost::function2<void, void *, archiver &> loader_t;
    typedef boost::function2<void, void *, archiver &> saver_t;
    typedef boost::function0<void *> creator_t;

    struct type_
    {
        type_() {}
        type_(loader_t l, saver_t s, creator_t c) : loader(l), saver(s), creator(c)
        {
        }
    public:
        loader_t loader;
        saver_t saver;
        creator_t creator;
    };

public:

    void add_callback(std::string name, loader_t lcb, saver_t scb, creator_t c)
    {
        map_[name]=type_(lcb, scb, c);
    }

    template<class T>
    void invoke_loader(T *ptr, archiver &ar)
    {
        map_[name(ptr)].loader(ptr, ar);
    }

    template<class T> static std::string name(T *t)
    {
        debug::Assert<fault>(t!=0, HERE);
        return typeid(*t).name();
    }

    template<> static std::string name(void *t)
    {
        debug::Assert<type_not_found>(0, HERE);
        return "unknown";
    }

    template<class T>
    void invoke_saver(T *t, archiver &ar)
    {
        if (t!=0)
        {
            if (!registered(t))
                debug::Assert<type_not_found>(registered(t), HERE);
            map_[name(t)].saver(t, ar);
        }
    }

    template<class T>
    T *create(T *, std::string name)
    {
        debug::Assert<type_not_found>(registered(name), HERE);
        return reinterpret_cast<T *>( map_[name].creator() );
    }

    bool registered(std::string name)
    {
        return map_.count(name)>0;
    }

    template<class T>
    bool registered(T *t)
    {
        return registered(name(t));
    }

private:
    std::map<std::string, type_> map_;
};

template<class T>
class rtti_helper
{
public:
    static void loader(void *hm, serl::archiver &x)
    {
        reinterpret_cast<T *>(hm)->serialization(x);
    }
    static void saver(void *hm, serl::archiver &x)
    {
        reinterpret_cast<T *>(hm)->serialization(x);
    }
    static void *creator()
    {
        return new T();
    }
};


template<class T>
struct mark_as_rtti
{
    mark_as_rtti()
    {
        rtti::instance()->add_callback(
            typeid(T).name(),
            &rtti_helper<T>::loader,
            &rtti_helper<T>::saver,
            &rtti_helper<T>::creator
        );
    }
};


template<class T>
class rtti_abstract_helper
{
public:
    static void loader(void *hm, serl::archiver &x)
    {
        reinterpret_cast<T *>(hm)->serialization(x);
    }
    static void saver(void *hm, serl::archiver &x)
    {
        reinterpret_cast<T *>(hm)->serialization(x);
    }
    static void *creator()
    {
        debug::Assert<fault>(0, HERE);
        return NULL;
    }
};

template<class T>
struct mark_as_rtti_abstract
{
    mark_as_rtti_abstract()
    {
        rtti::instance()->add_callback(
            typeid(T).name(),
            &rtti_abstract_helper<T>::loader,
            &rtti_abstract_helper<T>::saver,
            &rtti_abstract_helper<T>::creator
        );
    }
};



}
