//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "data_fwrd.h"

#include "criterion.h"
#include <boost/format.hpp>

namespace monpac
{

class classifier;

/////////////////////////////////////////////////////////////////////////

class outputter
{
public:

    outputter();
    virtual void accept(classifier*);

    std::string get_text() const;

protected:

    unsigned get_holds_count() const;

    virtual void out_header();
    virtual void out_finish();

    virtual std::string get_line_format(channel const &)=0;

    std::string get_zip(channel const &chan) const;
    std::pair<bool, double> hold_criterion(channel const &chan, int index) const;
    std::pair<bool, double> total_criterion(channel const &chan) const;
    std::pair<bool, double> a65_criterion(channel const &chan) const;
    std::pair<bool, double> dur_criterion(channel const &chan) const;

    void print_line(channel const &);

protected:

    void out(boost::format &fmt, const std::string &str);

protected:
    criterion criterion_;
    std::ostringstream buffer_;
};

}
