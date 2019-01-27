#pragma once

#include "data/data_fwrd.h"
#include "data/collection.h"
#include "data/datafactory.h"
#include "data/collection_writer.h"

#include "data/dta_record.h"

#include "utilites/streams.h"

namespace data
{

class dta_loader : public loader
{
public:
    dta_loader(std::string path);
    bool check_file_format();

    virtual void get_active_channels(channel_map& result) const;
    virtual bool read(data::pwriter_t writer);
    virtual int get_progress(int*) const;

private:

    fs::istream stream_;

    dta_record  record_;
    channel_map channels_;

    friend class dta_record;
};

}
