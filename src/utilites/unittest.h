//////////////////////////////////////////////////////////////////////////
//
// utilites library
// 
// Written by Pavel Amialiushka 
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <math.h>
#include <string>
#include <list>
#include <iostream>
#include <exception>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/mem_fn.hpp>

#include "utilites\singleton.h"
#include "utilites\exception.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
namespace UnitTest {

//////////////////////////////////////////////////////////////////////////

int run();

//////////////////////////////////////////////////////////////////////////


template<class T>
struct test_ptr_t
{
  typedef boost::function1<void, T*> fun_t;
  
  test_ptr_t(fun_t f, std::string name, debug::SourcePos_tag loc) 
  : fun_ptr_(f), name_(name), location_(loc) 
  {
  }

  fun_t fun_ptr_;
  std::string name_;
  debug::SourcePos_tag location_;
};

template<class T>
class auto_register : public utils::singleton<auto_register<T> >
{
public: 
  typedef std::vector< test_ptr_t<T> > vector_t;
  boost::scoped_ptr<vector_t> pvector_;

  auto_register() 
  : pvector_(new vector_t)
  {
  }
  ~auto_register()
  {
  }

public:

  vector_t& get_vector()
  {
    return *pvector_;
  }
};



#define UNITTEST_SUITE(a) \
a; class a ## __helper : public UnitTest::TestCase {\
public:\
  typedef a self_tx; \
  virtual void perform_registration()\
  {\
    using namespace UnitTest;\
    unsigned size=auto_register<self_tx>::instance()->get_vector().size();\
    std::string suite_name=#a "::"; \
    for(unsigned index=0; index<size; ++index)\
    {\
      test_ptr_t<self_tx> &test=\
        auto_register<self_tx>::instance()->get_vector()[index];\
      tests_.push_back( \
        test_t( \
          boost::bind(\
            &test_ptr_t<self_tx>::fun_t::operator(),\
            &test.fun_ptr_, \
            reinterpret_cast<self_tx*>(this)), \
          suite_name + test.name_,\
          test.location_)\
        );\
    } \
  }\
}; class a : public a ## __helper

#define UNITTEST(a) public: \
  struct auto_ ## a ## _t \
  { \
    typedef self_tx self_t;\
    auto_ ## a ## _t () \
    { \
      UnitTest::test_ptr_t<self_t> test(boost::mem_fn(&self_t:: a), #a, HERE);\
      boost::function1<void, self_t*> fn=boost::mem_fn(&self_t :: a);\
      UnitTest::auto_register<self_t>::instance()->get_vector().push_back(test);\
    } \
  } auto ## a; \
  void a()

} // namespace

#define TestUnit_TEST_SUITE(a) \
private:  \
  typedef a test_suite_type_name;\
  virtual void perform_registration() {\
    std::string suite_name=#a "::";
#define TestUnit_TEST(b) \
    tests_.push_back( test_t( bind(&test_suite_type_name::b, this), suite_name + #b) );
#define TestUnit_TEST_EXCEPTION(b, e) 
#define TestUnit_TEST_SUITE_END() \
  }

#define JOIN0(a, b) a##b
#define JOINX(a,b) JOIN0(a, b)
#define UNIQUE(x) JOINX(x, __LINE__)
#define TestUnit_TEST_SUITE_REGISTRATION(x) x UNIQUE(x)
#define UNITTEST_SUITE_REGISTRATION(x) x UNIQUE(x)

#define INSTANCE UNIQUE(go)

namespace UnitTest
{

  class failure : public std::exception
  {
    std::string what_;
  public:
    failure(std::string w) 
    {
      what_=w;
    }

    failure::~failure() throw()
    {
    }

    const char* what() const throw()
    {
      return what_.c_str();
    }
  };
  class TestSuite;

  class TestFactoryRegistry {
  public:
   
    static TestFactoryRegistry& instance()
    {
      static TestFactoryRegistry inst;
      return inst;
    }

    void add_test(TestSuite* s);
    int run();

    bool accept_test_execution(std::string);

  private:
    std::vector<std::string> templates_;
    std::vector<TestSuite*> suites_;
    int total_;
    int failures_;
  };

  class Test 
  {
  public:
    static
    std::string registerFailure(std::string testname, std::string);

    static
    std::string registerFailure(std::string testname, debug::SourcePos lc, const failure& f);

    static
    std::string registerFailure(std::string testname, debug::SourcePos lc, const debug::aera_error& e);

    static
    std::string registerFailure(std::string name, debug::SourcePos lc, const std::exception& e);

    static
    std::string registerFailure(std::string testname, debug::SourcePos lc);

    // assertions

    static void
    failAt(std::string message, const char* loc);

    static void
    assertTrueMessageHere(bool v, std::string message, const char* loc);

    static void
    assertTrueAt(bool v, const char* loc);

    template<class T, class U> static void
    assertEqualAt(const T& a, const U& b, const char* loc) 
    {
      std::stringstream str;
      str << "value \'" << a << "\' not equal to\'" << b << "\'";
      assertTrueMessageHere(a==b, str.str(), loc);
    }

    static void 
    assertDoublesEqualAt(double a, double b, double delta, const char* loc);
  };

  // testSuite

  class TestSuite : public Test {
  public:
    typedef boost::function0<void> test_ptr;
    struct test_t {
      test_t(test_ptr ptr, std::string name, debug::SourcePos_tag lc=debug::SourcePos_tag()) : ptr(ptr), name(name), loc(lc) {}
      debug::SourcePos_tag loc;
      test_ptr      ptr;
      std::string   name;
    };

    TestSuite();
    virtual ~TestSuite();

    virtual void setUp();
    virtual void tearDown();

    virtual void run(int& total, int& failured, std::vector<std::string>& failed_tests);

  private:

    bool safe_run(test_t test);
    int run_test(test_t test);
    std::string failure_string;

  protected:

    virtual void perform_registration()=0;
    std::vector<test_t> tests_;
  };
  
  //////////////////////////////////////////////////////////////////////////
  
  typedef TestSuite TestCase;
  
  //////////////////////////////////////////////////////////////////////////
  
  template<class T> class AutoTestSuite : public TestSuite 
  {
  protected:    
    typedef T self_t;
    typedef typename AutoTestSuite<self_t> base_t;

    AutoTestSuite()
    {
      TestFactoryRegistry::instance().add_test(this);
    }

    static self_t this_is_registrator_mark;

    virtual void perform_registration() 
    {
      using namespace UnitTest;
      unsigned size=auto_register<base_t>::instance()->get_vector().size();
      std::string suite_name=std::string(typeid(self_t).name()) + "::"; 
      for(unsigned index=0; index<size; ++index)
      {
        test_ptr_t<base_t> &test=
          auto_register<base_t>::instance()->get_vector()[index];
        tests_.push_back(
          test_t(
            bind(
              &boost::function1<void, base_t*>::operator(),
              &test.fun_ptr_,
              this),
            suite_name + test.name_,
            test.location_)
          );
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////

  struct time_marker
  {
    mutable DWORD finish;
    DWORD start;
    time_marker()
    : start(GetTickCount())
    {
      finish=start;
    }

    double get_time() const 
    {
      if (finish==start) finish=GetTickCount();
      return (finish-start)/1000.0;
    }
  };


  #define assertDoublesEqual(e,a,d) UnitTest::Test::assertDoublesEqualAt(e, a, d, HERE)
  #define assertEqual(e, a)         UnitTest::Test::assertEqualAt(e, a, HERE)
  #define assertTrue(a)             UnitTest::Test::assertTrueAt(a, HERE)
  #define failtest(a)               UnitTest::Test::failAt(a, HERE)

  #define assertTrueMessage(e, m)   UnitTest::Test::assertTrueMessageHere(e, m, HERE)

  #define assertMessage(a)          do { std::string s = a; UnitTest::Test::assertTrueMessageHere(s.empty(), s, HERE); } while(0)

  #define assertBlock               { UnitTest::time_marker __marker; \
                                      bool success=false; \
                                      try {

  #define assertPerformance(d)        } catch(int) { throw; } \
                                        assertTrueMessage(__marker.get_time() < d, \
                                        strlib::strf("Low performance: %.3f sec expected, but was %.3f src", (double)d, __marker.get_time())); \
                                    }

  #define assertException(e)          } catch(e&) { \
                                        success=true; \
                                      } assertTrue(success); \
                                    }

  
} // TestUnit
