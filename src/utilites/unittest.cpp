#include "stdafx.h"

#include "unittest.h"
#include "CmdLine.h"
#include "foreach.hpp"

#include <stdlib.h>

#include <boost/format.hpp>

namespace UnitTest
{

static int ReportHook(int reportType, char *message, int *return_value)
{
    switch ( reportType )
    {
    case _CRT_ASSERT:
        if (IsDebuggerPresent()) _CrtDbgBreak();
        throw failure(message);

        return 1; // return value and retVal are not important since we never reach this line
    case _CRT_ERROR:
        if (IsDebuggerPresent()) _CrtDbgBreak();
        throw failure(message);

        return 1; // return value and retVal are not important since we never reach this line
    default:
        return 0; // use usual reporting method
    }

}

int TestFactoryRegistry::run()
{
    debug::structured_exception::install_unittest();

    ::SetPriorityClass(::GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    _CrtSetReportHook(ReportHook);
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_DELAY_FREE_MEM_DF);

    int repeat_count = 1;

    CmdLine line;
    for (unsigned index=0; index<line.option_size(); ++index)
    {
        if (line.option(index)=="test")
        {
            templates_.push_back(line.option_addenda(index));
        }

        if (line.option(index)=="repeat")
        {
            repeat_count = atoi(line.option_addenda(index).c_str());
            repeat_count = std::min(10000, std::max(1, repeat_count));
        }
    }


    //////////////////////////////////////////////////////////////////////////

    total_=0;
    failures_=0;

    std::vector<std::string> failed_strings;

    while(repeat_count --> 0)
    {
        for (unsigned index=0; index<suites_.size(); ++index)
        {
            try
            {
                suites_[index]->run(total_, failures_, failed_strings);
            }
            catch (...) {}
        }

        if (repeat_count)
        {
            std::string out = strlib::strf("---\nrepeat again (countdown: %d last)\n---\n", repeat_count);
            LOG(out);
            std::cout << out << std::flush;
        }
    }

    if (failed_strings.size())
    {
        std::string out = "\n----------------------------------------------------------------------------------------"
                "\nList of failed tests: \n";
        LOG(out); std::cout << out << std::flush;
    }
    for(unsigned index=0; index < failed_strings.size(); ++index)
    {
        std::string out = strlib::replace(failed_strings[index], ": error A0000", "");
        LOG(out); std::cout << out << std::endl;
    }

    std::string out=strlib::strf("\n----------------------------------------------------------------------------------------"
                                 "\n%d total, %d failured\n",
                                 total_,
                                 failures_);
    LOG(out);
    std::cout << out << std::flush;

    return failures_ ? 2 : 0;
}

void TestFactoryRegistry::add_test(TestSuite *s)
{
    suites_.push_back(s);
}

bool TestFactoryRegistry::accept_test_execution(std::string name)
{
    if (templates_.empty()) return true;
    foreach(std::string templ, templates_)
    {
        if (name.find(templ)!=std::string::npos)
            return true;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////

std::string Test::registerFailure(std::string testname, std::string contents)
{
    std::string report = strlib::strf("\n-- %s\n%s\n", testname.c_str(), contents.c_str());

    LOG(report);
    std::cout << report << std::flush;

    return report;
}


std::string Test::registerFailure(std::string testname, debug::SourcePos lc, const failure &f)
{
    return registerFailure(testname, (std::string)f.what());
}


std::string Test::registerFailure(std::string testname, debug::SourcePos lc, const debug::aera_error &e)
{
    return registerFailure(testname,
                    strlib::strf("%s:\n%s",
                                 lc.get_string(),
                                 e.get_string()));
}

std::string Test:: registerFailure(std::string name, debug::SourcePos lc, const std::exception &e)
{
    return registerFailure(name,
                    strlib::strf("%s: unexpected std::exception: \"%s\"\n",
                                 lc.get_string(),
                                 e.what()));
}


std::string Test::registerFailure(std::string testname, debug::SourcePos lc)
{
    return registerFailure(testname,
                    strlib::strf("%s: unexcpected exception.\n",
                                 lc.get_string()));
}

// assertions

void Test::
failAt(std::string message, const char *loc)
{
    throw failure(
        std::string(loc) + std::string(" : error A0000: Failure: ")+message+"\n");
}

void Test::
assertTrueMessageHere(bool v, std::string message, const char *loc)
{
    if (!v)
    {
        failAt(message, loc);
    }
}

void Test::
assertTrueAt(bool v, const char *loc)
{
    assertTrueMessageHere(v, "expression is not true", loc);
}


void Test::
assertDoublesEqualAt(double a, double b, double delta, const char *loc)
{
    assertTrueMessageHere(
              fabs(a-b)<delta && !_isnan(a) && !_isnan(b),
              (
                  boost::format("value '%.8f' not equal to '%.8f' (D=%.8f of %.8f)")
                  %a
                  %b
                  %fabs(b-a)
                  %delta
              ).str(), loc);
}

//////////////////////////////////////////////////////////////////////////

TestSuite::TestSuite()
{
    TestFactoryRegistry::instance().add_test(this);
}

TestSuite::~TestSuite()
{
}

bool TestSuite::safe_run(test_t test)
{
    try
    {
        test.ptr();
        return true;
    }
    catch (const UnitTest::failure &f)
    {
        failure_string += registerFailure(test.name, test.loc, f);
        return false;
    }
    catch (const debug::aera_error &e)
    {
        failure_string += registerFailure(test.name, test.loc, e);
        return false;
    }
    catch (const std::exception &e)
    {
        failure_string += registerFailure(test.name, test.loc, e);
        return false;
    }
    catch (...)
    {
        failure_string += registerFailure(test.name, (debug::SourcePos)test.loc);
        return false;
    }
}

int TestSuite::run_test(test_t test)
{
    int succ=0;
    failure_string = "";
    if (safe_run(test_t( bind(&TestSuite::setUp, this) , test.name+"(setUp)")))
    {
        succ+=safe_run(test);
        succ+=safe_run(test_t( bind(&TestSuite::tearDown, this), test.name+"(tearDown)"));
    }
    if (_CrtCheckMemory() == 0)
    {
        failure_string = test.name + "(_CrtCheckMemory)";
        return 0;
    }
    return succ==2 ? 1 : 0;
}

void TestSuite::setUp() {}
void TestSuite::tearDown() {}


static char print_out_buffer[1024];
static void __cdecl at_axit_print_out_buffer(void)
{
    puts(print_out_buffer);
}

static void register_at_exit(std::string str)
{
    if (str.size()) str+=" - failured.\n";
    strncpy(print_out_buffer, str.c_str(), sizeof(print_out_buffer));
    print_out_buffer[std::min(sizeof(print_out_buffer), str.size())]=0;

    static int registered=0;
    if (!registered)
    {
        registered=1;
        atexit(at_axit_print_out_buffer);
    }
}

void TestSuite::run(int &total, int &failured, std::vector<std::string>& failed_tests)
{
    tests_.clear();
    perform_registration();
    for (std::vector<test_t>::iterator index=
                tests_.begin(); index!=tests_.end(); ++index)
    {
        if ( !TestFactoryRegistry::instance().accept_test_execution(index->name))
            continue;

        ++total;
        register_at_exit(index->name);
        {
            std::string report=index->name + " starting test...\n";
            LOG(report);
            std::cout << report << std::flush;
        }

        if (run_test(*index))
        {
            std::string report=index->name + " - success.\n";
            LOG(report);
            std::cout << report << std::flush;
        }
        else
        {
            failed_tests.push_back( failure_string );
            ++failured;
        }
        register_at_exit("");
    }
}

//////////////////////////////////////////////////////////////////////////

int run()
{
    return TestFactoryRegistry::instance().run();
}

}
