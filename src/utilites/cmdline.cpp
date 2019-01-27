//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "utilites/Exception.h"
#include "utilites/cmdline.h"

CmdLine::CmdLine()
{
    cmdline_=::GetCommandLine();
    parse();
}

CmdLine::CmdLine(const char *str)
    : cmdline_(str)
{
    parse();
}

CmdLine &CmdLine::operator=(const char *str)
{
    cmdline_=str;
    parse();
    return *this;
}

int CmdLine::size() const
{
    return argv_.size()-1;
}

std::string CmdLine::operator[](int index) const
{
    return argv_.at(index);
}

unsigned CmdLine::option_size() const
{
    return std::count_if(argv_.begin(), argv_.end(), is_option());
}

std::string CmdLine::full_option(unsigned index) const
{
    debug::Assert<overflow>(index < option_size(), HERE);
    std::deque<std::string> tmp;
    stdex::copy_if(
        argv_.begin(), argv_.end(),
        std::back_inserter(tmp),
        is_option());
    return tmp.at(index).substr(1);
}
std::string CmdLine::option(unsigned index) const
{
    std::string option=full_option(index);
    size_t pos=option.find("=");
    if (pos!=std::string::npos)
    {
        option.erase(pos);
    }
    return option;
}

std::string CmdLine::option_addenda(unsigned index) const
{
    std::string option=full_option(index);
    size_t pos=option.find("=");
    if (pos!=std::string::npos)
    {
        option.erase(0, pos+1);
        return option;
    }
    else
    {
        return std::string();
    }
}

unsigned CmdLine::file_size() const
{
    return size()-option_size();
}

std::string CmdLine::file(unsigned index) const
{
    debug::Assert<overflow>(index < file_size(), HERE);
    std::deque<std::string> tmp;
    stdex::copy_if(
        argv_.begin(), argv_.end(),
        std::back_inserter(tmp),
        std::not1(is_option()));
    return tmp.at(index+1);
}

bool CmdLine::has_option(std::string opt) const
{
    for(unsigned index=0; index < option_size(); ++index)
        if (option(index) == opt)
            return true;

    return false;
}

std::string CmdLine::get_option_param(std::string opt) const
{
    for(unsigned index=0; index < option_size(); ++index)
        if (option(index) == opt)
            return option_addenda(index);

    return "";
}

void CmdLine::parse()
{
    std::vector<char> buffer(cmdline_.begin(), cmdline_.end());
    if (buffer.empty()) buffer.push_back(0);

    std::vector<char> tmp; tmp.reserve(buffer.size()+1);
    char *start=&buffer[0];
    char *index=start;
    char *tokenend=start;
    char *end=start+buffer.size();

    char *quotes="\"\'";
    int quote=0;
    for (; index<end;)
    {

        // searching first not-space
        for (; index<end && isspace(*index); ++index);

        // extracting token
        start=index;

        for (; index<end ; ++index)
        {
            if (quote)
            {
                if (quote==*index)
                {
                    quote=0;
                    continue;
                }
            }
            else
            {
                if (isspace(*index))
                {
                    break;
                }
                if (strchr(quotes, *index)!=0)
                {
                    quote=*index; continue;
                }
            }
            tmp.push_back(*index);
        }

        if (start<index)
        {
            if (!tmp.empty())
            {
                if (tmp[0]=='/')
                {
                    tmp[0]='-';
                }
                argv_.push_back(std::string(&tmp[0], &tmp[0]+tmp.size()));
                tmp.clear();
            }
        }
    }
}

std::string CmdLine::exefile() const
{
    return argv_[0];
}
