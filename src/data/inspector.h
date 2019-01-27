#pragma once

#include "data/node.h"
#include "data/datafactory.h"
#include "data/minmax_map.h"

namespace nodes
{

struct inspector_result : nodes::result
{
    data::minmax_map  minmax_map;
    data::minmax_map  tdd_minmax_map;
};

class inspector : public node
{
public:
    inspector();
    ~inspector();

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void restart();

private:
    struct config : process::config
    {
    } config_;

    struct processor;

};

}
