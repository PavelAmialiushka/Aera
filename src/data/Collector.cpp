#include "stdafx.h"

#include "Collector.h"
#include "Traits.h"

#include "hitref.h"
#include "data/aeslice.h"

namespace location
{

Collector::Collector()
{
    use_peak_time_ = false;
}

void Collector::append_record(data::pslice slice, unsigned index)
{
    index_t ox= {0};
    ox.index = index;
    const double *record=&slice->get_value(index);

    ox.rec      =record;
    ox.time     =*record;
    ox.channel  =slice->get_value(index, aera::C_Channel);
    ox.amplitude=slice->get_value(index, aera::C_Amplitude);
    ox.energy   =slice->get_value(index, aera::C_Energy);
    double rt   =slice->get_value(index, aera::C_Risetime);
    ox.peak     =ox.time + rt / 1e6;

    if (!package_.empty())
    {
        switch (join_hit(ox))
        {
        case hitIgnore:     // не записывать
            ignore_.push_back( ox );
            return;
        case hitNext:       // сначала записать предыдущее событие
            create_event();
        case hitJoin:       // добавить
            break;
        }
    }

    package_.push_back(ox);
    hit_inserted();
}

void Collector::create_event()
{
    if (package_.empty()) return;

    double* crds = 0;
    bool event_created = calculate_coords(crds);

    if (event_created)
    {
        assert(package_[0].rec);
        set_->append( package_[0].rec );
        pos_indexes_.push_back( package_[0].index );

        int packageHitCount =package_.size();
        int ignoreHitCount = ignore_.size();
        double* pos_rec = pos_data_.allocate_record(
                    pos_chars_.size()
                    + 2*packageHitCount
                    + 2*ignoreHitCount);
        pos_data_.append_allocated_record(pos_rec);

        unsigned idx=0;
        for(; idx < pos_coords_count_; ++idx )
            pos_rec[idx] = crds[idx];

#define REG_MINMAX(type, value) do {\
	    minmax_map_.add_value(type, value); \
        } while(0)

        if (use_peak_time_)
            pos_rec[idx++] = package_[0].peak;

        pos_rec[idx++] = packageHitCount-1;
        REG_MINMAX(aera::C_SubHitCount, pos_rec[idx-1]);

        pos_rec[idx++] = ignoreHitCount;
        REG_MINMAX(aera::C_IgnoredHitCount, pos_rec[idx-1]);

        for(int index=1; index < packageHitCount; ++index)
        {
            // сохраняем положительную запись
            index_t item = package_[index];
            pos_rec[idx++] = item.time;  // здесь минмакс регистрировать бессмысленно
            pos_rec[idx++] = item.channel;

            // сохраняем в отрицательную запись
            int nidx = 0;
            double* neg_rec = neg_data_.allocate_record(neg_chars_.size()
                                                        + 2 // половина C_FirstHit
                                                        );
            if (use_peak_time_)
                neg_rec[nidx++] = package_[index].peak;

            neg_rec[nidx++] = package_[index].time - package_[0].time;
            REG_MINMAX(aera::C_RelTime, neg_rec[nidx-1]);

            neg_rec[nidx++] = package_[index].peak - package_[0].peak;
            REG_MINMAX(aera::C_RelPeakTime, neg_rec[nidx-1]);

            // C_FirstHit
            neg_rec[nidx++] = package_[0].time;
            neg_rec[nidx++] = package_[0].channel;

            neg_data_.append_allocated_record(neg_rec);
            neg_indexes_.push_back(item.index);
        }

        // игнориуремые хиты проходим в обратном порядке
        for(int index=0; index<ignoreHitCount; ++index)
        {
            // сохраняем положительную запись
            index_t item = ignore_[index];
            pos_rec[idx++] = item.time;
            pos_rec[idx++] = item.channel;

            // сохраняем отрицательную запись
            int nidx = 0;
            double* neg_rec = neg_data_.allocate_record(neg_chars_.size()
                                                        + 2 // половина C_FirstHit
                                                        );
            if (use_peak_time_)
                neg_rec[nidx++] = ignore_[index].peak;

            neg_rec[nidx++] = ignore_[index].time - package_[0].time;
            REG_MINMAX(aera::C_RelTime, neg_rec[nidx-1]);

            neg_rec[nidx++] = ignore_[index].peak - package_[0].peak;
            REG_MINMAX(aera::C_RelPeakTime, neg_rec[nidx-1]);

            // C_FirstHit
            neg_rec[nidx++] = package_[0].time;
            neg_rec[nidx++] = package_[0].channel;

            neg_data_.append_allocated_record(neg_rec);
            neg_indexes_.push_back(item.index);
        }
    }

    package_.clear();
    ignore_.clear();
}

void Collector::end_of_file()
{
    if (package_.size()) create_event();
}

void Collector::setup_and_reserve(unsigned size)
{
    set_=shared_ptr<hits::hitset>( new hits::hitset() );
    set_->reserve(size);

    pos_chars_ = get_chars();
    pos_coords_count_ = pos_chars_.size();

    if (use_peak_time_)
        pos_chars_.push_back(aera::C_PeakTime);
    pos_chars_.push_back(aera::C_SubHitCount);
    pos_chars_.push_back(aera::C_IgnoredHitCount);
    pos_chars_.push_back(aera::C_FirstSubHit);
    pos_data_.set_typestring(&pos_chars_.front(), pos_chars_.size());

    if (use_peak_time_)
        neg_chars_.push_back(aera::C_PeakTime);
    neg_chars_.push_back(aera::C_RelTime);
    neg_chars_.push_back(aera::C_RelPeakTime);
    neg_chars_.push_back(aera::C_FirstHit);
    neg_data_.set_typestring(&neg_chars_.front(), neg_chars_.size());

    neg_indexes_.reserve(size);
}

data::pslice Collector::make_event_slice(data::pslice s1)
{
    data::pae_slice slice = s1->clone_ae();
    assert( slice );

    slice->set_indexes(pos_indexes_);
    slice->add_layer(&pos_data_);
    slice->add_minmax_map(minmax_map_);
    return slice;
}

data::pslice Collector::make_subhit_slice(data::pslice parent)
{
    data::pae_slice slice = parent->clone_ae();

    slice->set_indexes(neg_indexes_);
    slice->add_layer(&neg_data_);
    slice->add_minmax_map(minmax_map_);

    slice->mark_as_subhit_slice();
    return slice;
}

pclusters Collector::get_clusters() const
{
    return pclusters();
}

bool Collector::check(double const *record)
{
    return set_->get(record)!=0;
}

data::minmax_map Collector::get_minmax_map() const
{
    return minmax_map_;
}

shared_ptr<hits::hitset>          Collector::get_hitset() const
{
    return set_;
}
object_id Collector::tag() const
{
    return tag_;
}

void Collector::setTag(const object_id &tag)
{
    tag_ = tag;
}



}
