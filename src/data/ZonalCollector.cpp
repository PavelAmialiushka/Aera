#include "stdafx.h"

#include "ZonalCollector.h"
#include "ZonalSetup.h"
#include "LocationUnit.h"

namespace location
{

ZonalCollector::ZonalCollector(ZonalSetup const &setup)
{
    use_peak_time_ = setup.get_use_peak_time();
    work_velocity_ = setup.get_velocity().get(m_s);

    work_edtime_ = setup.get_definition_time().get_time( work_velocity_ );
    work_lockout_ = setup.get_lockout().get_time( work_velocity_ );
}

int ZonalCollector::join_hit(index_t const &ox)
{
    const double time=!use_peak_time_
                      ? ox.time - package_[0].time
                      : ox.peak - package_[0].peak;

    // если хит входит в событие то добавляем его
    // если входит в мертвую зону, то игнорируем
    // если превышает мертвую зону, то формируем новое событие

    const bool not_in_event = time > work_edtime_;
    const bool form_next_event = not_in_event && time > work_lockout_;

    if (form_next_event) return hitNext;
    else if (not_in_event) return hitIgnore;

    return hitJoin;
}

void ZonalCollector::hit_inserted()
{
    if (use_peak_time_ && package_[0].peak > package_.back().peak)
        std::swap(package_[0], package_.back());
}

bool ZonalCollector::calculate_coords(double*&)
{
    return true;
}

void ZonalCollector::create_minmax_map()
{
}

std::vector<aera::chars>           ZonalCollector::get_chars() const
{
    return std::vector<aera::chars>();
}

std::map<aera::chars, std::string> ZonalCollector::get_units() const
{
    return std::map<aera::chars, std::string>();
}


}
