#include "stdafx.h"

#include "parametric.h"
#include "utilites/serl/Archive.h"

//////////////////////////////////////////////////////////////////////////

parametric::parametric()
{
    addend=0;
    factor=1.0;
    name="parametric";
}

void parametric::serialization(serl::archiver &arc)
{
    arc.serial("addend", addend);
    arc.serial("factor", factor);
    arc.serial("name", name);
    arc.serial("unit", unit_name);
}