#include "Stdafx.h"

#include "dta_saver.h"

namespace data
{

dta_saver::dta_saver(std::string path)
    : record_(this)
{
    stream_.open(path.c_str(), std::ios::binary);
}

void dta_saver::write(pslice slice, unsigned index)
{
    record_.save_to_stream(stream_, slice, index);
}

}
