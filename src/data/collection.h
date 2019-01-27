#pragma once

#include "data/aecollection.h"
#include "data/rawcollection.h"
#include "data/tddcollection.h"

#include "data/read_access.h"

namespace data
{

class collection_writer;

class collection
{
public:

    collection();
    ~collection();

    shared_ptr<collection_writer> write_access();

    int locked() const;

public:
    const ae_collection &ae() const;
    const raw_collection &raw() const;
    const tdd_collection &tdd() const;

public:

    bool lock() const;
    bool write_lock() const;
    void unlock() const;
    void write_unlock() const;

private:
    ae_collection ae_collection_;
    raw_collection raw_collection_;
    tdd_collection tdd_collection_;

    mutable int locked_;
    mutable boost::mutex mutex_;

    friend class collection_writer;
    friend class node;
};

//////////////////////////////////////////////////////////////////////////

}
