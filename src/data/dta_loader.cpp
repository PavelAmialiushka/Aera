#include "stdafx.h"

#include "data/dta_loader.h"
#include "data/data_error.h"

namespace data
{

dta_loader::dta_loader(std::string path)
    : record_(this)
    , stream_(path)
{
    std::fill(STL_II(channels_), false);
}

bool dta_loader::check_file_format()
{    
    char tag[] = "DiSP (r)";
    char buf[] = "";

    stream_.skip(4);
    std::string s = stream_.read_str(SIZEOF(tag));
    if (!stream_) return false;

    stream_.seek(0);

    return s == tag;
}

void dta_loader::get_active_channels(channel_map& result) const
{
    result = channels_;
}

bool dta_loader::read(data::pwriter_t writer)
{
    if (!record_.read_msg(stream_, writer))
        return false;

    return  true;
}

int dta_loader::get_progress(int* size) const
{
    unsigned stream_size = stream_.size();
    if (size) *size = stream_size ? stream_size : 1;
    return stream_size ? stream_.offset(): 1;
}

}
