//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#ifdef UNICODE
namespace std
{
typedef wstring   tstring;

typedef wiostream tiostream;
typedef wostream  tostream;
typedef wistream  tistream;

typedef wifstream tifstream;
typedef wofstream tofstream;
typedef wfstream  tfstream;
}
typedef boost::wformat tformat;

#else

namespace std
{
typedef string   tstring;

typedef iostream tiostream;
typedef ostream  tostream;
typedef istream  tistream;

typedef ifstream tifstream;
typedef ofstream tofstream;
typedef fstream  tfstream;
}
typedef boost::format tformat;
#endif


namespace strlib
{

inline std::tstring replace(std::tstring text, std::tstring s, std::tstring d)
{
    for (size_t index=0; index=text.find(s, index), index!=std::tstring::npos;)
    {
        if (!d.empty())
            text.replace(index, s.length(), d);
        else
            text.erase(index, s.length());

        index+=d.length();
    }
    return text;
}

inline std::tstring trim_left(std::tstring line, std::tstring sep=_T(" "))
{
    unsigned k=line.find_first_not_of(sep);
    if (k<line.size() && k!=0)
    {
        line.erase(0, k);
    }
    return line;
}

inline std::tstring trim_right(std::tstring line, std::tstring sep=_T(" "))
{
    unsigned k=line.find_last_not_of(sep);
    if (k!=line.size()-1)
    {
        int sz=line.size();
        line.erase(1+k, std::tstring::npos);
    }
    return line;
}

inline std::tstring trim_both(std::tstring line, std::tstring sep=_T(" "))
{
    line = trim_left(line, sep);
    line = trim_right(line, sep);
    return line;
}

inline bool startswith(std::tstring const &string, std::tstring const &start)
{
    return
        string.size() >= start.size() &&
        string.substr(0, start.size()) == start;
}

inline bool endswith(std::tstring const &string, std::tstring const &end)
{
    return
        string.size()>=end.size() &&
        string.substr(string.size()-end.size())==end;
}

inline std::tstring to_lower(std::tstring string)
{
    std::transform(string.begin(), string.end(), string.begin(), tolower);
    return string;
}

inline std::tstring to_upper(std::tstring string)
{
    std::transform(string.begin(), string.end(), string.begin(), ::toupper);
    return string;
}

template<class TC>
inline std::tstring strf(const TC *format)
{
    return std::tstring(format);
}

template<class TC, class T1>
std::tstring strf(const TC *format, T1 t1)
{
    return (boost::basic_format<TC>(format) % t1).str();
}

template<class TC, class T1, class T2>
std::tstring strf(const TC *format, T1 t1, T2 t2)
{
    return (boost::basic_format<TC>(format) % t1 % t2).str();
}

template<class TC, class T1, class T2, class T3>
std::tstring strf(const TC *format, T1 t1, T2 t2, T3 t3)
{
    return (boost::basic_format<TC>(format) % t1 % t2 % t3).str();
}

template<class TC, class T1,class T2, class T3, class T4>
std::tstring strf(const TC *format, T1 t1, T2 t2, T3 t3, T4 t4)
{
    return (boost::basic_format<TC>(format) % t1 % t2 % t3 % t4).str();
}

template<class TC, class T1,class T2, class T3, class T4, class T5>
std::tstring strf(const TC *format, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
{
    return (boost::basic_format<TC>(format) % t1 % t2 % t3 % t4 % t5).str();
}

template<class TC, class T1,class T2, class T3, class T4, class T5, class T6>
std::tstring strf(const TC*format, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
{
    return (boost::basic_format<TC>(format) % t1 % t2 % t3 % t4 % t5 % t6).str();
}

template<class TC, class T1,class T2, class T3, class T4, class T5, class T6, class T7>
std::tstring strf(const TC*format, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
{
    return (boost::basic_format<TC>(format) % t1 % t2 % t3 % t4 % t5 % t6 % t7).str();
}

template<class TC, class T1,class T2, class T3, class T4, class T5, class T6, class T7, class T8>
std::tstring strf(const TC*format, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
{
    return (boost::basic_format<TC>(format) % t1 % t2 % t3 % t4 % t5 % t6 % t7 % t8).str();
}

static std::string strff(const char *format, double f)
{
    std::string r = strlib::strf(format, f);
    if (r.find(".") != r.npos)
    {
        int index=r.size()-1;
        while(index --> 0 && r[index]=='0')
            r.erase(index);

        if (r[index]=='.')
            r.erase(index);
    }

    return r;
}

static std::string strff(double f)
{
    return strff("%f", f);
}

//inline std::string strf(const char* format, ...)
//{
//  va_list arglist;
//  char buffer[2048];
//
//  va_start(arglist, format);
//
//  _vsnprintf(buffer, 2048, format, arglist);
//  return buffer;
//}

inline std::tstring split(std::tstring &string, std::tstring token)
{
    size_t index=string.find(token);
    if (index!=std::tstring::npos)
    {
        std::tstring result=string.substr(0, index);
        string.erase(0, index+token.length());
        return result;
    }
    else
    {
        std::tstring result=string;
        string=std::tstring();
        return result;
    }
}

inline std::tstring rsplit(std::tstring &string, std::tstring token)
{
    size_t index=string.rfind(token);
    if (index!=std::tstring::npos)
    {
        std::tstring result=string.substr(0, index);
        string.erase(0, index+token.length());
        return result;
    }
    else
    {
        std::tstring result=string;
        string=std::tstring();
        return result;
    }
}

inline std::tstring head(std::tstring const &string, std::tstring token)
{
    size_t index=string.find(token);
    if (index!=std::tstring::npos)
    {
        return string.substr(0, index);
    }
    else
    {
        return string;
    }
}

inline std::tstring tail(std::tstring string, std::tstring token)
{
    size_t index=string.find(token);
    if (index!=std::tstring::npos)
    {
        string.erase(0, index+token.length());
        return string;
    }
    else
    {
        return std::tstring();
    }
}

inline std::tstring rhead(std::tstring string, std::tstring token)
{
    return rsplit(string, token);
}

inline std::tstring rtail(std::tstring string, std::tstring token)
{
    rsplit(string, token);
    return string;
}

//////////////////////////////////////////////////////////////////////////

inline std::string encode(std::wstring const &unicode, int type)
{
    std::vector<char> buffer(unicode.size()*2);
    unsigned len=::WideCharToMultiByte(
                     type, 0,
                     unicode.c_str(), unicode.size(),
                     &buffer[0], buffer.size(),
                     0, 0);
    return std::string(&buffer[0], len);
}

inline std::string utf8_encode(std::wstring const &unicode)
{
    return encode(unicode, CP_UTF8);
}

inline std::string acp_encode(std::wstring const &unicode)
{
    return encode(unicode, CP_UTF8);
}

inline std::wstring decode(std::string const &string, int type)
{
    std::vector<wchar_t> buffer(string.size()*2);
    int len = ::MultiByteToWideChar(
                  type, 0,
                  string.c_str(), string.size(),
                  &buffer[0], buffer.size());
    return std::wstring(&buffer[0], len);
}

inline std::wstring utf8_decode(std::string const &string)
{
    return decode(string, CP_UTF8);
}

inline std::wstring acp_decode(std::string const &string)
{
    return decode(string, CP_ACP);
}

inline std::string narrow( std::wstring const &source )
{
    return acp_encode( source );
}

inline std::string narrow( std::string const &source)
{
    return source;
}

inline std::wstring widen( std::wstring const &source )
{
    return source;
}

inline std::wstring widen( std::string const &source)
{
    return acp_decode(source);
}

//////////////////////////////////////////////////////////////////////////

class c_str
{
public:
    const char *operator <<= (std::string const &str) const
    {
        result = str;
        return result.c_str();
    }

    const char *operator <<= (char const *str) const
    {
        result = str;
        return result.c_str();
    }

    const char *operator <<= (boost::format const &fmt) const
    {
        result = boost::str(fmt);
        return result.c_str();
    }
private:
    mutable std::string result;
};

template<class T, class U>
T read(U str, bool* ok)
{
    try
    {
        if (ok) *ok = true;
        return boost::lexical_cast<T>(str);
    }
    catch(boost::bad_lexical_cast&)
    {
        if (ok) *ok = false;
    }
}

template<class T, class U>
T read(U str, T def)
{
    try
    {
        return boost::lexical_cast<T>(str);
    }
    catch(boost::bad_lexical_cast&)
    {
        return def;
    }
}



//////////////////////////////////////////////////////////////////////////

template<class T>
class strout_t
{
public:
    strout_t(std::basic_string<T> &rec, size_t size)
        : buffer_(size)
        , recipient_(&rec)
    {
    }

    operator T *() const
    {
        return &buffer_[0];
    }

    ~strout_t()
    {
        try
        {
            *recipient_= &buffer_[0];
        }
        catch (std::exception &)
        {
        }
    }

    // str_buf_t(str_buf_t const&); // ok
    // str_buf_t& operator= (str_buf_t const&); // ok

private:
    std::basic_string<T> *recipient_;
    mutable std::vector<T> buffer_;
};

//////////////////////////////////////////////////////////////////////////

template<class T> strout_t<T> strout(std::basic_string<T> &str, size_t size)
{
    return strout_t<T>(str, size);
}

//////////////////////////////////////////////////////////////////////////

}

using strlib::strout;
#define NEED_LPCSTR strlib::c_str()<<=
