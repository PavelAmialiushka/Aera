#include "StdAfx.h"

#include "data/collection.h"
#include "data/collection_writer.h"

namespace data
{

//////////////////////////////////////////////////////////////////////////

collection::collection()
{
    locked_=0;
}

collection::~collection()
{
    debug::Assert<fault>(!locked(), HERE);
}

const ae_collection &collection::ae() const
{
    return ae_collection_;
}

const raw_collection &collection::raw() const
{
    return raw_collection_;
}

const tdd_collection &collection::tdd() const
{
    return tdd_collection_;
}

int collection::locked() const
{
    boost::mutex::scoped_lock lock(mutex_);
    return locked_;
}

bool collection::lock() const
{
    boost::mutex::scoped_lock lock(mutex_);
    if (locked_>=0) return ++locked_, true;
    return false;
}

bool collection::write_lock() const
{
    boost::mutex::scoped_lock lock(mutex_);
    debug::Assert<fault>(locked_>=0, HERE);
    if (locked_==0) return --locked_, true;
    return false;
}

void collection::write_unlock() const
{
    boost::mutex::scoped_lock lock(mutex_);
    debug::Assert<underflow>(locked_<0, HERE);
    ++locked_;
}

void collection::unlock() const
{
    boost::mutex::scoped_lock lock(mutex_);
    debug::Assert<underflow>(locked_>0, HERE);
    --locked_;
}

pwriter_t collection::write_access()
{
    while (1)
    {
        if (write_lock())
        {
            return pwriter_t(new collection_writer(this));
        }
    }
}

//////////////////////////////////////////////////////////////////////////

}