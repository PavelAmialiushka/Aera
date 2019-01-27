//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "utilites\sourcePos.h"

namespace debug
{

struct aera_error;

class warning
{
public:
    warning(const debug::aera_error &);
};

//////////////////////////////////////////////////////////////////////////


class message
{
public:
    message(const char *str, debug::SourcePos lt);

    template<class T> message(T &t, debug::SourcePos lt)
    {
        std::ostringstream ss;
        ss << t;
        message((const char*)ss.str().c_str(), lt);
    }

private:

    void report_string(char *start, char *end);
    const char *form_heading(debug::SourcePos lt);
    void form_message(const char *str, debug::SourcePos lt);
};

void log_enable_console();

}
/////////////////////////////////////////////////////////////////////////


enum {_LOG_TRACE=4, _LOG_INFO=2, _LOG_WARN=1, _LOG_NONE=0, };

#define aeraLOG(flag, mess) { debug::message(mess, HERE); }
#define aeraTRACE(mess)  aeraLOG(_LOG_TRACE, mess)
#define aeraINFO(mess)   aeraLOG(_LOG_INFO, mess)
#define aeraWARN(mess)   aeraLOG(_LOG_WARN, mess)

#define LOG(mess) { debug::message(mess, HERE); }

#define TraceError(mess) aeraLOG(0, mess))

#define TRACEF(msg) message(msg, debug::function())



