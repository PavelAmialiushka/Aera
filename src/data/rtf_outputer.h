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

class rtf_outputter : public outputter
{
public:
    rtf_outputter(bool in=false);

protected:


    void out_header();
    void out_finish();

    std::string get_line_format(channel const &);
private:
    bool intensivity_;
};

}