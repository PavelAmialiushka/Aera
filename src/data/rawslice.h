#pragma once

#include "slice.h"

namespace data
{

class raw_slice : public slice
{
public:

    raw_slice(const raw_collection  *m)
        : master_(m)
    {
    }

    // override slice functions
    unsigned size() const
    {
        return index_.empty()
               ? master_->size()
               : index_.size();
    }

    virtual pslice clone() const
    {
        return pslice( new raw_slice(master_) );
    }

    raw_record get_raw_record(unsigned index) const
    {
        raw_record record=
            index<index_.size()
            ? master_->get_record( index_[index] )
            : master_->get_record( index );
        return record;
    }

    const double &get_value(const double *, aera::chars, int channel) const
    {
        static double zero=0.0;
        return zero;
    }

    const double &get_value(unsigned index, aera::chars value, int=0) const
    {
        if (value==aera::C_Time)
        {
            const raw_record &record=
                index<index_.size()
                ? master_->get_record( index_[index] )
                : master_->get_record( index );
            return record.time_;
        }

        static double zero=0.0;
        return zero;
    }

    int get_type(unsigned index) const
    {
        return RAW;
    }

    std::vector<aera::chars> get_chars() const
    {
        std::vector<aera::chars> tmp;
        tmp.push_back(aera::C_Time);
        return tmp;
    }

private:
    std::vector<unsigned> index_;
    const raw_collection  *master_;
};

}
