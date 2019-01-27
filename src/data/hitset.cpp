#include "stdafx.h"

#include "hitset.h"

#include "utilites/foreach.hpp"
#include "utilites/tokenizer.h"

#include "utilites/serl/Archive.h"

namespace hits
{

hitset::~hitset()
{
}

void hitset::clear()
{
    assert(set_.size()<INT_MAX);
    set_.clear();
    deallocate_all();
}

void hitset::reserve(unsigned sz)
{
    assert(set_.size()<INT_MAX);
    set_.reserve(sz);
}

void hitset::append(const hitref &ref)
{
    hitref *temp=FASTMEM_ALLOC(this, hitref, (ref));
    set_.insert( temp );
}

void hitset::append(data::ae_record rec)
{
    assert(set_.size()<INT_MAX);
    hitref *ref=FASTMEM_ALLOC(this, hitref, (rec[0], rec[1]));
    set_.insert( ref );
}

void hitset::remove(const hitref &ref)
{
    set_.erase(const_cast<hitref *>(&ref));
}

void hitset::remove(data::ae_record rec)
{
    assert(set_.size()<INT_MAX);
    hitref ref(rec[0], rec[1]);
    set_.erase(&ref);
}

hitref *hitset::get(data::ae_record rec) const
{
    hitref tmp(rec[0], rec[1]);
    set_t::const_iterator index=set_.find( &tmp );
    return index!=set_.end() ? *index : 0;
}

bool hitset::contains(double const* rec) const
{
    hitref tmp(rec[0], rec[1]);
    set_t::const_iterator index=set_.find( &tmp );
    return index!=set_.end() ? *index : 0;
}

std::string hitset::to_string() const
{
    std::string string;
    string.reserve((sizeof(double)+sizeof(double)) * set_.size());

    foreach(set_t::value_type value, set_)
    {
        string.append(reinterpret_cast<char *>(&value->time), sizeof(double)*2);
    }
    return string;
}

void hitset::make_from_string(std::string string)
{
    clear();
    reserve(string.size()/2/sizeof(double));

    unsigned pos=0;
    for (const double *ptr=reinterpret_cast<const double *>(string.c_str());
            pos<string.size(); ptr+=2, pos+=2*sizeof(double))
    {
        hitref *ref=FASTMEM_ALLOC(this, hitref, (ptr[0], ptr[1]));
        set_.insert( ref );
    }
}

void hitset::serialization(serl::archiver &ar)
{
    std::string tmp=ar.is_saving() ? to_string() : "";
    ar.serial("hits", tmp);
    if (ar.is_loading()) make_from_string(tmp);
}

unsigned hitset::size() const
{
    return set_.size();
}

void hitset::check()
{
    assert(set_.size()<INT_MAX);
    double cf=set_.collision_factor();
}

}
