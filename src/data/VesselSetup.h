#pragma once

#include "data/ZonalSetup.h"

#include "VesselElements.h"
#include "VesselStructure.h"

namespace location
{


struct VesselCoords
{
    enum {ByElement=0,
          BySensor=1 };

    enum {Coords_XY=0,
          Coords_XZ=1};

    int    number;
    double x;
    double y_or_z;
    int    base_type;
    int    base_index;
    int    coords_type;

    // после компиляции
    gfiy_coords global;
    lxy_coords lxy;

    void serialization(serl::archiver & ar);

    bool operator!=(VesselCoords const& rhs) const
    {
        return number != rhs.number
                || base_type != rhs.base_type
                || base_index != rhs.base_index
                || fabs(x - rhs.x) > 1e-4
                || fabs(y_or_z - rhs.y_or_z) > 1e-4;
    }

    friend bool operator<(VesselCoords const& lhs, VesselCoords const& rhs)
    {
        return lhs.number < rhs.number;
    }
    friend bool operator<(int lhs, VesselCoords const& rhs)
    {
        return lhs < rhs.number;
    }
    friend bool operator<(VesselCoords const& lhs, int rhs)
    {
        return lhs.number < rhs;
    }

};


class VesselSensors
{
    std::vector<VesselCoords> vector_;
public:

    typedef std::vector<VesselCoords>::iterator iter;
    typedef std::vector<VesselCoords>::const_iterator const_iter;

    iter begin() { return vector_.begin(); }
    iter end() { return vector_.end(); }

    const_iter begin() const { return vector_.begin(); }
    const_iter end() const { return vector_.end(); }

    size_t size() const
    {
        return vector_.size();
    }

    bool contains(int ix) const
    {
        std::pair<const_iter, const_iter> range
                = std::equal_range(STL_II(vector_), ix);
        return range.first != range.second;
    }

    VesselCoords operator[](int ix) const
    {    
        std::pair<const_iter, const_iter> range
                = std::equal_range(STL_II(vector_), ix);
        if (range.first != range.second)
        {
            return *range.first;
        }

        assert(!"not found");
        return VesselCoords();
    }

    VesselCoords& operator[](int ix)
    {
        std::pair<iter, iter> range
                = std::equal_range(STL_II(vector_), ix);
        if (range.first != range.second)
        {
            return *range.first;
        } else
        {
            VesselCoords newby;
            newby.number = ix;
            iter it = vector_.insert(range.first, newby);
            return *it;
        }
    }

    void serialization(serl::archiver& ar)
    {
        std::vector<VesselCoords> coords;
        ar.serial_container("sensors", vector_);
    }

    bool operator!=(VesselSensors const& rhs) const;
};

class VesselSetup
        : public ZonalSetup
{
public:
    VesselSetup();

    void set_vessel(VesselStructure const&);
    const VesselStructure& get_vessel() const;

    void set_sensors(VesselSensors const &);
    VesselSensors const& get_sensors() const;

    void get_vessel_m(VesselStructure &) const;
    void get_sensors_m(VesselSensors &) const;

    void  get_coordinate_range(int chr, double &minimum, double &maximum);

    void       set_deviation(unit_value);
    unit_value get_deviation() const;

    void set_unit(Unit);
    Unit get_unit() const;

    bool operator!=(VesselSetup const& rhs) const;

public:

    virtual pcollector create_collector(unsigned size);
    virtual std::auto_ptr<SetupImpl> clone() const;
    virtual void serialization(serl::archiver &);

private:
    VesselStructure vessel_;
    VesselSensors   sensors_;
    unit_value      deviation_;
    Unit            unit_;

};

} // namespace location

