//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "outputter.h"

#include "stage.h"
#include "monpac.h"
#include "channel.h"
#include "criterion.h"

#include "monpac_result.h"

#include <boost/lexical_cast.hpp>

namespace monpac
{

static std::string cvt(int x)
{
    char buffer [30]; itoa(x, buffer, 10);
    return buffer;
}

static std::string cvt(unsigned x)
{
    return cvt((int)x);
}

static std::string cvt(double x, int pr=4)
{
    char buffer[30];
    _snprintf(buffer, SIZEOF(buffer), "%.*f", pr, x);
    return buffer;
}

outputter::outputter()
    : criterion_()
{
}

void outputter::accept(classifier *worker)
{
    pmonpac_result r = process::get_result<monpac_result>(worker);
    criterion_=worker->get_criterion();

    std::vector<int> channels;
    for (unsigned index=0; index<r->ch_info_.size(); ++index)
    {
        channels.push_back( r->ch_info_[index].get_number() );
    }
    out_header();

    for (unsigned index=0; index<r->ch_info_.size(); ++index)
    {
        print_line(r->ch_info_[index]);
    }

    out_finish();
}

unsigned outputter::get_holds_count() const
{
    return criterion_.get_holds().size();
}

std::string outputter::get_text() const
{
    return buffer_.str();
}

void outputter::out_header()
{
}

std::string outputter::get_line_format(channel const &)
{
    return "";
}

void outputter::out_finish()
{
}

std::string outputter::get_zip(channel const &chan) const
{
    std::ostringstream str;
    switch ( chan.get_zip() )
    {
    case zip_0: str << "-";	break;
    case zip_a: str <<"A";	break;
    case zip_b: str <<"B";	break;
    case zip_c: str <<"C";	break;
    case zip_d: str <<"D";	break;
    case zip_e: str <<"E";	break;
    case zip_na: str <<"N/A";	break;
    }
    return str.str();
}

std::pair<bool, double> outputter::hold_criterion(channel const &chan, int index) const
{
    double result=chan.get_hold_hits(index);
    return std::make_pair(
               result > criterion_.get_max_hold_hits(),
               result);
}

std::pair<bool, double> outputter::total_criterion(channel const &chan) const
{
    double result=chan.get_total_hits();
    return std::make_pair(
               result > criterion_.get_max_total_hits(),
               result);
}

std::pair<bool, double> outputter::a65_criterion(channel const &chan) const
{
    double result=chan.get_hits65();
    return std::make_pair(
               result > criterion_.get_max_hit65(),
               result);
}

std::pair<bool, double> outputter::dur_criterion(channel const &chan) const
{
    double result=chan.get_duration();
    return std::make_pair(
               result > criterion_.get_max_duration(),
               result);
}

void outputter::print_line(channel const &chan)
{
    double v;

    std::string text=get_line_format(chan);
    assert(!text.empty() && "get_line_format() cannot return empty string");

    text=strlib::replace(text, "$CHN", cvt(chan.get_number(),0));

    bool d=false;
    for (unsigned index=0; index<criterion_.get_holds().size(); ++index)
    {
        bool x; tie(x, v)=hold_criterion(chan, index);
        d = d || x;
        text=strlib::replace(text, "$HLD"+cvt(index), cvt(v,0));
    }

    text=strlib::replace(text, "$HLDX", d ? "-" : "+" );

    bool a, b, c;
    tie(a, v)=total_criterion(chan);
    text=strlib::replace(text, "$TTLX", a ? "-" : "+" );
    text=strlib::replace(text, "$TTL", cvt(v,0));

    tie(b, v)=a65_criterion(chan);
    text=strlib::replace(text, "$A65X", b ? "-" : "+" );
    text=strlib::replace(text, "$A65", cvt(v,0));

    tie(c, v)=dur_criterion(chan);
    text=strlib::replace(text, "$DURX", c ? "-" : "+" );
    text=strlib::replace(text, "$DUR", cvt(v, 0));

    text=strlib::replace(text, "$SEV", cvt(chan.s_index(),1));
    text=strlib::replace(text, "$HIX", cvt(chan.h_index(),4));

    text=strlib::replace(text, "$MPX", a || b || c || d ? "-" : "+");
    text=strlib::replace(text, "$ZIP", get_zip(chan) );

    buffer_ << text;
}

}
