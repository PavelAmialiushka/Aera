//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites\unittest.h"

class UNITTEST_SUITE(test_test)
{
public:

    UNITTEST(assert_failed)
    {
        assertEqual(0, 1);
    }

    UNITTEST(std_exception)
    {
        throw std::logic_error("logic_error");
    }

    UNITTEST(aera_exception)
    {
        try
        {
            debug::Assert<>(0, "testing aera exception", HERE);
        } LogException("logging exception path");
    }

    UNITTEST(unknown_exception)
    {
        throw 0;
    }
};


