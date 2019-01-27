#pragma once

#include "VesselElements.h"

#include "utilites/serl/Archive.h"

namespace location
{

struct VesselPart
{
    enum { Cylinder = 0,
           Ellipsoid = 1,
           Sphere = 2,
           Cone = 3,
           Flat = 4
         };

    int    type;
    double diameter1; // больший диаметр
    double diameter2; // меньший диаметр
    double height;

    bool operator==(const VesselPart& other) const;
    bool operator!=(const VesselPart& other) const;

    void serialization(serl::archiver& ar)
    {
        ar.serial("type", type);
        ar.serial("diam1", diameter1);
        ar.serial("diam2", diameter2);
        ar.serial("height", height);
    }

    bool has_diameter1() const {
        return has_diameter1_;
    }

    bool has_diameter2() const {
        return has_diameter2_;
    }

    bool has_height() const {
        return has_height_;
    }

    bool is_upper_bottom() const {
        return is_upper_bottom_;
    }

    PVElement get_element() const;

    double get_height_y() const;

private:
    //     элементы, доступные после компил€ции

    friend class VesselStructure;

    PVElement element;

    bool   has_diameter1_;
    bool   has_diameter2_;
    bool   has_height_;

    bool   is_upper_bottom_;
    double height_y;

    PVElement compile();
};


}
