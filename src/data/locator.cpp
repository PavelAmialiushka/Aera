#include "stdafx.h"

#include "utilites/foreach.hpp"

#include "Locator.h"
#include "LocationSetup.h"
#include "utilites/strings.h"
#include "utilites/Localizator.h"

#include "VesselCollector.h"
#include "nodeFactory.h"

namespace nodes
{

using namespace location;

struct cache_result : process::rslt
{
    shared_ptr<VesselCollectorCache> cache;
};

class cache_calculator
        : public process::host
{
public:

    ~cache_calculator()
    {
        detach_host();
    }

    void setup(process::hostsetup &setup)
    {
        setup.name = _ls("cache#Locator database creation");
        setup.need_config = true;
        setup.need_parent = 1;
        setup.weight = 10.0;
        setup.priority = -1;
    }

    void set_setup(VesselSetup const& setup)
    {
        if (config_.setup != setup)
        {
            config_.setup = setup;
            host::restart_using(config_);
        }
    }

    process::processor* create_processor();

    struct processor;
    struct config : process::config
    {
        VesselSetup setup;
    } config_;
};

struct cache_calculator::processor : process::processor_t<
        cache_calculator::config,
        /*out*/cache_result,
        /*in */process::rslt>
{
    void process_sensor(process::pfiber_info info, int index)
    {
        PVSensor current = result->cache->select_sensor(index);
        if (!current) return;

        int size = current->ray_step_count();
        for(int index=0;;++index)
        {
            if (!current->process_step())
                break;

            if (!check_status(info, index, 0, size))
                return;
        }
    }

    bool process()
    {
        result->cache.reset(
                    new VesselCollectorCache(
                        config->setup));

        int size = result->cache->init_and_get_size();

        for(int index=0; index < size; ++index)
        {
            create_fiber(boost::bind(
                  &processor::process_sensor,
                  this, _1, index));
        }

        join_fibers();

        status_helper = strlib::strf("steps: %d", size);
        return true;
    }
};

process::processor *cache_calculator::create_processor()
{
    return new cache_calculator::processor;
}

///////////////////////////////////////////////////////////////////////////////


void locator::setup(process::hostsetup & setup)
{
    setup.name = _ls("locator#Source location");
    setup.need_parent = cache_ ? 2 : 1;
    setup.weight = 0.5;
    if (config_.setup.vessel())
        setup.weight = 17;
}

locator::locator()
{
}

locator::~locator()
{
    detach_host();
}

object_id locator::get_location_id()
{
    return config_.setup.get_id();
}

void locator::restart()
{
    if (VesselSetup* setup = config_.setup.vessel())
    {
        if (!cache_)
        {
            cache_.reset( new cache_calculator() );
            cache_->set_parent( nodes::factory().get_root_node().get() );
            this->add_parent(cache_.get());
        }
        cache_->set_setup(*setup);
    } else
    {
        cache_.reset();
    }

    host::restart_using(config_);
}

void locator::set_location(Setup st)
{
    config_.setup = st;
    restart();
}

void locator::set_parent(node* parent)
{
    host::set_parent(parent);

    if (cache_)
    {
        add_parent(cache_.get());
    }
}

Setup locator::get_location() const
{
    return config_.setup;
}

struct locator::processor
        : process::processor_t<locator::config,
                               locator_result,
                               nodes::result, cache_result>
{
    bool process_collector(process::pfiber_info info,
                           pcollector col, pslice slice,
                           int index_from, int index_to)
    {
        // обновляем карту
        col->create_minmax_map();

        for(int index=index_from; index < index_to; ++index)
        {
            col->append_record(slice, index);

            if (!check_status(info, index, index_from, index_to))
                return false;
        }

        // finishing processing
        col->end_of_file();

        return true;
    }

    unsigned correct_position(pslice slice, unsigned end)
    {
        double t0 = slice->get_value(end);
        for(++end; end < slice->size(); ++end)
        {
            double t1 = slice->get_value(end);

            if (t1 - t0 > 0.5)
                return end;

            t0 = t1;
        }
        return end;
    }

    virtual bool process()
    {
        pslice slice = source->ae;
        assert(slice);
        unsigned size = slice->size();

//        assert(!config->setup.empty());
        if (config->setup.empty())
            return false;

        int collector_count = std::max(1u, std::min<unsigned>(size / 1000, processor::thread_count));
        unsigned position = 0;

        for(int index=0; index < collector_count; ++index)
        {
            // определяем положение окончания интервала
            unsigned end = (index+1) * size / collector_count;
            if (index == collector_count - 1) end = size;
            else end = correct_position(slice, end);

            // создаём коллектор
            pcollector collector = config->setup.create_collector(end - position);
            result->collectors.push_back(collector);

            // для vessel локации подгружаем кэш
            if (source2)
            {
                assert(source2->cache);
                collector->apply_cache(source2->cache.get());
            }

            // создаём фибер
            create_fiber(boost::bind(&locator::processor::process_collector,
                                     this, _1,
                                     collector, slice, position, end));

            position = end;
        }

        if (!join_fibers())
            return false;

        std::vector<pslice> evts, subs;
        foreach(pcollector col, result->collectors)
        {
            evts.push_back(col->make_event_slice(slice));
            subs.push_back(col->make_subhit_slice(slice));
        }

        result->ae = slice->merge(evts);
        result->ae_sub = slice->merge(subs);

        result->setup = config->setup;

        status_helper = strlib::strf("%d", result->ae->size());

        return true;
    }
};

process::processor *locator::create_processor()
{
    return new locator::processor;
}

location::pclusters locator_result::get_clusters() const
{
    pclusters r(new clusters);

    foreach(pcollector collector, collectors)
    {
        location::pclusters ps = collector->get_clusters();
        r->items.insert(r->items.end(), STL_II(ps->items));
    }

    return r;
}

}
