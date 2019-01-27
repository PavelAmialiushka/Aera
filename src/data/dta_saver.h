#pragma once

#include "slice.h"
#include "dta_record.h"

namespace data
{

class dta_saver
{
public:
    dta_saver(std::string path);

    void write(pslice, unsigned);

private:
    dta_record    record_;
    std::ofstream stream_;

    friend class dta_record;
};

}
