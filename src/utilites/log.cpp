//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites\application.h"

#include "utilites\log.h"
#include "utilites\singleton.h"
#include "utilites\exception.h"
#include "utilites\utilites.h"

#include <boost/format.hpp>

#define _NO_CONSOLE_OUTPUT

#ifdef NDEBUG
// FINAL
#define _NO_FILE_OUTPUT
#define _NO_DEBUG_OUTPUT
#endif

namespace debug
{

warning::warning(const debug::aera_error &e)
{
    std::string str=(
                        boost::format("Exception was raised:\n%s\nApplication will be closed")
                        %e
                    ).str();
    LOG(strlib::strf("%s %s", e.get_location().get_string(), str.c_str()));
    _RPT2(_CRT_ERROR, "%s %s", e.get_location().get_string(), str.c_str());
    if (IsDebuggerPresent())
    {
        _CrtDbgBreak();
    }
    else
    {
        exit(0);
    };
}


class initializer
    : public utils::singleton<initializer, 16>
{
protected:
    initializer()
        : hconsole_(0),
          logfile_(0)
    {
#ifndef _NO_FILE_OUTPUT
        init_logfile();
#endif

#ifndef _NO_CONSOLE_OUTPUT
        init_console();
#endif

    }

    ~initializer()
    {
        close_logfile();
        close_console();
    }

    friend class utils::singleton<initializer, 16>;

public:

    void init_console()
    {
        ::AllocConsole();
        ::SetConsoleTitle("Aera log");

        hconsole_=::GetStdHandle(STD_ERROR_HANDLE);

        COORD crds= {120, 500};
        ::SetConsoleScreenBufferSize(hconsole_, crds);
    }

    void write(char *start, char *end)
    {
        DWORD written=0;

        if (hconsole_)
        {
            ::CharToOemBuff(start, start, end-start);
            ::WriteFile(hconsole_, start, end-start, &written, NULL);
        }

        if (logfile_)
        {
            ::WriteFile(logfile_, start, end-start, &written, NULL);
        }

        char buffer[256];
        strncpy(buffer, start, end-start);
        buffer[ std::min((int)(end-start), (int)SIZEOF(buffer)-1) ]=0;
        OutputDebugString(buffer);
    }

    void init_logfile()
    {
        logfile_=::CreateFile(
                     (utils::get_app_path("AeraTools/aera.log")).c_str(),
                     GENERIC_WRITE,
                     FILE_SHARE_READ,
                     NULL,
                     CREATE_ALWAYS,
                     FILE_ATTRIBUTE_NORMAL
                    , NULL);

        SetFilePointer(logfile_, 0, 0, FILE_END);
    }

    void close_logfile()
    {
        if (logfile_)
            ::CloseHandle(logfile_);
    }

    void close_console()
    {
        if (hconsole_)
        {
            ::CloseHandle(hconsole_);
            hconsole_=0;
        }
    }

private:
    HANDLE      hconsole_;
    HANDLE      logfile_;
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

inline
void message::report_string(char *start, char *end)
{
    if (std::find(start, end, '\n')==end) *end++='\n';

    initializer::instance()->write(start, end);
}

inline
const char *message::form_heading(debug::SourcePos lt)
{
    static char buf[200];

    SYSTEMTIME st;
    ::GetLocalTime(&st);
    sprintf(buf, "%02d:%02d:%02d.%04d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    const char *index=strrchr(lt.get_string(), '\\');
    if (!index) strcat(buf, lt.get_string());
    else        strcat(buf, index+1);
    strcat(buf, ": ");
    return buf;
}

void message::form_message(const char *str, debug::SourcePos lt)
{
    const char *heading=form_heading(lt);
    int heading_size=strlen(heading);

    const int line_size = 119;

    const char *start=str;
    const char *end=strchr(start, 0);
    const char *endofportion=start;
    while (start<end)
    {
        endofportion=strchr(start, '\n');
        endofportion=endofportion ? endofportion+1 : end;
        if (endofportion-start >= line_size-heading_size )
        {
            endofportion=start+line_size-heading_size;
            while (strchr(". ,\t:+-*\\/", *--endofportion)==0)
                ;
            ++endofportion;
        }

        char buf[2000];
        strcpy(buf, heading);
        std::size_t freespace=SIZEOF(buf) - strlen(buf);
        endofportion =std::min(endofportion, start+freespace);

        for (char *out=strchr(buf, 0); start<endofportion || (*out=0, 0); *out++=*start++)
            ;

        report_string(buf, strchr(buf, 0));
    }
}

void log_enable_console()
{
    initializer::instance()->init_console();
}

message::message(const char *str, debug::SourcePos lt)
{
    form_message(str, lt);
}


}
