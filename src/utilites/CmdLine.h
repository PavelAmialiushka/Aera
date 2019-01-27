//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "utilites\copy_if.h"

class CmdLine
{
public:
    CmdLine();
    CmdLine(const char *str);
    CmdLine &operator=(const char *str);

    int size() const;
    std::string operator[](int index) const;

    unsigned option_size() const;
    std::string option(unsigned index) const;
    std::string option_addenda(unsigned index) const;

    unsigned file_size() const;
    std::string file(unsigned index) const;

    bool has_option(std::string) const;
    std::string get_option_param(std::string) const;

    std::string exefile() const;

private:

    std::string full_option(unsigned index) const;
    void parse();

private:

    std::string             cmdline_;
    std::deque<std::string> argv_;

    struct is_option : std::unary_function<std::string, bool>
    {
        bool operator()(const std::string &string) const
        {
            return !string.empty()
                   ? string[0]=='-'
                   : false;
        }
    };
};
