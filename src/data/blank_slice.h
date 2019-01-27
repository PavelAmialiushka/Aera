#pragma once

#include "slice.h"

namespace data
{

class blank_slice : public slice
{
public:

    blank_slice()
    {
    }

    virtual pslice clone() const
    {
        return pslice ( new blank_slice );
    }
    virtual pslice clone(const std::vector<unsigned> &other) const
    {
        return clone();
    }

    virtual unsigned size() const
    {
        return 0;
    }

    virtual const double &get_value(unsigned index, aera::chars=aera::C_Time, int channel=0) const
    {
        assert(false);
        throw not_implemented(HERE);
    }

    virtual std::vector<aera::chars> get_chars() const
    {
        return std::vector<aera::chars>();
    }

    // fat interface


    // raw
    virtual raw_record get_raw_record(unsigned index) const
    {
        throw not_implemented(HERE);
    }

    // tdd
    virtual unsigned get_channel_count() const
    {
        throw not_implemented(HERE);
    }

};

}
