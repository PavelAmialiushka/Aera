#include "stdafx.h"

#include "data/rawcollection.h"
#include "data/rawrecord.h"

namespace data
{

//////////////////////////////////////////////////////////////////////////

class raw_record_t : public raw_record
{
public:
    raw_record_t(const raw_info &inf);
    void receive_data(
        char *data, const char *srcdata, unsigned datalen,
        char *string, const char *srcstring, unsigned strlen);
};

//////////////////////////////////////////////////////////////////////////


raw_info::raw_info(int id, double time, raw_type_t type, std::string string)
    : type_(type),
      id_(id),
      time_(time),
      string_(string),
      submessage_(false)
{
}

//////////////////////////////////////////////////////////////////////////

raw_record_t::raw_record_t(const raw_info &inf)
{
    id_=inf.id_;
    type_=inf.type_;
    time_=inf.time_;
    submessage_=inf.submessage_;
}

void raw_record_t::receive_data(
    char *data, const char *srcdata, unsigned datalen,
    char *string, const char *srcstring, unsigned strlen)
{
    if (data_size_=datalen)
    {
        data_=data;
        memmove(data, srcdata, data_size_);
    }
    else data_=0;

    if (string_size_=strlen)
    {
        string_=string;
        memmove(string, srcstring, string_size_);
    }
    else string_=0;
}

//////////////////////////////////////////////////////////////////////////

raw_collection::raw_collection()
{
}

void raw_collection::append_record(const char *begin, const char *end, const raw_info &inf)
{
    // правильность передаваемых данных
    debug::Assert<fault>(end>=begin && (begin && end || begin==end), HERE);

    raw_record_t *rec=FASTMEM_ALLOC( this, raw_record_t, (inf));
    rec->receive_data(
        (char *)allocate(end-begin), begin, end-begin,
        (char *)allocate(inf.string_.size()), inf.string_.c_str(), inf.string_.size());

    data_.push_back(rec);
}

unsigned raw_collection::size() const
{
    return data_.size();
}

const raw_record &raw_collection::get_record(unsigned index) const
{
    debug::Assert<fault>(index < data_.size(), HERE);
    return *data_[index];
}

void raw_collection::clear()
{
    data_.clear();
    deallocate_all();
}
}
