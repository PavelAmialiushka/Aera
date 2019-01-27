//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "utilites\unittest.h"

#include "utilites\Exception.h"
#include "utilites\log.h"
#include "utilites\counter.h"

class counter_test
    : public UnitTest::TestCase
{
    TestUnit_TEST_SUITE( counter_test );
    TestUnit_TEST( test );
    TestUnit_TEST_SUITE_END();
public:
    void test()
    {
        //COUNTER_STARTUP;
    }

};

TestUnit_TEST_SUITE_REGISTRATION( counter_test );
