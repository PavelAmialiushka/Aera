#include "stdafx.h"

#include "slice.h"

#include "complex_slice.h"

namespace data
{

pslice slice::merge(pslice b) const
{
    return pslice( new complex_slice(clone(), b));
}

pslice slice::clone_indexed(const std::vector<unsigned> &indexes) const
{
    return pslice( new complex_slice(clone(), indexes));
}

pslice slice::merge(const std::vector<pslice> &v)
{
    return pslice( new complex_slice(v));
}
}
