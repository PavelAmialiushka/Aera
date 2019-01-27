#include "stdafx.h"

#include "data/modifier.h"
#include "data/minmax_map.h"

#include "utilites/serl/Archive.h"
#include "utilites/Localizator.h"

#include <boost/format.hpp>

#include "locator.h"

#include "aecollection.h"
#include "tddcollection.h"
#include "blank_slice.h"
#include "aeslice.h"
#include "tddslice.h"

serl::mark_as_rtti<nodes::modifier> _modifier_;

namespace nodes
{

void nodes::modifier::setup(process::hostsetup & setup)
{
    setup.name = _ls("modifier#Data correction");
    setup.weight = 0.5;
    setup.need_config = false;
}

modifier::modifier()
{
}

modifier::~modifier()
{
    detach_host();
}


void modifier::set_parametric_data(parametric *prm)
{
    if (0==memcmp(prm, config_.parametrics_, SIZEOF(config_.parametrics_) * sizeof(*config_.parametrics_)))
        return;

    std::copy(prm, prm+SIZEOF(config_.parametrics_), config_.parametrics_);
    restart();
}

void modifier::get_parametric_data(parametric *prm)
{
    std::copy(STL_AA(config_.parametrics_), prm);
}

void modifier::restart()
{
    restart_using(config_);
}

struct modifier_result : nodes::result
{
    shared_ptr<data::ae_collection> ae_collection;
    shared_ptr<data::tdd_collection> tdd_collection;
};

struct modifier::processor
        : process::processor_t<modifier::config,
                               modifier_result,
                               nodes::result>
{
    virtual bool process()
    {
        // ничего не делаем без конфига
        if (!config)
            return true;

        pslice slice = source->ae;
        unsigned ssize=slice->size();
        std::vector<aera::chars> ae_chars, src_ae_chars;
        std::vector<aera::chars> chars0 = slice->get_chars();
        foreach(aera::chars c, chars0)
        {
            if (aera::C_OriginalParametric1 <= c
                    && c <=aera::C_OriginalParametric4)
            {
                src_ae_chars.push_back(c);
                ae_chars.push_back(
                            aera::chars(
                                c
                                - aera::C_OriginalParametric1
                                + aera::C_Parametric1)
                            );
            }
        }
        bool has_ae = !ae_chars.empty();

        pslice tddslice = source->tdd;
        unsigned tddsize = tddslice->size();
        std::vector<aera::chars> tdd_chars, src_tdd_chars;
        std::vector<aera::chars> chars1 = tddslice->get_chars();
        foreach(aera::chars c, chars1)
        {
            if (aera::C_OriginalParametric1 <= c
                    && c <=aera::C_OriginalParametric4)
            {
                src_tdd_chars.push_back(c);
                tdd_chars.push_back(
                            aera::chars(
                                c
                                - aera::C_OriginalParametric1
                                + aera::C_Parametric1)
                            );
            }
        }
        bool has_tdd = !tdd_chars.empty();

        unsigned total_size = 0;
        if (has_ae) total_size += ssize;
        if (has_tdd) total_size += tddsize;

        unsigned total_index = 0;

        if (has_ae)
        {
            result->ae_collection.reset( new data::ae_collection );
            result->ae_collection->set_typestring(&ae_chars[0], ae_chars.size());

            data::minmax_map minmax_map;
            for(unsigned index=0; index<ssize; ++index, ++total_index)
            {
                double *record, *field;
                record = field = result->ae_collection->allocate_record(ae_chars.size());

                foreach(aera::chars c, src_ae_chars)
                {
                    int nparam = c - aera::C_OriginalParametric1;
                    double addend=config->parametrics_[nparam].addend;
                    double factor=config->parametrics_[nparam].factor;
                    double v = slice->get_value(index, c);

                    *field = v * factor + addend;
                    minmax_map.add_value( aera::chars(nparam + aera::C_Parametric1), *field);

                    ++field;
                }
                result->ae_collection->append_allocated_record(record);

                if (!check_status(total_index, total_size))
                    return false;
            }

            // подготовка ае срезки
            pae_slice as = slice->clone_ae();
            as->add_layer(result->ae_collection.get());
            as->add_minmax_map( minmax_map );
            result->ae = as;
        }

        if (has_tdd)
        {
            result->tdd_collection.reset( new data::tdd_collection );            
            result->tdd_collection->set_common_types(&tdd_chars[0], tdd_chars.size());
            result->tdd_collection->set_channel_count(
                        source->tdd->get_channel_count());

            data::minmax_map minmax_map;
            for(unsigned index=0; index<tddsize; ++index, ++total_index)
            {
                double *record = result->tdd_collection->allocate_record(tdd_chars.size());

                unsigned subindex=0;
                foreach(aera::chars c, src_ae_chars)
                {
                    int nparam = c - aera::C_OriginalParametric1;
                    double addend=config->parametrics_[nparam].addend;
                    double factor=config->parametrics_[nparam].factor;
                    double v = tddslice->get_value(index, c);

                    record[subindex] = v * factor + addend;

                    minmax_map.add_value(
                                aera::chars(nparam + aera::C_Parametric1),
                                record[subindex]);

                    ++subindex;
                }
                result->tdd_collection->append_allocated_record(record);

                if (!check_status(total_index, total_size))
                    return false;
            }

            // подготовка ае срезки
            data::ptdd_slice tdd = tddslice->clone_tdd();
            tdd->add_layer(result->tdd_collection.get());
            tdd->add_minmax_map( minmax_map );
            result->tdd = tdd;
        }

        return true;
    }
};


process::processor *modifier::create_processor()
{
    return new modifier::processor;
}
}
