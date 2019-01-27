#include "stdafx.h"

#include "monpac_result.h"
#include "channel.h"

namespace monpac
{

Result::Result(std::vector<monpac::channel> const &result)
    : channels_(result)
{
}

unsigned Result::size() const
{
    return channels_.size();
}

channel const &Result::channel(unsigned index) const
{
    assert(index < channels_.size());
    return channels_[index];
}

}