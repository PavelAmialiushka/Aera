#include "stdafx.h"

#include "stage.h"
#include "utilites/serl/Archive.h"

stage::stage()
    : start(0)
    , end(0)
{
}

stage::stage(std::string c, double a, double b)
    : start(a)
    , end(b)
    , name(c)
    , id_(object_id::create())
{
}

void stage::serialization(serl::archiver &arc)
{
    arc.serial("name",  name);
    arc.serial("start", start);
    arc.serial("end",   end);
    arc.serial("id",    id_);
    arc.serial_static_ptr_container("vector", children_);

    if (arc.is_loading())
    {
        if (id_==object_id())
            id_=object_id::create();
    }
}

std::string stage::get_name() const
{
    return name;
}

shared_ptr<stage> stage::at(unsigned index) const
{
    return children_.at(index);
}

void stage::remove(unsigned index)
{
    if (index < children_.size())
        children_.erase(children_.begin()+index);
}

void stage::insert(unsigned index, shared_ptr<stage> stg)
{
    index < children_.size()
    ? children_.insert(children_.begin()+index, stg)
    : children_.push_back(stg);
}

unsigned stage::size() const
{
    return children_.size();
}

bool stage::contains(double time) const
{
    return start==end ? true : start <= time && time <= end;
}

bool stage::operator==(const stage &other) const
{
    return start==other.start && end==other.end;
}

bool stage::operator!=(const stage &other) const
{
    return !operator==(other);
}

object_id stage::get_id() const
{
    return id_;
}
//////////////////////////////////////////////////////////////////////////

#include <boost/signal.hpp>

class stages::impl
{
public:
    boost::signal0<void>	changed;
};

stages::stages()
    : pimpl(new impl)
{
}

stages::~stages()
{
}

void stages::serialization(serl::archiver &arc)
{
    stage::serialization(arc);

    if (arc.is_loading())
    {
        changed();
    }
}

void stages::connect(int tp, boost::function0<void> fn)
{
    pimpl->changed.connect(tp, fn);
}

void stages::disconnect(int tp)
{
    pimpl->changed.disconnect(tp);
}

void stages::changed()
{
    pimpl->changed();
}

