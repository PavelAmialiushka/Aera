//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites\Exception.h"

#include "eh.h" // SEH

#include "ExceptionHandler.h"
#include "security.h"

namespace debug
{

static int unittest_marker=0;
static int developer_machine=0;

static void my_translator(unsigned, EXCEPTION_POINTERS *info)
{
    debug::structured_exception error(*info);
    LOG(error.what());

    if (unittest_marker) throw error;

    if (developer_machine)
    {
        DebugBreak();
        return;
    }

    // чтобы другой поток не натворил делов
    SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL );

    LONG retcode=RecordExceptionInfo(info, "Aera");


    // уничтожение глобальных переменных
    _c_exit();

    // выход из программы
    ::ExitProcess(-1);
}

static LONG WINAPI UnhandledExceptionFilter(LPEXCEPTION_POINTERS /*ExceptionInfo*/)
{
    return EXCEPTION_CONTINUE_SEARCH;
}

namespace
{
struct just_an_installer
{
    just_an_installer()
    {
        //structured_exception::install();
    }
} INSTANCE;
}

structured_exception::structured_exception(EXCEPTION_POINTERS const &info) throw()
{
    EXCEPTION_RECORD const &exception = *(info.ExceptionRecord);
    address_ = exception.ExceptionAddress;
    code_ = exception.ExceptionCode;
}

void structured_exception::install() throw()
{
    if (security::manager().get_registration_info()=="developer")
        developer_machine=true;

    SetUnhandledExceptionFilter(UnhandledExceptionFilter);
    _set_se_translator(my_translator);
}

void structured_exception::install_unittest() throw()
{
    unittest_marker=1;
}

unsigned structured_exception::code() const throw()
{
    return code_;
}

void const *structured_exception::where() const throw()
{
    return address_;
}

const char *structured_exception::what() const
{
    char *tmp;
    switch (code())
    {
    case EXCEPTION_ACCESS_VIOLATION:
        tmp="EXCEPTION_ACCESS_VIOLATION";   break;
    case EXCEPTION_DATATYPE_MISALIGNMENT:
        tmp="EXCEPTION_DATATYPE_MISALIGNMENT";    break;
    case EXCEPTION_BREAKPOINT:
        tmp="EXCEPTION_BREAKPOINT";   break;
    case EXCEPTION_SINGLE_STEP:
        tmp="EXCEPTION_SINGLE_STEP";    break;
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        tmp="EXCEPTION_ARRAY_BOUNDS_EXCEEDED";    break;
    case EXCEPTION_FLT_DENORMAL_OPERAND:
        tmp="EXCEPTION_FLT_DENORMAL_OPERAND";   break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        tmp="EXCEPTION_FLT_DIVIDE_BY_ZERO";   break;
    case EXCEPTION_FLT_INEXACT_RESULT:
        tmp="EXCEPTION_FLT_INEXACT_RESULT";   break;
    case EXCEPTION_FLT_INVALID_OPERATION:
        tmp="EXCEPTION_FLT_INVALID_OPERATION";  break;
    case EXCEPTION_FLT_OVERFLOW:
        tmp="EXCEPTION_FLT_OVERFLOW";   break;
    case EXCEPTION_FLT_STACK_CHECK:
        tmp="EXCEPTION_FLT_STACK_CHECK";    break;
    case EXCEPTION_FLT_UNDERFLOW:
        tmp="EXCEPTION_FLT_UNDERFLOW"; break;
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
        tmp="EXCEPTION_INT_DIVIDE_BY_ZERO"; break;
    case EXCEPTION_INT_OVERFLOW:
        tmp="EXCEPTION_INT_OVERFLOW"; break;
    case EXCEPTION_PRIV_INSTRUCTION:
        tmp="EXCEPTION_PRIV_INSTRUCTION"; break;
    case EXCEPTION_IN_PAGE_ERROR:
        tmp="EXCEPTION_IN_PAGE_ERROR"; break;
    case EXCEPTION_ILLEGAL_INSTRUCTION:
        tmp="EXCEPTION_ILLEGAL_INSTRUCTION";  break;
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
        tmp="EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
    case EXCEPTION_STACK_OVERFLOW:
        tmp="EXCEPTION_STACK_OVERFLOW"; break;
    case EXCEPTION_INVALID_DISPOSITION:
        tmp="EXCEPTION_INVALID_DISPOSITION"; break;
    case EXCEPTION_GUARD_PAGE:
        tmp="EXCEPTION_GUARD_PAGE"; break;
    case EXCEPTION_INVALID_HANDLE:
        tmp="EXCEPTION_INVALID_HANDLE"; break;
    }
    static char buf [150];
    sprintf(buf, "%s, at adress=0x%08x\n", tmp, where());
    return buf;
}


//////////////////////////////////////////////////////////////////////////

error_location::error_location(const debug::SourcePos &loc, const char *desc)
    : location_(loc),
      desc_(desc)
{
}

const debug::SourcePos &error_location::get_location() const throw()
{
    return location_;
}

const char *error_location::get_description() const throw()
{
    return desc_.c_str();
}

//////////////////////////////////////////////////////////////////////////

aera_error::aera_error(const char *what, const debug::SourcePos &loc)
    : string_(what), location_(loc)
{
}

void aera_error::trace(const debug::SourcePos &loc, const char *desc)
{
    loc_list_.push_back(error_location(loc, desc));
}

aera_error::~aera_error() throw()
{
}

const char *aera_error::what() const throw()
{
    return get_string();
}

const debug::SourcePos &aera_error::get_location() const throw()
{
    return location_;
}

const char *aera_error::get_string() const throw()
{
    return string_.c_str();
}

const std::list<error_location> &aera_error::get_loc_list() const
{
    return loc_list_;
}

//

std::ostream &operator<<(std::ostream &s, const error_location &self)
{
    return s
           << strip(self.location_.get_string())
           << ':'
           << self.desc_;
}

std::ostream &operator<<(std::ostream &out, const aera_error &self)
{

    out << strip(self.get_location().get_string()) << ": \"" << typeid(self).name() << "\" exception\n"
        << strip(self.get_location().get_string()) << ": " << self.get_string()  << "\n";

    std::copy(
        self.get_loc_list().begin(), self.get_loc_list().end(),
        std::ostream_iterator<error_location>(out, "\n"));

    return out;
}



}
