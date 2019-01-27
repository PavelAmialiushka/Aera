//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#undef _DEBUG_
#define _DEBUG_ 0

#include "utilites\Exception.h"

#include "utilites\unittest.h"

DEFINE_ERROR(test_exception, "Test exception");

///class exception_test : public UnitTest::TestCase {
class UNITTEST_SUITE( exception_test )
{
public:
    UNITTEST( aera_exception_test )
    {
        try
        {
            const char *string="test: !@#$%^&*()";
            debug::SourcePos loc=HERE; debug::aera_error err(string, HERE);
            assertEqual(loc, err.get_location());
            assertEqual(string, err.get_string());
            debug::SourcePos throw_location;
            const char *exception_string="test-exception";
            try
            {
                try
                {
                    throw_location=HERE; debug::Assert<>(0, exception_string, HERE);
                }
                catch (std::exception &e)
                {
                    assertEqual(std::string(exception_string), std::string(e.what()));
                    throw;
                }
            }
            catch (debug::aera_error &e)
            {
                assertEqual(std::string(exception_string), std::string(e.what()));
                assertEqual(std::string(exception_string), std::string(e.get_string()));
                assertEqual(throw_location, e.get_location());
            }
            try
            {
                debug::Assert<>(rand()==rand(), "test error", HERE);
                //debug::Assert<>("test error", HERE);
                debug::Assert<test_exception>(rand()==rand(), HERE);
                //debug::Assert<test_exception>(HERE);
            }
            catch (...) {}
        }
        catch (...) {}
    }

//  UNITTEST( translation )
//  {
//    assertBlock
//    {
//
//      try {
//        try {
//          std::vector<char> a(10);
//          a.at(20)=0;
//        } LogExceptionPath("inner level");
//      } LogExceptionPath("outer level");
//
//    } assertException(debug::aera_error);
//  }

    static void damn()
    {
        memset(0, 1, 1);
    }

//  UNITTEST( seh_test )
//  {
//    debug::structured_exception::install();
//
//    assertBlock
//    {
//      try {
//        damn();
//      } LogExceptionPath("log");
//    } assertException(debug::structured_exception)
//  }
} go ;

