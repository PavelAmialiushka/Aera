//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "console_out.h"

#include <boost/format.hpp>

#include "stage.h"
#include "channel.h"
#include "criterion.h"

namespace monpac
{

void console_outputter::out_header()
{
    buffer_.clear();
    buffer_ << (boost::format("%3s ") % "#");
    for (unsigned index=0; index<criterion_.get_holds().size(); ++index)
    {
        buffer_ << boost::format("%5s ") % criterion_.get_holds().at(index).name;
    }
    buffer_ << boost::format("%5s %7s %7s %10s %6s %3s\n")
            % "holds"
            % "Total"
            % "A>65"
            % "Durat"
            % "Monpac"
            % "ZIP";
}

std::string console_outputter::get_line_format(channel const &chan)
{
    std::string string="%3d ";
    for (unsigned index=0; index<criterion_.get_holds().size(); ++index)
    {
        string += "%5d " ;
    }
    string += ("%5c %5d %c %5d %c %8.0f %c %6c %3s\n");

    return string;
}

}