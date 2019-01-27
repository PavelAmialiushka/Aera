#pragma once

#include "traits.h"

struct parametric;
class stage;

namespace monpac
{
class channel;
class criterion;
}

namespace nodes
{
MAKE_SHARED(node);
}

namespace data
{
typedef boost::array<bool, 256> channel_map;
typedef shared_ptr<channel_map> pchannel_map;
}
