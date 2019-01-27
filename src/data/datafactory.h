#pragma once

#include "data/data_fwrd.h"
#include "data/collection_writer.h"

namespace data
{

class loader
{
public:
    virtual void get_active_channels(channel_map& result) const=0;
    virtual bool read(data::pwriter_t writer)=0;
    virtual int get_progress(int* size) const=0;

    virtual void init(data::pwriter_t);
};

class factory : public utils::singleton<factory>
{
public:
    shared_ptr<loader> get_loader(std::string);
};



}
