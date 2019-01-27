//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include "utilites\SourcePos.h"
#include "utilites\singleton.h"
#include "utilites\application.h"

#include <boost/lexical_cast.hpp>

//#include <imagehlp.h>
//#pragma comment(lib, "imagehlp.lib")

namespace debug
{


//////////////////////////////////////////////////////////////////////////

typedef unsigned long DWORD;

//////////////////////////////////////////////////////////////////////////

static const char *strip(const char *ch)
{
    if (const char *begin=strrchr(ch, '\\'))
    {
        return begin+1;
    }
    return ch;
}

SourcePos::SourcePos()
    : loc_("unknown(0)")
{
}

SourcePos::SourcePos(const SourcePos &loc)
    : loc_(loc.get_string())
{
}

bool SourcePos::operator<(const SourcePos &loc) const
{
    return strcmp(loc_, loc.loc_) < 0;
}

bool SourcePos::operator==(const SourcePos &loc) const
{
    return strcmp(loc_, loc.loc_) == 0;
}

bool SourcePos::operator!=(const SourcePos &loc) const
{
    return strcmp(loc_, loc.loc_) != 0;
}

std::ostream &operator<<(std::ostream &stream, const SourcePos &loc)
{
    return stream << loc.loc_;
}

SourcePos::SourcePos(const char *loc)
{
    loc_=loc;
}

SourcePos &SourcePos::operator =(const char *loc)
{
    loc_=loc;
    return *this;
}

const char *SourcePos::get_string() const
{
    return loc_ ? loc_ : "unknown(0)";
}


//////////////////////////////////////////////////////////////////////////

namespace sym
{
/*
  struct initializer
  : utils::singleton<initializer>
  {
    initializer () {
      SymInitialize(
        GetCurrentProcess(),
        (char*)utils::get_exefile_path().native_file_string().c_str(),
        TRUE);

      SymSetOptions(
        SYMOPT_UNDNAME |
        SYMOPT_DEFERRED_LOADS |
        SYMOPT_LOAD_LINES);
    }

    ~initializer () {
      ::SymCleanup(GetCurrentProcess());
    }
  };

  DWORD FindAddr(int back=0);
  DWORD MyStackWalk(DWORD address, int back=0);

  std::string GetFunction(DWORD);
  std::string GetFileLine(DWORD, int&);

  //////////////////////////////////////////////////////////////////////////
  // lo-level routines

  #pragma warning( push )
  #pragma warning(disable : 4035)

  __declspec(naked) DWORD GetIP()
  {
      __asm mov   eax, [esp]
      __asm ret
  }

  #pragma warning( pop )

  DWORD MyStackWalk(DWORD address, int back)
  {
    BOOL res = FALSE;

    CONTEXT context = { 0 };
    context.ContextFlags = CONTEXT_FULL;
    res = GetThreadContext(GetCurrentThread(), &context);
    if (!res)
        return 0;

    STACKFRAME frame = { 0 };
    frame.AddrPC.Offset = address;
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrStack.Offset = context.Esp;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = context.Ebp;
    frame.AddrFrame.Mode = AddrModeFlat;

    while (back--) {
      res = StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), NULL, &frame, &context,
            NULL, NULL, NULL, NULL); //skip FindAddr function
      if (!res) return 0;
    }

    res = StackWalk(IMAGE_FILE_MACHINE_I386, GetCurrentProcess(), NULL, &frame, &context,
            NULL, NULL, NULL, NULL);

    if (!res) return 0;

    return frame.AddrPC.Offset;
  }


  DWORD FindAddr(int back)
  {
    DWORD ip = GetIP();
    return MyStackWalk(ip, back+2);
  }

  std::string GetFunction(DWORD addr)
  {
    DWORD displacement=0;
    char buffer[2048]={0};
    PIMAGEHLP_SYMBOL symbol=reinterpret_cast<PIMAGEHLP_SYMBOL>(buffer);
    symbol->SizeOfStruct=sizeof(IMAGEHLP_SYMBOL);
    symbol->MaxNameLength=sizeof(buffer)-sizeof(IMAGEHLP_SYMBOL) + 1;

    if (SymGetSymFromAddr(
          GetCurrentProcess(),
          addr,
          &displacement,
          symbol))
    {
      char out[1024]="unknown";
      UnDecorateSymbolName(
        symbol->Name,
        out,
        sizeof out,
        UNDNAME_COMPLETE|UNDNAME_NO_THISTYPE|UNDNAME_NO_SPECIAL_SYMS|UNDNAME_NO_ACCESS_SPECIFIERS|UNDNAME_NAME_ONLY);
      return out;
    }
    return "=error=";
  }

  std::string GetFileLine(DWORD addr)
  {
    DWORD dwDisplacement=0;
    IMAGEHLP_LINE line={sizeof(IMAGEHLP_LINE)};

    SymSetOptions(SYMOPT_LOAD_LINES);

    if (::SymGetLineFromAddr(::GetCurrentProcess(), addr, &dwDisplacement, &line)) {
      return std::string(line.FileName)
        +"("+
          boost::lexical_cast<std::string>(line.LineNumber)
        +")";
    } else {
      return "unknown(0)";
    }
  }
*/
} // namespace sym

//////////////////////////////////////////////////////////////////////////

function::function()
{
    using namespace sym;
    /*initializer::instance();

    function_=GetFileLine(FindAddr(1))+"@"+GetFunction( FindAddr(1) );
    loc_=function_.c_str();*/
}

}
