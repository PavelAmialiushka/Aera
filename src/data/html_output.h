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

class html_outputter : public outputter
{
protected:

    void out_header();
    std::string get_line_format(channel const &);

    void out_finish();
};

}