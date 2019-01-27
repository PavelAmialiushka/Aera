//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "monpac.h"
#include "monpac_result.h"

#include "node.h"
#include "stage.h"

#include "channel.h"
#include "criterion.h"
#include "outputter.h"

#include "utilites/foreach.hpp"
#include "utilites/Localizator.h"

namespace monpac
{

classifier::classifier(criterion const &c)
{
    config_.criterion_ = c;
    restart();
}

classifier::~classifier()
{
   detach_host();
}

static bool operator<(channel const &a, channel const &b)
{
    return a.get_number() < b.get_number();
}


class classifier::processor
        : public process::processor_t<classifier::config,
                                      monpac_result,
                                      nodes::result>
{
public:
    processor()
        : prev_hit_(-1, 0, 0, 0, 0)
    {
    }
private:
    std::map<unsigned, unsigned> map_;
    hit prev_hit_;

    void append(double time, double chan, double amp, double ener, double duration)
    {
        hit h=hit(time, chan, amp, ener, duration);

        if (map_.count((int)h.channel)==0)
        {
            result->channels.push_back( pcalculator( new calculator(&config->criterion_, (int)h.channel) ));
            map_[(int)h.channel]=result->channels.size()-1;
        }
        pcalculator pchan=result->channels[map_[(int)h.channel]];
        pchan->append(h);
        prev_hit_=h;
    }

    // input data


    virtual bool process()
    {
        pslice slice=source->ae;

        stage        cstage;
        cstage=config->criterion_.get_stage();

        //  working data

        for(unsigned index = 0; index < slice->size(); ++index)
        {
            const double *record=&slice->get_value(index);
            if (cstage.contains(record[0]))
            {
                append(
                    slice->get_value(index, aera::C_Time),      slice->get_value(index, aera::C_Channel),
                    slice->get_value(index, aera::C_Amplitude), slice->get_value(index, aera::C_Energy),
                    slice->get_value(index, aera::C_Duration)
                );
            }

            if (!check_status(index, slice->size()))
                return false;
        }

        for (unsigned index=0; index<result->channels.size(); ++index)
        {
            result->ch_info_.push_back(
                result->channels[index]->calculate());
        }

        std::sort(STL_II(result->ch_info_));
        result->criterion_ = config->criterion_;

        status_helper = strlib::strf("%d", result->ch_info_.size());

        return true;
    }
};


criterion classifier::get_criterion() const
{
    return config_.criterion_;
}


void classifier::setup(process::hostsetup &setup)
{
    setup.name = _ls("monpac#Data classification");
}

process::processor *classifier::create_processor()
{
    return new classifier::processor;
}

void classifier::restart()
{
    restart_using( config_ );
}
}
