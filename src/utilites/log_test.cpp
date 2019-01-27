//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "utilites\Exception.h"

#include "utilites\unittest.h"

#include "utilites\log.h"
#include "utilites\counter.h"

#include <boost/format.hpp>

class UNITTEST_SUITE( log_test )
{
public:
    UNITTEST( common )
    {
        mark m(HERE);
        debug::message("Hello", HERE);
        debug::message("first line\nsecond line [newline]\n", HERE);
        debug::message("first line\nsecond line [no newline]", HERE);
        debug::message("extremly long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long line ", HERE);
        debug::message("extremly long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long long line with newline\n", HERE);
    }

    UNITTEST( templ )
    {
        mark m(HERE);
        debug::message(boost::format("this is a formated s=\"%s\" x=%04x c=\'%c\'\n") % "message" % 1 % -1, HERE);
    }

    UNITTEST( tracef )
    {
        debug::TRACEF("hello");
    }

} INSTANCE;

