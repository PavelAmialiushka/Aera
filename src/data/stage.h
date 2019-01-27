#pragma once

#include "object_id.h"

#include "utilites/serl/Archive.h"

class stage
{
public:
    stage();
    stage(std::string c, double a, double b);

    virtual void serialization(serl::archiver &arc);

    bool contains(double) const;

    std::string get_name() const;
    unsigned size() const;
    shared_ptr<stage> at(unsigned index) const;

    void remove(unsigned index);
    void insert(unsigned index, shared_ptr<stage>);

    object_id get_id() const;

    bool operator==(const stage &) const;
    bool operator!=(const stage &) const;

    bool empty() const { return start == end; }

public:
    double start;
    double end;
    std::string name;
    std::vector<shared_ptr<stage> > children_;

private:
    object_id id_;
};

//////////////////////////////////////////////////////////////////////////

class stages
    : public utils::singleton<stages>
    , public stage
    , public serl::serializable
{
public:
    stages();
    ~stages();

    void serialization(serl::archiver &arc);

public:
    void changed();
    void connect(int, boost::function0<void>);
    void disconnect(int);

    class impl; scoped_ptr<impl> pimpl;
};

