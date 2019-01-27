//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "channel.h"
#include "criterion.h"
#include "stage.h"

namespace monpac
{

channel::channel(int num)
    : duration_(0)
    , hits65_(0)
    , total_hits_(0)
    , number_(num)
    , zip_hits_(0)
    , zip_(zip_0)
    , s_index_(0)
    , h_index_(0)
    , meet_duration_(false)
    , meet_hits65_(false)
    , meet_hold_hits_(false)
    , meet_total_hits_(false)
{
}

zip_t channel::get_zip() const
{
    return zip_;
}

double channel::get_duration() const
{
    return duration_;
}

double channel::get_hits65() const
{
    return hits65_;
}

double channel::get_total_hits() const
{
    return total_hits_;
}

double channel::get_hold_hits(unsigned index) const
{
    if (index < hold_hits_.size())
        return hold_hits_[index];

    assert(!"incorrect hold");
    return 0;
}

int channel::get_number() const
{
    return number_;
}

int channel::get_zip_hits() const
{
    return zip_hits_;
}

bool channel::meet_duration() const
{
    return meet_duration_;
}

bool channel::meet_hold_hits() const
{
    return meet_hold_hits_;
}

bool channel::meet_total_hits() const
{
    return meet_total_hits_;
}

bool channel::meet_hits65() const
{
    return meet_hits65_;
}

double channel::h_index() const
{
    return h_index_;
}

double channel::s_index() const
{
    return s_index_;
}


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////


calculator::calculator(criterion *criterion_, int num)
    : criterion_(criterion_)
    , number_(num)
    , duration_(0)
    , hits65_(0)
    , total_hits_(0)
    , s_index_(0)
    , h_index_(0)
    , zip_hits_(0)
    , total_sum_(0)
    , last_sum_(0)
    , traced_h_index_(0)
{
    hold_hits_.resize(criterion_->get_holds().size());
}

void calculator::append(hit h)
{
    // критерии
    if (h.amplitude >= criterion_->get_threashold())
    {
        duration_+=h.duration;
        total_hits_+=1;
        if (h.amplitude>=65) hits65_+=1;

        if (criterion_)
        {
            unsigned y=criterion_->get_hold(h.time);
            if (y<hold_hits_.size())
                hold_hits_[y]+=1;
        }
    }

    // интенсивность
    if (criterion_->get_test_threshold()
            ||h.amplitude >= criterion_->get_threashold())
    {
        values_.push_back(h.energy);
        trace_h_index();
        zip_hits_++;
    }

}

int calculator::k_index(unsigned sz) const
{
    if (sz<10) return 0;
    if (sz<=15) return 0;
    if (sz<=75) return sz-15;
    if (sz<=1000) return sz*8/10;
    return sz-200;
}

double calculator::calc_h_index(unsigned count_all) const
{
    // N-K
    unsigned count_first=k_index(count_all);
    unsigned count_last=count_all-count_first;

    // sigma i=K+1;N (s)
    double sum_last=std::accumulate(
                        values_.begin()+count_first,
                        values_.begin()+count_all,
                        0.0);

    // sigma i=1;N (s)
    double sum_all=std::accumulate(
                       values_.begin(),
                       values_.begin()+count_all,
                       0.0);

    return count_last!=0 && sum_all!=0
           ? count_all/sum_all  * sum_last/count_last
           : 1.0;
}

void calculator::trace_h_index()
{
    const size_t N=values_.size();

    if (N > 0)
    {
        total_sum_+=values_.back();
        last_sum_+=values_.back();
        if (N > 200) last_sum_-=values_[N - 200];

        if (total_sum_ > 0 && N>1000)
        {
            const double candidate = N / total_sum_ * last_sum_ / 200;
            traced_h_index_=std::max(traced_h_index_, candidate);
        }
    }
}

double calculator::calc_h_index() const
{
    double h_index_max=1.0;

    unsigned count=std::min<unsigned>(values_.size(), 1000u);
    for (unsigned index=1; index<=count; ++index)
    {
        h_index_max=std::max(h_index_max, calc_h_index(index));
    }

    if (values_.size()>1000)
        h_index_max = std::max(h_index_max, traced_h_index_);


    return h_index_max;
}

double calculator::calc_s_index() const
{
    std::vector<double> temp(values_);
    unsigned k=std::min<unsigned>(10u, temp.size());
    unsigned n=temp.size()-k;
    std::nth_element(temp.begin(), temp.begin()+n, temp.end());

    return k ? std::accumulate(temp.begin()+n, temp.end(), 0.0)/k : 0;
}

zip_t calculator::calc_zip() const
{
    return criterion_->get_zip(values_.size(), h_index_, s_index_);
}

channel calculator::calculate()
{
    channel ch(number_);

    ch.hold_hits_=hold_hits_;
    ch.hits65_=hits65_;
    ch.total_hits_=total_hits_;
    ch.duration_=duration_;

    ch.s_index_ = s_index_= calc_s_index();
    ch.h_index_ = h_index_ = calc_h_index();

    ch.zip_hits_=zip_hits_;
    ch.zip_=calc_zip();

    ch.meet_hold_hits_=hold_hits_.empty()
                       ? true
                       : *std::max_element(STL_II(hold_hits_)) <= criterion_->get_max_hold_hits();

    ch.meet_duration_= duration_ <= criterion_->get_max_duration();
    ch.meet_hits65_ = hits65_ <= criterion_->get_max_hit65();
    ch.meet_total_hits_ = total_hits_ <= criterion_->get_max_total_hits();

    return ch;
}
}
