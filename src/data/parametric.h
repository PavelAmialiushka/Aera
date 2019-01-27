#pragma once

#include "utilites/serl_fwd.h"

struct parametric
{
    double	factor;
    double  addend;
    std::string unit_name;
    std::string name;

    parametric();
    void serialization(serl::archiver &);
};

