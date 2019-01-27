#pragma once

#include "utilites\mpl.h"
#include "utilites\exception.h"

#include "utilites\tokenizer.h"
#include "utilites\strings.h"
#include "utilites\utilites.h"

#include "utilites/serl/exception.h"

#include "utilites/foreach.hpp"

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN64
#define _64_BIT_TIME_T 1
#endif

#if _MSC_VER > 1200
#ifndef _USE_32BIT_TIME_T
#define _64_BIT_TIME_T 1
#endif
#endif

namespace serl
{

typedef shared_ptr<class element> pelement;
typedef shared_ptr<class element_factory> pelement_factory;

class element_factory
{
public:
    virtual ~element_factory() {}

    virtual pelement root_node() const=0;

    virtual void do_save_data() {}
};

///////////////////////////////////////////////////

class element
        : public boost::enable_shared_from_this<element>
{
public:
    explicit element(std::string);

    std::string name() const;
    std::string value() const;

    bool is_folder() const;
    bool is_named_value() const;
    bool is_value() const;

    // поиск и всавка значений
    bool lookup(std::string name, pelement& result);
    pelement lookup(std::string name);

    // вставка именнованного значения
    void insert(std::string name, std::string value);

    void set_value(std::string);

    // получить по индексу
    pelement at(unsigned index) const;
    unsigned size() const;

    // модификация дерева
    pelement take(std::string);
    void insert(std::string where, pelement node);

public:
    //////////////////////////////////////////////////////////////////////////
    // interface

    template<class T> bool load(std::string name, T &value)
    {
        pelement child;
        if (lookup(name, child))
        {
            child->load(value);
            return true;
        }
        return false;
    }

    // пользовательские типы

    template<class T> void load(T &value)
    {
        value.serialization(load_archiver(shared_from_this()));
    }

    // строка

    void load(std::string &val)
    {
        val = value();
    }

    // другие примитивные типы
    template<class T> void primitive_loader(T &result)
    {
        try {
        result=boost::lexical_cast<T>(value());
        } catch(boost::bad_lexical_cast&)
        {}
    }
    void load(long &value) { primitive_loader(value);  }
    void load(unsigned long &value) { primitive_loader(value); }
    void load(int &value) { primitive_loader(value); }
    void load(unsigned &value) { primitive_loader(value); }
    void load(double &value) { primitive_loader(value); }
    void load(float &value) { primitive_loader(value); }
    void load(bool &value) { primitive_loader(value); }
#ifdef _64_BIT_TIME_T
    void load(time_t& value) { primitive_loader(value); }
#endif

    //////////////////////////////////////////////////////////////////////////

    // именованное значение
    template<class T> void save(std::string name, T &value)
    {
        lookup(name)->save(value);
    }

    // пользовательские типы

    template<class T> void save(T &value)
    {
        value.serialization(save_archiver(shared_from_this()));
    }

    // строка

    void save(std::string value)
    {
        set_value(value);
    }

    // другие примитивные типы

    template<class T> void primitive_saver(T &value)
    {
        set_value( boost::lexical_cast<std::string>(value));
    }
    void save(long value) { primitive_saver(value); }
    void save(unsigned long value) { primitive_saver(value); }
    void save(int value) { primitive_saver(value); }
    void save(unsigned value) { primitive_saver(value); }
    void save(double value) { primitive_saver(value); }
    void save(float value) { primitive_saver(value); }
    void save(bool value) { primitive_saver(value); }
#ifdef _64_BIT_TIME_T
    void save(time_t value) { primitive_saver(value); }
#endif

private:
    std::string name_;
    std::vector<pelement> children_;
};

}
