#include "StdAfx.h"

#include "data/aecollection.h"

//////////////////////////////////////////////////////////////////////////

namespace data
{

ae_collection::ae_collection()
    : sorted_(true)
    , prevtime_(-1)
{

}

void ae_collection::set_typestring(const aera::chars *chars, unsigned count)
{
    chars_.assign(chars, chars+count);
    debug::Assert<fault>(chars_.size()>=1, HERE);
}

void ae_collection::copy_append_record(const double *data)
{
    double *buffer=static_cast<double *>(allocate(chars_.size()*sizeof(double)));
    std::copy(data, data+chars_.size(), buffer);

    append_allocated_record(buffer);
}

double *ae_collection::allocate_record(unsigned sz)
{
    return reinterpret_cast<double *>(allocate(sz*sizeof(double)));
}

void ae_collection::append_allocated_record(double *data)
{
    double time = data[0];
    if (time < prevtime_)
        sorted_ = false;
    prevtime_ = time;

    data_.push_back(data);
}

const double *ae_collection::get_record(unsigned index) const
{
    assert(index<data_.size());
    return data_[index];
}

unsigned ae_collection::size() const
{
    return data_.size();
}

bool ae_collection::sorted() const
{
    return sorted_;
}

void ae_collection::get_typestring(std::vector<aera::chars> &result) const
{
    result.resize(chars_.size());
    std::copy(STL_II(chars_), result.begin());
}

void ae_collection::clear()
{
    data_.clear();
    chars_.clear();

    deallocate_all();
}

}
