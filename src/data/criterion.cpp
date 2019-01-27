//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "criterion.h"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "stage.h"

namespace monpac
{

criterion::criterion()
    : vessel_(0)
{
}

void criterion::set_stage(const stage &stg)
{
    stage_=stg;
    holds_.clear();

    for (unsigned index=0; index<stg.children_.size(); ++index)
    {
        assert(stg.children_[index]);
        const std::vector< shared_ptr<stage> > &vector=
            stg.children_[index]->children_;

        if (vector.size())
        {
            for (unsigned jfoo=0; jfoo<vector.size(); ++jfoo)
            {
                assert( vector[jfoo] );
                holds_.push_back( *vector[jfoo] );
            }
        }
        else
        {
            holds_.push_back( *stg.children_[index] );
        }
    }
}

stage criterion::get_stage() const
{
    return stage_;
}

void criterion::set_vessel(int v)
{
    vessel_=v;
}

double criterion::get_threashold() const
{
    return vessel_==1
           ? 60
           : 50;
}

const std::vector<hold> &criterion::get_holds() const
{
    return holds_;
}

int criterion::get_hold_pause() const
{
    return vessel_==0
           ? 0
           : 120;
}

static bool contains(const hold &hld, double time)
{
    return hld.start <= time && time <= hld.end;
}

int criterion::get_hold(double time) const
{
    std::vector<hold>::const_iterator index
        =std::find_if(STL_II(holds_), bind(&contains, _1, time));
    if (index==holds_.end() || time - index->start < get_hold_pause())
    {
        return -1;
    }
    else
    {
        return std::distance(holds_.begin(), index);
    }
}

int criterion::get_vessel() const
{
    return vessel_;
}

int criterion::get_max_hit65() const
{
    return 0;
}

double criterion::get_max_duration() const
{
    return vessel_==0
           ? 1000
           : vessel_==1
           ? 9999999
           : 2500;
}

int criterion::get_max_total_hits() const
{
    return vessel_==0
           ? std::max(2, 8*5)
           : vessel_==1
           ? 99999999
           : 20;
}

unsigned criterion::get_max_hold_hits() const
{
    return 2;
//  2
//  2+
//  2+
}


namespace
{

double line(double x, double XA, double ya, double XB, double yb)
{
    // (y-a)/(b-a)==(x-A)/(B-A)
    return (x-XA)*(yb-ya)/(XB-XA)+ya;
}

}

zip_t criterion::get_zip(unsigned cnt, double h, double s)
{
    /*
    hi=hid**0.65
    s=210.6*sd**0.45

    // units of Ex = Amplitude * Counts

    const double A=3;
    const double B=5;
    const double C=6;
    const double a=0.01;
    const double b=0.04;
    const double c= 0.1;
    const double d0=0.6;
    const double d=   2;
    const double e=  10;

    // units of Energy
    */

    const double A=2.042;   // Ax ** 0.65
    const double B=2.847;
    const double C=3.205;

    const double a=26.51;   // 210.6 * ax ** 0.45
    const double b=49.47;
    const double c=74.72;
    const double d0=167.35;
    const double d=287.69;
    const double e=593.55;

    if (cnt<10) return zip_na;
    if (s<a) return zip_0;
    if (s<b && h<A) return zip_a;

    // zipb
    if (s<c && h<A || s < c && h < B && s < line(h, A, c, B, b)) return zip_b;

    // zipc
    if (s<d && h<A || s < d0 && h < C && s < line(h, A, d0, C, c) || s < c ) return zip_c;

    // zipd
    if (s<e && h<C || s < d) return zip_d;

    return zip_e;
}

void criterion::set_test_threshold(bool b)
{
    test_threshold_=b;
}

bool criterion::get_test_threshold() const
{
    return test_threshold_;
}

void criterion::serialization(serl::archiver &ar)
{
    ar.serial("vessel_type",
              serl::indirect(
                  this, vessel_,
                  &criterion::set_vessel,
                  &criterion::get_vessel));

    ar.serial_container("holds", holds_);
    ar.serial("use_test_threshold", serl::makeint(test_threshold_));
}

}