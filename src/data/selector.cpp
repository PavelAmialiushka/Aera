#include "stdafx.h"

#include "selector.h"
#include "inspector.h"

#include "aeslice.h"
#include "tddslice.h"

#include "utilites/Localizator.h"

namespace nodes
{

selector::selector()
{
    set_selection(hits::phitset(new hits::hitset));
}

selector::~selector()
{
    detach_host();
}

void selector::set_selection(hits::phitset sel)
{
    config_.selection = sel;
    restart_using(config_);
}

void selector::setup(process::hostsetup &setup)
{
    setup.name = _ls("selector#Data selection");
    setup.need_parent = 2;
    setup.weight = 0;
}

class selector::processor
        : public process::processor_t
        <selector::config, inspector_result
        ,nodes::result
        ,inspector_result>
{
    bool process()
    {
        result->selection = config->selection;
        result->minmax_map = source2->minmax_map;
        result->tdd_minmax_map = source2->tdd_minmax_map;

        return true;
    }
};


process::processor *selector::create_processor()
{
    return new selector::processor;
}

}
