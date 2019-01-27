#include "stdafx.h"

#include "data/inspector.h"
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

serl::mark_as_rtti<nodes::inspector> _inspector;

namespace nodes
{

inspector::inspector()
{
}

inspector::~inspector()
{
    detach_host();
}


void nodes::inspector::setup(process::hostsetup & setup)
{
    setup.name = _ls("inspector#Checking data");
    setup.weight = 0.5;
    setup.need_config = false;
}

void inspector::restart()
{
    restart_using(config_);
}

struct inspector::processor
        : process::processor_t<process::config,
                               inspector_result,
                               nodes::result>
{

    void ae_fiber(process::pfiber_info info,
                  std::vector<aera::chars> const& chars,
                  data::minmax_map& minmax_map,
                  pslice slice,
                  int begin, int end
                  )
    {
        for(int index=begin; index < end; ++index)
        {
            foreach(aera::chars c, chars)
            {
                const double value=slice->get_value(index, c);
                minmax_map.add_value(c, value);
            }

            if (!check_status(info, index, begin, end))
                return;
        }

    }

    void tdd_fiber(process::pfiber_info info,
                   std::vector<aera::chars> const& tddchars,
                   data::minmax_map& minmax_map,
                   pslice tddslice,
                   int begin, int end)
    {
        for(int index=begin; index < end; ++index)
        {
            foreach(aera::chars c, tddchars)
            {
                if (aera::traits::is_tdd_common_data(c))
                {
                    const double value=tddslice->get_value(index, c);
                    minmax_map.add_value(c, value);
                }
                else
                {
                    const unsigned count=tddslice->get_channel_count();
                    for (unsigned ch=0; ch<count; ++ch)
                    {
                        const double value=tddslice->get_value(index, c, ch);
                        minmax_map.add_value(c, value);
                    }
                }
            }

            if (!check_status(info, index, begin, end))
                return;
        }
    }

    virtual bool process()
    {
        // используется только внутри потока
        pslice slice = source->ae;
        std::vector<aera::chars> chars = slice->get_chars();

        pslice tddslice = source->tdd;
        std::vector<aera::chars> tddchars = tddslice->get_chars();

        int ssize = slice -> size();
        int tddsize = tddslice->size();

        int quantum_count = std::max(1, thread_count / 2);
        std::vector<data::minmax_map> maps1(quantum_count);

        for(int quantum = 0; quantum < quantum_count; ++quantum)
        {
            int begin = ssize * quantum / quantum_count;
            int end =   ssize * (quantum+1) / quantum_count;

            create_fiber(boost::bind(&inspector::processor::ae_fiber,
                                     this, _1,
                                     boost::cref(chars),
                                     boost::ref(maps1[quantum]),
                                     slice, begin, end));
        }

        // анализ TDD
        std::vector<data::minmax_map> maps2(quantum_count);
        for(int quantum = 0; quantum < quantum_count; ++quantum)
        {
            int begin = tddsize * quantum / quantum_count;
            int end =   tddsize * (quantum+1) / quantum_count;

            create_fiber(boost::bind(&inspector::processor::tdd_fiber,
                                     this, _1,
                                     boost::cref(tddchars),
                                     boost::ref(maps2[quantum]),
                                     tddslice, begin, end));
        }

        join_fibers();

        foreach(data::minmax_map const& map, maps1)
            result->minmax_map.merge(map);

        foreach(data::minmax_map const& map, maps2)
            result->tdd_minmax_map.merge(map);


        return true;
    }
};


process::processor *inspector::create_processor()
{
    return new inspector::processor;
}
}
