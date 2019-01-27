#pragma once

//////////////////////////////////////////////////////////////////////////

#include "utilites/serl/element.h"

#include "utilites/serl/rtti.h"
#include "utilites/serl/wrappers.h"

#include <boost/lexical_cast.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
//////////////////////////////////////////////////////////////////////////

namespace serl
{

class archiver;

struct serializable
{
    virtual ~serializable() {}
    virtual void serialization(serl::archiver &)=0;
    virtual void on_serial_initiate() {}
};

template<bool> struct initate_policy
{
    template<class T> static void initiate(T *self)
    {
        *self=T();
    }
};

template<> struct initate_policy<1>
{
    static void initiate(serializable *self)
    {
        self->on_serial_initiate();
    }
};

template<class T> void initiate(T *self)
{
    initate_policy<boost::is_base_and_derived<serializable, T>::value>::initiate(self);
}

//////////////////////////////////////////////////////////////////////

class archiver
{
public:
    enum { in, out };

protected:
    pelement element_;

protected:
    archiver(pelement el)
        : element_(el)
    {}

public:

    virtual ~archiver()
    {
    }

    virtual archiver* clone(std::string name) const=0;
    virtual int  get_type() const=0;

    bool is_loading() const {
        return get_type()==in; }

    bool is_saving() const {
        return get_type()==out; }

    //////////////////////////////////////////////////////////////////////////

    template<class T> void serial(T &value)
    {
        if (is_saving())
        {
            element_->save(value);
        }
        else
        {
            element_->load(value);
        }
    }

    template<class T> void serial(std::string name, T &value)
    {
        if (is_saving())
        {
            element_->save(name, value);
        }
        else
        {
            if (!element_->load(name, value))
            {
                initiate(&value);
            }
        }
    }

    template<class T> void serial_static_ptr(std::string name, shared_ptr<T> &value)
    {
        T *pointer=value.get();
        if (is_saving())
        {
            serial_static_ptr(name, pointer);
        }
        else
        {
            pointer=0;
            serial_static_ptr(name, pointer);
            value=shared_ptr<T>(pointer);
        }
    }

    template<class T> void serial_static_ptr(std::string name, T *&pointer_value)
    {
        if (is_saving())
        {
            element_->save(name, *pointer_value);
        }
        else
        {
            if (!pointer_value) pointer_value=new T();

            if (!element_->load(name, *pointer_value))
            {
                initiate(pointer_value);
            }
        }        
    }

    template<class T> void serial_virtual_ptr(std::string name, shared_ptr<T> &value)
    {
        T *pointer=value.get();
        if (is_saving())
        {
            serial_virtual_ptr(name, pointer);
        }
        else
        {
            pointer=0;
            serial_virtual_ptr(name, pointer);
            value=shared_ptr<T>(pointer);
        }
    }


    template<class T> void serial_virtual_ptr(std::string name, T *&value)
    {
        scoped_ptr<archiver> child(clone(name));
        child->serial_virtual_ptr(value);
    }

    template<class T> void serial_virtual_ptr(T *&value)
    {
        if (is_saving())
        {
            // записываем имя типа
            element_->save("#typeid", rtti::name(value));

            // сериализация            
            rtti::instance()->invoke_saver(value, *this);
        }
        else
        {
            // имя записанного типа
            std::string tname;
            if (element_->load("#typeid", tname))
            {
                // удалаем предыдущее значение
                delete value; value=0;

                // создаем новый объект заданного типа
                value=rtti::instance()->create(value, tname);

                // сериализация
                rtti::instance()->invoke_loader(value, *this);
            }
            else
            {
                ; // создавать нечего
            }
        }
    }

#define SERIAL_CONTAINER_X(serialX)                                                     \
    template<class C> void serialX##_container(std::string name, C& vector)             \
    {                                                                                   \
        typedef typename C::iterator It;                                                \
        typedef typename C::value_type value_type;                                      \
        scoped_ptr<archiver> child(clone(name));                                        \
        if (is_saving())                                                                \
        {                                                                               \
            unsigned count=vector.size();                                               \
            child->serial("#size", count);                                              \
                                                                                        \
            unsigned index=0;                                                           \
            for(It iter=vector.begin(), end=vector.end();                               \
                iter!=end; ++iter, ++index)                                             \
            {                                                                           \
                std::string id="#"+boost::lexical_cast<std::string>(index);             \
                child->serialX(id, *iter);                                              \
            }                                                                           \
        }                                                                               \
        else                                                                            \
        {                                                                               \
            unsigned count=0;                                                           \
            child->serial("#size", count);                                              \
            if (count)                                                                  \
            {                                                                           \
                vector.clear();                                                         \
                unsigned index=0;                                                       \
                for(;count;--count, ++index)                                            \
                {                                                                       \
                  std::string id="#"+boost::lexical_cast<std::string>(index);           \
                                                                                        \
                  value_type value;                                                     \
                  child->serialX(id, value);                                            \
                  vector.push_back(value);                                              \
                }                                                                       \
            }                                                                           \
        }                                                                               \
    }

    SERIAL_CONTAINER_X(serial)
    SERIAL_CONTAINER_X(serial_static_ptr)
    SERIAL_CONTAINER_X(serial_virtual_ptr)

#undef SERIAL_CONTAINER_X

};

//////////////////////////////////////////////////////////////////////////

struct load_archiver : archiver
{
private:
    load_archiver(const load_archiver &other);

private:

    friend class element;
    load_archiver(pelement elem)
        : archiver(elem)
        , factory_()
    {
    }

private:

    pelement_factory factory_;

public:

    load_archiver(element_factory *factory)
        : archiver( factory->root_node() )
        , factory_(factory)
    {
    }

    ~load_archiver()
    {
    }

    void close()
    {
    }

    virtual archiver* clone(std::string name) const
    {
        return new load_archiver(element_->lookup(name));
    }

    virtual int get_type() const
    {
        return in;
    }
};

struct save_archiver : archiver
{
private:
    save_archiver(const save_archiver &); // forbiden

private:

    friend class element;
    save_archiver(pelement nd)
        : archiver(nd)
        , factory_()
    {
    }

private:

    pelement_factory factory_;

public:

    save_archiver(element_factory *factory)
        : archiver(factory->root_node())
        , factory_(factory)
    {
    }

    ~save_archiver()
    {
        close();
    }

    void close()
    {
        if (factory_)
            factory_->do_save_data();
    }

    virtual archiver* clone(std::string name) const
    {
        return new save_archiver(element_->lookup(name));
    }

    virtual int get_type() const
    {
        return out;
    }
};



} // namespace serial



//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
