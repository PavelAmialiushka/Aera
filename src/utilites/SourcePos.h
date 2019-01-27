//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <boost/preprocessor/stringize.hpp>

namespace debug
{

//#define HERE std::make_pair(__LINE__, (const char*)__FILE__)
//typedef std::pair<int, const char*> SourcePos_tag;

typedef const char *SourcePos_tag;
#define HERE (__FILE__ "(" BOOST_PP_STRINGIZE(__LINE__) ")")

struct SourcePos
{
    SourcePos();
    SourcePos(const SourcePos &loc);

    SourcePos(SourcePos_tag tag);
    SourcePos &operator=(SourcePos_tag tag);

    const char *get_string() const;

    bool operator ==(const SourcePos &loc) const;
    bool operator !=(const SourcePos &loc) const;
    bool operator <(const SourcePos &loc) const;
protected:
    const char *loc_;

    friend std::ostream &operator <<(std::ostream &str, const debug::SourcePos &loc);
};

typedef SourcePos SourcePos;

std::ostream &operator <<(std::ostream &str, const debug::SourcePos &loc);

class function : public SourcePos
{
public:
    function();
private:
    std::string function_;
};

}

