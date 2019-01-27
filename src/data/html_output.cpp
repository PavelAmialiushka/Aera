//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "html_output.h"

#include "stage.h"
#include "channel.h"
#include "criterion.h"

#include <boost/format.hpp>

namespace monpac
{

void html_outputter::out_header()
{
    buffer_
            << "<html>\n<head><head>\n<body>\n<table border=1>\n"
            << boost::format("<tr><th>%s")
            % "#" ;
    for (unsigned index=0; index<get_holds_count(); ++index)
        buffer_ << boost::format("<th>%d") % criterion_.get_holds().at(index).name;

    buffer_ << boost::format("<th>%s<th colspan=2>%s<th colspan=2>%s<th colspan=2>%s<th>%s<th>%s</tr>\n")
            % "Hit during holds"
            % "Total hits"
            % "Amp>65"
            % "Duration"
            % "Monpac"
            % "ZIP";
}

namespace
{
void out(boost::format &fmt, std::pair<bool, double> pair)
{
    fmt % pair.second % (pair.first ? '-' : '+');
}
}

std::string html_outputter::get_line_format(channel const &)
{
    std::string string = "<tr><td>%3d ";
    for (unsigned index=0; index<criterion_.get_holds().size(); ++index)
    {
        string += "<td>%5d ";
    }
    string += "<td>%5c<td>%5d<td>%c<td>%5d<td>%c<td>%8.0f<td>%c<td>%6c<td>%3s</tr>\n";
    return string;
}


void html_outputter::out_finish()
{
    buffer_ << "</table></body></html>\n";
}

}
