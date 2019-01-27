#pragma once

#include "node.h"

#include "LocationSetup.h"
#include "Collector.h"

namespace nodes
{

using location::pcollector;
using location::Setup;

struct locator_result : nodes::result
{
    Setup      setup;

    std::vector<pcollector> collectors;

    location::pclusters get_clusters() const;
};

class cache_calculator;

class locator
        : public node
{
public:
    locator();
    ~locator();

    object_id get_location_id();
    Setup get_location() const;
    void set_location(Setup);
    void set_parent(node* parent);

public:

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void restart();

private:

    shared_ptr<cache_calculator> cache_;
    struct config : process::config
    {
        Setup setup;
    } config_;

    struct processor;
    friend struct processor;
};

}

