#include "stdafx.h"

#include "data/tddcollection.h"

namespace data
{

//////////////////////////////////////////////////////////////////////////

tdd_collection::tdd_collection()
    : channel_count_(0)
{
}

void tdd_collection::set_common_types(const aera::chars *chars, unsigned count)
{
    common_types_.assign(chars, chars+count);
    debug::Assert<fault>(common_types_.size()>=1, HERE);
//    debug::Assert<fault>(common_types_[0]==aera::C_Time, HERE);
}

void tdd_collection::get_common_types(std::vector<aera::chars> &result) const
{
    result.resize(common_types_.size());
    std::copy(STL_II(common_types_), result.begin());
}

void tdd_collection::set_channel_types(const aera::chars *chars, unsigned count)
{
    channel_types_.assign(chars, chars+count);
}

void tdd_collection::get_channel_types(std::vector<aera::chars> &result) const
{
    result.resize(channel_types_.size());
    std::copy(STL_II(channel_types_), result.begin());
}

void tdd_collection::copy_append_record(const double *data, unsigned count)
{
    double *buffer=static_cast<double *>(allocate((count+1)*sizeof(double)));

    const size_t cmn_sz=common_types_.size();
    std::copy(data, data+cmn_sz, buffer);

    // число каналов
    int channel_count = channel_types_.size()
            ? (count-cmn_sz)/channel_types_.size()
            : 0;
    reinterpret_cast<unsigned &>(buffer[cmn_sz]) = channel_count;

    std::copy(data+cmn_sz, data+count, buffer+cmn_sz+1);

    data_.push_back(buffer);
}

void tdd_collection::append_allocated_record(double *rec)
{
    data_.push_back(rec);
}

double *tdd_collection::allocate_record(unsigned sz)
{
    return reinterpret_cast<double *>(allocate(sz*sizeof(double)));
}

const double *tdd_collection::get_record(unsigned index) const
{
    debug::Assert<fault>(index<size(), HERE);

    return data_[index];
}

unsigned tdd_collection::size() const
{
    return data_.size();
}

void tdd_collection::set_channel_count(unsigned c)
{
    assert(size()==0);
    channel_count_ = c;
}

unsigned tdd_collection::get_channel_count() const
{
    return channel_count_;
}

void tdd_collection::clear()
{
    data_.clear();
    common_types_.clear();
    channel_types_.clear();

    deallocate_all();
}

}
