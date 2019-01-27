//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "utilites/sourcePos.h"

#define COUNTER_STARTUP mark m(HERE);

class mark
{
public:

    mark(debug::SourcePos lt);
    ~mark();

private:

    debug::SourcePos tag_;
    __int64 time_;
};