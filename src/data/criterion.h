//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "data_fwrd.h"
#include "stage.h"

#include "utilites/serl/Archive.h"

namespace monpac
{

//////////////////////////////////////////////////////////////////////////

enum zip_t
{
    zip_0,
    zip_a,
    zip_b,
    zip_c,
    zip_d,
    zip_e,
    zip_na,
};

//////////////////////////////////////////////////////////////////////////

struct hit
{
    hit(double t, double c, double a, double e, double d)
        : amplitude(a), channel(c), time(t), energy(e), duration(d)
    {
    }

    double time;
    double channel;
    double amplitude;
    double duration;
    double energy;
};

//////////////////////////////////////////////////////////////////////////

typedef stage hold;

//////////////////////////////////////////////////////////////////////////

class criterion : public serl::serializable

{
public:

    criterion();

    void   set_stage(const stage &stg);
    stage  get_stage() const;

    void   set_vessel(int);
    void   set_test_threshold(bool);

    zip_t  get_zip(unsigned cnt, double h, double s);
    int    get_vessel() const;

    int    get_max_hit65() const;
    double get_max_duration() const;
    int    get_max_total_hits() const;
    unsigned get_max_hold_hits() const;
    int    get_hold_pause() const;

    bool   get_test_threshold() const;

    double get_threashold() const;
    int    get_hold(double) const;
    const std::vector<hold> &get_holds() const;

    void   serialization(serl::archiver &ar);

private:

    bool test_threshold_;
    int vessel_;
    std::string text_;

    stage     stage_;
    std::vector< stage > holds_;
};

//////////////////////////////////////////////////////////////////////////
}