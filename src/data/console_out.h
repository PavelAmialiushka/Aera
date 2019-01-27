//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "outputter.h"

namespace monpac
{

class channel;

class console_outputter : public outputter
{
protected:

    void out_header();
    int get_holds() const;
    std::string get_line_format(channel const &);

};
}