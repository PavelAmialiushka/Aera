#pragma once

#include "object_id.h"
#include "utilites/serl_fwd.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

class Collector;
typedef shared_ptr<Collector> pcollector;

//////////////////////////////////////////////////////////////////////////

class SetupImpl
{
public:
    virtual ~SetupImpl() {};

    virtual pcollector create_collector(unsigned size)=0;
    virtual std::auto_ptr<SetupImpl> clone() const=0;
    virtual void serialization(serl::archiver &)=0;
};

class ZonalSetup;
class LinearSetup;
class PlanarSetup;
class VesselSetup;

//////////////////////////////////////////////////////////////////////////

enum {locZonal, locLinear, locPlanar, locVessel};

class Setup
{
public:
    Setup();
    Setup(SetupImpl *);
    static Setup create(int);

    Setup(Setup const &other);
    Setup &operator=(Setup other);

    void swap(Setup &other);

    bool operator==(Setup const &other) const;
    bool operator!=(Setup const &other) const;

    friend bool operator==(Setup const &self, object_id const &id);
    friend bool operator==(object_id const &id, Setup const &self);

    void serialization(serl::archiver &);

    bool empty() const;
    std::string name() const;

    int get_number() const;
    void set_number(int);

    object_id get_id() const;

    ZonalSetup* zonal() const;
    LinearSetup* linear() const;
    PlanarSetup* planar() const;
    VesselSetup* vessel() const;

    pcollector create_collector(unsigned size);

private:
    shared_ptr<SetupImpl> impl_;
    object_id             id_;
    int                   number_;
};

//////////////////////////////////////////////////////////////////////////

}
