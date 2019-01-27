#include "stdafx.h"

#include "data/grader.h"
#include "data/minmax_map.h"

#include "utilites/serl/Archive.h"
#include "utilites/Localizator.h"

#include <boost/format.hpp>

#include "locator.h"

#include "blank_slice.h"
#include "aeslice.h"
#include "tddslice.h"

#include "inspector.h"
#include "locator.h"

serl::mark_as_rtti<nodes::grader> _grader_;

namespace nodes
{

grader::grader()
{
    std::fill(STL_II(config_.channels_), true);
    restart();
}

grader::~grader()
{
    detach_host();
}


void grader::get_channel_list(data::channel_map &result) const
{
    result = config_.channels_;
}

void grader::set_channel_sheme(channel_sheme const &sheme)
{
    config_.sheme_ = sheme;

    for (unsigned ch=0; ch<config_.channels_.size(); ++ch)
    {
        config_.channels_[ch] = sheme.isactive(ch+1);
    }

    restart();
}

void grader::get_channel_sheme(channel_sheme &sheme) const
{
    sheme = config_.sheme_;
}


void grader::set_stage(const stage &stg)
{
    config_.stage_=stg;
    restart();
}

stage grader::get_stage() const
{
    return config_.stage_;
}

void nodes::grader::setup(process::hostsetup & setup)
{
    setup.name = _ls("grader#Selecting");
    setup.need_parent = 2;
    setup.weight = 0.4;
}

void grader::restart()
{
    restart_using(config_);
}

struct grader::processor
        : process::processor_t<grader::config, grader_result,
                               nodes::result, inspector_result>
{
    virtual bool process()
    {
        pslice slice = source->ae;
        unsigned ssize=slice->size();

        // какие хиты принимаются
        hits::hitset accepted_set;
        accepted_set.reserve(slice->size());

        std::vector<unsigned>   ae_indexes;      ///< индексы отфильтрованных записей
        ae_indexes.reserve(slice->size());

        for(unsigned index=0; index<ssize; ++index)
        {
            const double *record=&slice->get_value(index);

            bool filtered_out=false;

            unsigned channel=static_cast<unsigned>(record[1])-1;
            if (channel>=config->channels_.size() || !config->channels_[ channel ])
            {
                filtered_out=true;
            }

            if (config->stage_.start!=config->stage_.end)
            {
                if (!config->stage_.contains( record[0] ))
                    filtered_out=true;
            }

            if (!filtered_out)
            {
                ae_indexes.push_back( index );     // запоминаем индекс
                accepted_set.append( record ); // запоминаем сам хит
            }

            if (!check_status(index, ssize*3))
                return false;
        }

        pslice sub_slice = source->ae_sub;
        unsigned sub_size = sub_slice ? sub_slice->size() : 0;
        std::vector<unsigned>   sub_indexes;

        // определяем, был ли выделен выделен основной хит
        // если да то выделяем и его
        for(unsigned index=0; index<sub_size; ++index)
        {
            double const *ref= &sub_slice->get_value(index, aera::C_FirstHit);
            if (accepted_set.contains(ref))
                sub_indexes.push_back(index);

            if (!check_status(index + sub_size, sub_size*3))
                return false;
        }

        pslice tddslice = source->tdd;
        unsigned tdd_size = tddslice->size();
        std::vector<unsigned>   tdd_indexes;

        for(unsigned index = 0; index<tdd_size; ++index)
        {
            const double *record=&tddslice->get_value(index);
            bool filtered_out=false;

            if (!config->stage_.empty())
            {
                if (!config->stage_.contains( record[0] ) )
                {
                    filtered_out = true;
                }
            }

            if (!filtered_out)
            {
                tdd_indexes.push_back( index );
            }

            if (!check_status(index + tdd_size*2, tdd_size*3))
                return false;
        }

        // finish

        data::minmax_map map;

        if (!config->stage_.empty())
        {
            map.add_value(aera::C_Time, config->stage_.start);
            map.add_value(aera::C_Time, config->stage_.end);
        }

        //////////////////////////////////////////////////////////////////////////

        // подготовка ае срезки

        pslice as = slice->clone_indexed( ae_indexes );
        as->add_minmax_map( map );
        as->add_minmax_map( source2->minmax_map );
        result->ae = as;

        pslice sub = sub_slice->clone_indexed( sub_indexes );
        sub->add_minmax_map( map );
        sub->add_minmax_map( source2->minmax_map );
        result->ae_sub = sub;

        pslice ts = source->tdd->clone_indexed(tdd_indexes);
        ts->add_minmax_map( map );
        ts->add_minmax_map( source2->tdd_minmax_map );
        result->tdd = ts;

        result->selected_channels.reset( new data::channel_map );
        *result->selected_channels = config->channels_;

        result->selection = source2->selection;

        if (shared_ptr<locator_result> r = boost::dynamic_pointer_cast<locator_result>(source))
            result->location = r->setup;

        status_helper = strlib::strf("%d+%d+%d",
                                     result->ae->size(),
                                     result->ae_sub->size(),
                                     result->tdd->size());

        return true;
    }
};


process::processor *grader::create_processor()
{
    return new grader::processor;
}
}
