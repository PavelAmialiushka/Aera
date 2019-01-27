#pragma once

#include "Collector.h"
#include "LinearSetup.h"

#include "ZonalCollector.h"

#include "allocator.h"

//////////////////////////////////////////////////////////////////////////

namespace location
{

class LinearCollector
    : public ZonalCollector
    , public fastmem::allocator
{
public:
    LinearCollector(LinearSetup const &);

public:

    virtual std::vector<aera::chars> get_chars() const;
    virtual std::map<aera::chars, std::string> get_units() const;
    virtual bool calculate_coords(double*&);
    virtual void create_minmax_map();

private:

    double                work_deviation_;
    double                work_circulation_;
    std::map<int, double> work_channels_;
    Unit                  unit_;

};

}
