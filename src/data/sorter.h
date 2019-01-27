#pragma once

#include "node.h"
#include "minmax_map.h"

namespace nodes
{

class sorter
        : public node
{
public:

    sorter();
    ~sorter();

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

private:    
    struct processor;
};

}
