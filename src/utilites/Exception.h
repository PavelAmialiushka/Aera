//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <exception>

#include "sourcePos.h"
#include "log.h"
#include "strings.h"

namespace debug
{

#define LogExceptionPath(expr) \
  catch(debug::aera_error& e) { \
    e.trace(HERE, expr); throw; \
  } catch(debug::structured_exception& er) {\
    if (IsDebuggerPresent()) {\
      const char *e=er.what();\
      OutputDebugString(HERE);\
      OutputDebugString(er.what());\
      _CrtDbgBreak(); \
    }\
    throw;\
  } catch(std::exception& e) { \
    if (IsDebuggerPresent()) _CrtDbgBreak(); \
    debug::aera_error err(e.what(), HERE);\
    err.trace(HERE, expr); throw err; \
  } catch(...) { \
    if (IsDebuggerPresent()) _CrtDbgBreak(); \
    debug::aera_error err("caught unknown exception", HERE); \
    err.trace(HERE, expr); throw err; \
  }

#define LogException LogExceptionPath

#define StopAndLog  \
  catch(debug::aera_error& e) { \
    LOG(strlib::strf("%s: %s", e.get_location().get_string(), e.what()));\
    if (IsDebuggerPresent()) _CrtDbgBreak(); \
    exit(-1); \
  } catch(debug::structured_exception& er) { \
    LOG(er.what());\
    if (IsDebuggerPresent()) _CrtDbgBreak(); \
    exit(-1); \
  } catch(std::exception& e) { \
    LOG(e.what());\
    if (IsDebuggerPresent()) _CrtDbgBreak(); \
    exit(-1); \
  } catch(...) { \
    LOG("unknown exception");\
    if (IsDebuggerPresent()) _CrtDbgBreak(); \
    exit(-1); \
  }

#define IgnoreException(ex) catch(ex) {}


class error_location
{
public:

    error_location(const debug::SourcePos &loc, const char *desc);

public:

    const debug::SourcePos &get_location() const throw();
    const char *get_description() const throw();

    friend std::ostream &operator<<(std::ostream &s, const error_location &self);

private:
    debug::SourcePos location_;
    std::string     desc_;
};

std::ostream &operator<<(std::ostream &s, const error_location &self);

//////////////////////////////////////////////////////////////////////////

struct aera_error : public std::exception
{
public:
    aera_error(const char *what, const debug::SourcePos &loc);
    virtual ~aera_error() throw();

    virtual const char *what() const throw();

    const debug::SourcePos &get_location() const throw();
    const char *get_string() const  throw();
    const std::list<error_location> &get_loc_list() const;

    void trace(const debug::SourcePos &loc, const char *desc);

    virtual bool stop_debugger() const throw()
    {
        return true;
    }

protected:
    std::string string_;
    debug::SourcePos location_;
    std::list<error_location> loc_list_;
};

std::ostream &operator<<(std::ostream &out, const aera_error &error);

//////////////////////////////////////////////////////////////////////////

#define DEFINE_ERROR_OF_TYPE(base, type, message)\
struct type : base {\
  type(const debug::SourcePos& loc) : base(message, loc) {}\
  type(const char* ms, const debug::SourcePos& loc=HERE) : base(ms, loc) {}\
  virtual bool stop_debugger() const throw() { return true; } \
};

#define DEFINE_NERROR_OF_TYPE(base, type, message) \
struct type : base { \
  type(const debug::SourcePos& loc) : base(message, loc) {} \
  type(const char* ms, const debug::SourcePos& loc=HERE) : base(ms, loc) {}\
  virtual bool stop_debugger() const throw() { return false; } \
};


#define DEFINE_ERROR(type, message)  DEFINE_ERROR_OF_TYPE(debug::aera_error, type, message)
#define DEFINE_NERROR(type, message) DEFINE_NERROR_OF_TYPE(debug::aera_error, type, message)

DEFINE_ERROR(deadlock, "Potential deadlock. Counter reached zero.");
DEFINE_ERROR(fault, "Unexpected behaviour.");
DEFINE_ERROR(overflow, "Overflow.");
DEFINE_ERROR(underflow, "underflow.");


//////////////////////////////////////////////////////////////////////////

static const char *strip(const char *string)
{
    if (const char *ptr=strrchr(string, '\\'))
    {
        return ++ptr;
    }
    return string;
}

//////////////////////////////////////////////////////////////////////////

template<class E=aera_error>
class Assert
{
public:
    Assert(bool expr)
    {
        if (!expr)
        {
            throw E(debug::SourcePos());
        }
    }

    Assert(bool expr, debug::SourcePos loc)
    {
        if (!expr)
        {
            E ex(loc);
            const char *fmt="\nRaise '%s' exception: %s\n%s";
            if (ex.stop_debugger())
            {
                aeraLOG(0, strlib::strf(fmt, typeid(E).name(), ex.what(), strip(loc.get_string())));
                if (IsDebuggerPresent()) _CrtDbgBreak();
            }
            throw ex;
        }
    }

    Assert(bool b, const char *string, debug::SourcePos loc)
    {
        if (!b)
        {
            debug::aera_error e(string, loc);
            aeraLOG(0, e);
            if (IsDebuggerPresent()) _CrtDbgBreak();
            throw e;
        }
    }

    operator bool()
    {
        return true;
    }
};

//////////////////////////////////////////////////////////////////////////

class structured_exception : public std::exception
{
public:
    structured_exception(EXCEPTION_POINTERS const &) throw();

    static void install() throw();
    static void install_unittest() throw();

    unsigned code() const throw();
    void const *where() const throw();
    virtual const char *what() const;
private:
    void const *address_;
    unsigned code_;
};

} // namespace debug

using debug::fault;
using debug::overflow;
using debug::underflow;

