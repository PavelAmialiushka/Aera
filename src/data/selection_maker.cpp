#include "stdafx.h"

#include "data/syntaxer.h"
#include "data/grader.h"
#include "data/locator.h"

#include "nodeFactory.h"

#include "utilites/foreach.hpp"
#include "utilites/Localizator.h"

#include "data/selection_maker.h"

#include "slice.h"

using data::pslice;

selection_maker::selection_maker(select_action act)
{
    attach_listener(this);
    config_.action = act;
    restart_using(config_);
}

selection_maker::~selection_maker()
{
    detach_listener(this);
    detach_host();
}

void selection_maker::setup(process::hostsetup & setup)
{
    setup.name = _ls("selection_maker#Data selection");
    setup.auto_delete = true;
    setup.weight = 0.1;
}

class selection_maker::processor
        : public process::processor_t
        <selection_maker::config
        ,selection_result
        ,nodes::result>
{

    pslice slice;

    filtering::node     tree;
    std::vector<std::pair<double, double> > ranges;

    hits::phitset prev;
    hits::phitset current;

    void prepare_filtering_tree()
    {
        std::vector<aera::chars> string=slice->get_chars();

        filtering::vars_t vars;
        foreach(aera::chars chr, string)
        {
            std::string name;

            name=aera::traits::get_short_name(chr);
            std::transform(STL_II(name), name.begin(), tolower);
            vars[name]=chr;

            name=aera::traits::get_wide_name(chr);
            std::transform(STL_II(name), name.begin(), tolower);
            vars[name]=chr;
        }

        std::transform(STL_II(config->action.filter), config->action.filter.begin(), tolower);
        tree=filtering::tree_maker(config->action.filter, vars);
    }

    // вспомогательная функция для поиск подходящего временного интервала
    static bool compare(std::pair<double, double> const &pair,
        std::pair<double, double> v)
    {
        return pair.second < v.second;
    }

    void process_subhits(unsigned index, bool add_mode)
    {
        unsigned count=slice->get_subhit_count(index);
        for (unsigned jindex=0; jindex<count; ++jindex)
        {
            hits::hitref ref=slice->get_subhit(index, jindex);
            if (add_mode) current->append(ref);
            else          current->remove(ref);
        }
    }

    bool process()
    {
        slice=source->ae;
        unsigned ssize = slice->size();

        prepare_filtering_tree();

        prev = source->selection;
        if (config->action.mode == selection_add
                || config->action.mode == selection_sub)
        {
            current = prev;
        } else
        {
            current=hits::phitset(new hits::hitset);
            current->reserve(slice->size());
        }

        if (config->action.mode == selection_invert)
        {
            for(unsigned index=0; index < ssize; ++index)
            {
                const double *record=&slice->get_value(index);

                bool add_mode =prev->get(record)==0;
                if ( add_mode ) current->append(record);
                else            current->remove(record);

                // субхиты
                process_subhits(index, add_mode);

                if (!check_status(index, slice->size()))
                    return false;
            }
        }
        else if (config->action.use_time_ranges)
        {
            ranges.reserve(ssize);
            for(unsigned index=0; index < ssize; ++index)
            {
                // если критерий удовлетворяет хиту
                if (bool match=tree.get_value(slice.get(), index))
                {
                    const double time=slice->get_value(index);
                    const double start=time - config->action.range_begin;
                    const double finish=time + config->action.range_end;
                    if (!ranges.empty() && ranges.back().second > start)
                    {
                        // расширяем последний интервал
                        ranges.back().second=finish;
                    }
                    else
                    {
                        // создаем новый интервал
                        ranges.push_back( std::make_pair(start, finish) );
                    }
                }

                if (!check_status(index, ssize, 0, 2))
                    return false;
            }

            // проверяем попадают ли хиты в интервал
            for(unsigned index=0; index < ssize; ++index)
            {
                double const &time=slice->get_value(index);

                // подбор возможно подходящего интервала по времени окончания
                std::vector<std::pair<double, double> >::iterator iter=
                        std::lower_bound(STL_II(ranges),
                                         std::make_pair(0.0, time),
                                         compare);

                // проверка вхождения в интервал
                if (iter!=ranges.end() && iter->first <= time && time <= iter->second)
                {
                    bool add_mode = config->action.mode == selection_add
                            || config->action.mode == selection_set;
                    if (add_mode)
                        current->append(&time);
                    else
                        current->remove(&time);

                    process_subhits(index, add_mode);
                }


                if (!check_status(index, ssize, 1, 2))
                    return false;
            }
        } else
        {
            bool add_mode = config->action.mode == selection_add
                    || config->action.mode == selection_set;
            for(unsigned index=0; index<ssize; ++index)
            {
                double match=tree.get_value(slice.get(), index);

                const double *record=&slice->get_value(index);
                if (match)
                {
                    if (add_mode) current->append(record);
                    else          current->remove(record);

                    // субхиты
                    process_subhits(index, add_mode);
                }

                if (!check_status(index, slice->size()))
                    return false;
            }
        }

        result->selection = current;

        status_helper = strlib::strf("%d", current->size());

        return true;
    }
};

process::processor* selection_maker::create_processor()
{
    return new selection_maker::processor;
}

void selection_maker::safe_on_finish(process::prslt rslt)
{
    pselection_result result = boost::dynamic_pointer_cast<selection_result>(rslt);

    nodes::factory().set_selection(result->selection);
}
