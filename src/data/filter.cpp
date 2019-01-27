#include "stdafx.h"

#include "data/filter.h"

#include "datafactory.h"
#include "data/syntaxer.h"

#include "utilites/serl/Archive.h"
#include "utilites/tokenizer.h"
#include "utilites/Localizator.h"

#include <boost/format.hpp>

#include "blank_slice.h"
#include "aeslice.h"

serl::mark_as_rtti<nodes::filter> _filter_;

namespace nodes
{
using namespace hits;

void filter::setup(process::hostsetup & setup)
{
    setup.name = _ls("filter#Filtering");
    setup.weight = 0.5;
}

pfilter_result filter_result::create(presult parent)
{
    pfilter_result r( new filter_result );
    r->inherite_from(parent);

    return r;
}

filter::filter()
{    
    config_.negative = false;
    attach_listener(this);
    restart();
}

filter::~filter()
{
    detach_listener(this);
    detach_host();
}

void filter::set_negative_flag(bool n)
{
    if (config_.negative == n)
        return;

    config_.negative = n;
    restart();
}

void filter::clear()
{
    config_.filters.clear();
    restart();
}

void filter::add_filter(const std::string &filters)
{
    tokenizer token(filters, "\n");
    foreach(std::string it, token)
    {
        filter_item item;
        item.fstring = it;
        config_.filters.push_back( item );
    }

    restart();
}

void filter::append_set(phitset hs)
{
    if (hs && hs->size())
    {
        filter_item item;
        item.fset = hs;
        config_.filters.push_back(item);
        restart();
    }
}


std::vector<filter_item> filter::get_filter_set() const
{
    return config_.filters;
}

void filter::set_filter_string(std::string s)
{
    filter_item item;
    item.fstring = s;

    config_.filters.clear();
    config_.filters.push_back(item);
    restart();
}

void filter::set_filter_set(const std::vector<filter_item> &foo)
{
    config_.filters = foo;
    restart();
}

void filter::restart()
{
    restart_using(config_);
}

struct filter::processor
        : process::processor_t<filter::config,
                               nodes::result,
                               nodes::result>
{
    static void append(filtering::vars_t &var,
                       const aera::chars *begin, const aera::chars *end,
                       aera::chars chr)
    {
        std::string name;
        name=aera::traits::get_short_name(chr); std::transform(STL_II(name), name.begin(), tolower);
        var[name]=chr;

        name=aera::traits::get_wide_name(chr); std::transform(STL_II(name), name.begin(), tolower);
        var[name]=chr;
    }

    pslice slice;
    struct work_filter_item : filter_item
    {
        bool            active;
        filtering::node tree;
    };
    std::vector<work_filter_item> items;


    void setup_filter_trees()
    {
        std::vector<aera::chars> chars=slice->get_chars();
        filtering::vars_t vars;
        foreach(aera::chars c, chars)
        {
            append(vars, &chars[0], &chars[0]+chars.size(), c);
        }

        items.reserve(config->filters.size());
        foreach(filter_item item, config->filters)
        {
            work_filter_item workitem;
            workitem.fset = item.fset;
            workitem.fstring = item.fstring;
            workitem.active = !workitem.fset;

            if (workitem.active)
                workitem.fset = phitset(new hits::hitset);

            std::string lower = workitem.fstring;
            std::transform(STL_II(lower), lower.begin(), tolower);

            try
            {
                workitem.tree=filtering::tree_maker(lower, vars);
            }
            catch (filtering::syntax_error &)
            {
                workitem.tree=filtering::tree_maker("0", vars);
            }

            items.push_back(workitem);
        }
    }

    virtual bool process()
    {
        slice = source->ae;
        unsigned ssize = slice->size();

        // подготовка к сбору данных
        std::vector<unsigned> array;
        array.reserve(slice->size());

        // подготовка к фильтрации
        setup_filter_trees();

        bool any_active = false;
        foreach(work_filter_item& item, items)
            if (item.active)
                any_active = true;

        // сканируем данные одновременно вычисляя заданное выражение
        if (any_active)
        {
            for (unsigned index=0; index<ssize; ++index)
            {
                foreach(work_filter_item& item, items)
                {
                    if (!item.active) continue;

                    double val=item.tree.get_value(slice.get(), index);
                    if (val==0)
                    {
                        item.fset->append(&slice->get_value(index));
                        break;
                    }
                }
            }
        }

        // сканируем данные, проверяя попадает ли конкретная запись
        // в диапазон, расчитанный ранее
        for(unsigned index=0; index < ssize; ++index)
        {
            const double *record=&slice->get_value(index);

            bool equal_to_zero=false;
            foreach(work_filter_item& item, items)
            {
                if (item.fset->get( record ))
                {
                    equal_to_zero=true;
                    break;
                }
            }

            if (config->negative)
                equal_to_zero = !equal_to_zero;

            if (!equal_to_zero)
            {
                array.push_back(index);
            }

            if (!check_status(index, ssize))
                return false;
        }

        data::ae_slice *as=new data::ae_slice;
        as->set_indexed_source(source->ae, array);
        result->ae.reset( as );

        status_helper = strlib::strf("%d", array.size());


        unsigned index=0;
        foreach(work_filter_item& item, items)
        {
            config->filters[index++].fset = item.fset;
        }

        return true;
    }
};

void filter::serialization(serl::archiver &ar)
{
    node::serialization(ar);

    std::vector<phitset> sets;
    std::vector<std::string> strings;

    if (ar.is_saving())
    {
        foreach(filter_item item, config_.filters)
        {
            sets.push_back( item.fset );
            strings.push_back( item.fstring );
        }
    }

    ar.serial_static_ptr_container("sets", sets);
    ar.serial_container("strings", strings);

    if (ar.is_loading())
    {
        config_.filters.clear();
        strings.resize( sets.size() );
        for(unsigned index=0; index < sets.size(); ++index)
        {
            filter_item item;
            item.fset = sets[index];
            item.fstring = strings[index];
            config_.filters.push_back( item );
        }
        restart();
    }
}

void filter::safe_on_finish(process::prslt r)
{
    shared_ptr<filter::config> end_config = boost::dynamic_pointer_cast<filter::config>(r->config);

    foreach(filter_item &item, config_.filters)
    {
        if (!item.fset && item.fstring.size())
        {
            foreach(filter_item& end_item, end_config->filters)
            {
                if (end_item.fstring == item.fstring)
                {
                    item.fset = end_item.fset;
                    break;
                }
            }
        }
    }
}

process::processor *filter::create_processor()
{
    return new filter::processor;
}

void filter::on_serial_create()
{
    clear();
}


}
