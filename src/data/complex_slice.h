#pragma once

#include "traits.h"
#include <set>

#include "minmax_map.h"

namespace data
{

class complex_slice : public slice
{
public:
    complex_slice(pslice a, pslice b)
    {
        slices_.push_back(a);
        slices_.push_back(b);

        init();
        sort_by_records();
        init_minmax();
    }
    complex_slice(std::vector<pslice> const& v)
        : slices_(v)
    {
        init();
        sort_by_blocks();
        init_minmax();
    }

    complex_slice(pslice a, std::vector<unsigned> const& indexes)
    {
        slices_.push_back(a);
        index_.reserve(indexes.size());
        for(int ix=0; ix < indexes.size(); ++ix)
        {
            slice_index sl = {0, indexes[ix]};
            index_.push_back(sl);
        }

        init_minmax();
    }

    void init()
    {
        size_t size = 0;
        for(int index=0; index < slices_.size(); ++index)
            size += slices_[index]->size();
        index_.resize(size);
    }

    void sort_by_records()
    {
        assert(slices_.size()==2);
        std::vector<aera::chars> cs;
        cs = slices_[0]->get_chars();
        aera::chars ac = std::count(STL_II(cs), aera::C_PeakTime) >0
                ? aera::C_PeakTime : aera::C_Time;

        cs = slices_[1]->get_chars();
        aera::chars bc = std::count(STL_II(cs), aera::C_PeakTime) >0
                ? aera::C_PeakTime : aera::C_Time;

        unsigned asize=slices_[0]->size();
        unsigned bsize=slices_[1]->size();
        unsigned aindex=0;
        unsigned bindex=0;
        for (unsigned index=0; index<index_.size(); ++index)
        {
            if (aindex >= asize)
            {
select_second:
                    index_[index].layer=1;
                    index_[index].index=bindex++;
            }
            else if (bindex >= bsize)
            {
select_first:
                    index_[index].layer=0;
                    index_[index].index=aindex++;
            }
            else
            {
                const double &x=slices_[0]->get_value(aindex, ac);
                const double &y=slices_[1]->get_value(bindex, bc);

                if (x<=y) goto select_first;
                else      goto select_second;
            }
        }
    }

    void sort_by_blocks()
    {
        int jindex=0;
        for(int layer=0; layer < slices_.size(); ++layer)
        {
            for(int index=0; index < slices_[layer]->size(); ++index)
            {
                index_[jindex].layer = layer;
                index_[jindex].index = index;
                ++jindex;
            }
        }
    }

    void init_minmax()
    {
        for(int index=0; index<slices_.size(); ++index)
        {
            add_minmax_map( slices_[index]->get_minmax_map() );
        }
    }

    pslice clone() const
    {
        return pslice( new complex_slice(*this) );
    }

    unsigned size() const
    {
        return index_.size();
    }

    const double &get_value(unsigned index, aera::chars value=aera::C_Time, int channel=0) const
    {
        const slice_index &pos=index_[index];
        return slices_[pos.layer]->get_value(pos.index, value, channel);
    }

    std::vector<aera::chars> get_chars() const
    {
        std::set<aera::chars> set;
        for(int index=0; index < slices_.size(); ++index)
        {
            std::vector<aera::chars> t=slices_[index]->get_chars();
            set.insert(STL_II(t));
        }

        std::vector<aera::chars> result;
        result.reserve(set.size());
        std::copy(STL_II(set), std::back_inserter(result));
        return result;
    }

    int get_type(unsigned index) const
    {
        const slice_index &pos=index_[index];
        return slices_[pos.layer]->get_type(pos.index);
    }

    // raw
    raw_record get_raw_record(unsigned index) const
    {
        const slice_index &pos=index_[index];
        return slices_[pos.layer]->get_raw_record(pos.index);
    }

    // tdd
    unsigned get_channel_count() const
    {
        unsigned m = 0;
        for(int index=0; index<slices_.size(); ++index)
            m = std::max(m, slices_[index]->get_channel_count());
        return m;
    }

    unsigned get_subhit_count(unsigned index) const
    {
        const slice_index &pos=index_[index];
        return slices_[pos.layer]->get_subhit_count(pos.index);
    }

    hits::hitref get_subhit(unsigned index, unsigned subindex) const
    {
        const slice_index &pos=index_[index];
        return slices_[pos.layer]->get_subhit(pos.index, subindex);
    }

private:

    std::vector<pslice> slices_;

    struct slice_index
    {
        int layer;
        int index;
    };

    std::vector<slice_index> index_;

};

}
