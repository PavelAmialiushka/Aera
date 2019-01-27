#pragma once

#include "data_fwrd.h"

#include "node.h"
#include "hitset.h"
#include "inspector.h"

namespace nodes
{

class selector
        : public node
{
public:
    selector();
    ~selector();

    void set_selection(hits::phitset);

private:

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

private:

    struct config : process::config
    {
        hits::phitset   selection;
    } config_;

    struct processor;
};

}
