#pragma once

#include "data/allocator.h"
#include "data/rawrecord.h"

namespace data
{

////////////////////////////////////////////////////////////////////////

struct raw_info
{
    raw_info(int id, double time=0.0, raw_type_t type=CR_NONE, std::string string=std::string());
public:
    int         id_;
    double      time_;
    raw_type_t  type_;
    bool		submessage_;
    std::string string_;
};

//////////////////////////////////////////////////////////////////////////

class raw_collection : fastmem::allocator
{
public:
    raw_collection();

    void clear();
    void append_record(const char *begin, const char *end, const raw_info &inf);

    const raw_record &get_record(unsigned index) const;
    unsigned size() const;

private:
    std::vector<raw_record *> data_;
};

}
