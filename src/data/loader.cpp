#include "stdafx.h"

#include "loader.h"
#include "collection_writer.h"

#include "utilites/document.h"
#include "utilites/serl/Archive.h"
#include "utilites/Localizator.h"

#include "blank_slice.h"
#include "aeslice.h"
#include "tddslice.h"
#include "rawslice.h"

#include <boost/format.hpp>

serl::mark_as_rtti<nodes::loader> _loader_;

namespace nodes
{

//////////////////////////////////////////////////////////////////////////

loader::loader()
{
}

loader::~loader()
{
    detach_host();
}

void loader::open(const std::string &filename)
{
    if (config_.filename_ == filename)
        return;

    config_.filename_ = filename;
    restart();
}

struct loader::processor
        : process::processor_t<
            loader::config,
            loader_result,
            process::rslt>
{
    virtual bool process()
    {
        shared_ptr<data::loader> loader_;

        data::pwriter_t writer=result->collection_.write_access();
        writer->clear();

        result->working_channels.reset( new data::channel_map );

        if (!writer) return false;

        process::tick_timer t;
        int size = 0;

        if (!config->filename_.empty())
        {
            loader_=data::factory::instance()->get_loader(config->filename_);

            for(;;)
            {
                if (!loader_->read(writer))
                    break;

                int index = loader_->get_progress(&size);

                if (!check_status(index, size))
                    return false;
            }

            loader_->get_active_channels(*result->working_channels);
        }

        DWORD elapsed = t.elapsed();

        // закрываем запись данных
        writer.reset();

        result->ae = pslice( new data::ae_slice(&result->collection_.ae()) );
        result->tdd = pslice( new data::tdd_slice(&result->collection_.tdd()) );
        result->raw = pslice( new data::raw_slice(&result->collection_.raw()) );
        result->ae_sub = pslice( new data::ae_slice() );
        result->sorted = result->collection_.ae().sorted();

        result->selected_channels.reset( new data::channel_map );
        result->selected_channels->assign(true);

        result->selection.reset( new hits::hitset );

        status_helper = strlib::strf("%d+%d",
                                     result->ae->size(),
                                     result->tdd->size());

        LOG(strlib::strf("loaded in %.3f[s], %.4f[MB/s] ",
                         elapsed/1000.0, elapsed ? size*1000/elapsed/1048576.0 : 0));

        return true;
    }
};

void loader::setup(process::hostsetup & setup)
{
    setup.name = _ls("loader#Data loading");
    setup.need_parent = 0;
    setup.auto_delete = false;
    setup.weight = 1.0;
}

process::processor *loader::create_processor()
{
    return new loader::processor;
}

void loader::restart()
{
    host::restart_using(config_);
}


}
