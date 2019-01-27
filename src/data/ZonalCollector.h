#pragma once

#include "Collector.h"
#include "ZonalSetup.h"

namespace location
{

class ZonalCollector : public Collector
{
public:
    ZonalCollector(ZonalSetup const &);

public:

    virtual std::vector<aera::chars> get_chars() const;
    virtual std::map<aera::chars, std::string> get_units() const;

    virtual int join_hit(index_t const &ox);
    virtual void hit_inserted();
    virtual bool calculate_coords(double*&);
    virtual void create_minmax_map();

protected:

    double work_edtime_;
    double work_lockout_;
    double work_velocity_;
};

}
