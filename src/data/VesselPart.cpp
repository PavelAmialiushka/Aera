#include "stdafx.h"

#include "VesselPart.h"

namespace location
{

bool VesselPart::operator==(const VesselPart &other) const
{
    if (type == other.type
            && fabs(diameter1 - other.diameter1) < 1e-4
            && fabs(diameter2 - other.diameter2) < 1e-4
            && fabs(height - other.height) < 1e-4
            )
        return true;
    return false;
}

bool VesselPart::operator!=(const VesselPart &other) const
{
    return !((*this) == other);
}

PVElement VesselPart::get_element() const
{
    return element;
}

double VesselPart::get_height_y() const
{
    return height_y;
}

PVElement VesselPart::compile()
{
    if (element) return element;

    double d_bottom = diameter1; // больший диаметр
    double d_top = diameter2;    // меньший
    if (!is_upper_bottom())
        std::swap(d_bottom, d_top); // для нижнего днища

    switch(type)
    {
    case VesselPart::Cylinder:
        element = PVElement(new VesselElementCyl(height, diameter1));
        break;
    case VesselPart::Ellipsoid:
        element = PVElement(new VesselElementEllipsoid(height, d_bottom, d_top));
        break;
    case VesselPart::Sphere:
        element = PVElement(new VesselElementEllipsoid(height, d_bottom, d_top));
        break;
    case VesselPart::Cone:
        element = PVElement(new VesselElementCone(height, d_bottom, d_top));
        break;
    case VesselPart::Flat:
        element = PVElement(new VesselElementCone(0, d_bottom, d_top));
        break;
    default:
        assert(!"incorrect enum value");
        return PVElement();
    }
    height_y = element->height_y;
    return element;
}


}
