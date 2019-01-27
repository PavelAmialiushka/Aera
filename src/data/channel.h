//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#pragma once

#include "criterion.h"

namespace monpac
{


class calculator;
class channel
{
public:

    explicit channel(int num);
    int get_number() const;

    double get_duration() const;
    double get_hits65() const;
    double get_total_hits() const;
    double get_hold_hits(unsigned index) const;

    double h_index() const;
    double s_index() const;
    int	   get_zip_hits() const;
    zip_t  get_zip() const;

    bool   meet_duration() const;
    bool	 meet_hold_hits() const;
    bool   meet_total_hits() const;
    bool   meet_hits65() const;

private:

    size_t number_;

    std::vector<unsigned> hold_hits_;
    double duration_;
    double hits65_;
    double total_hits_;

    double s_index_;
    double h_index_;

    int    zip_hits_;
    zip_t  zip_;

    bool   meet_duration_;
    bool   meet_hold_hits_;
    bool   meet_total_hits_;
    bool   meet_hits65_;

    friend class calculator;
};

//////////////////////////////////////////////////////////////////////////

MAKE_SHARED(calculator);

class calculator
{
public:

    calculator(criterion *, int num);

    void append(hit h);
    void set_criterion(criterion cri);

    channel calculate();

private:

    int    k_index(unsigned sz) const;
    void   trace_h_index();
    double calc_h_index(unsigned sz) const;
    double calc_h_index() const;
    double calc_s_index() const;
    zip_t  calc_zip() const;

private:

    size_t number_;

    std::vector<unsigned> hold_hits_;
    double duration_;
    double hits65_;
    double total_hits_;

    double traced_h_index_;
    double total_sum_;
    double last_sum_;

    double s_index_;
    double h_index_;

    int zip_hits_;

    criterion *criterion_;
    std::vector<double> values_;
};

}
