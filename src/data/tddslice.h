#pragma once

#include <vector>

#include "slice.h"
#include "tddcollection.h"

namespace data
{


struct tdd_layer
{
    /**
     * use_index = true - ���� ���������� �������, �� ������ �������� ������������� ������� �
     * ���������� ���������� ������
     * use_index = false - ���� �� ���������� �������, �� ������ ������������
     * ��������������� � �������� � ��������� ���������
     */
    bool                  use_index;

    /**
     * ������ ������������ ��������
     */
    std::vector<unsigned> indexes;

    /**
     * ���������, �� ������� ����������� ������
     */
    const tdd_collection *master;

    /**
     * ���������� �������, ������ �������� (�������� channel_map_[n]),
     * �� ������� ����� ���������� ��� ������� � ������ n. �����
     * record[n] - ����� ��������� �� ������ ������ ������ n.
     */
    std::vector<unsigned> channel_map;

    /**
     * ��������� ������ @ref tdd_slice::channel_map_, �.�. �������
     * ������������� ������� ���������� ������������ ������ ������.
     */
    void init_channels()
    {
        if (channel_map.empty() && master->size())
        {
            // ����� ������ ������ ��� �������
            const double *record=master->get_record(0);

            unsigned common_size=master->common_types_.size();
            unsigned channel_size=master->channel_types_.size();

            // ������� �����
            channel_map.resize(master->get_channel_count(), -1);

            // ���������� �������� ����� ������ ������� ������
            for (unsigned index=0; index<master->get_channel_count(); ++index)
            {
                unsigned pos=common_size + index*channel_size;
                channel_map.at( index )=pos;
            }
        }
    }
};


/**
 * @brief ��������� ������ � tdd ������.
 *
 * ���� ������ ������ ��������� � @ref tdd_collection, ������ � ��� ����������
 * ���� �������� ���� �� ������ ��������.
 *
 * tdd_slice ������ ���������� ���� �����. ����� ���������� � ���������� �� �������.
 * ������ ���������� ������������ �� ������ ������, � ������ �� ������ ������ � ��
 * ������ ������.
 *
 */
class tdd_slice : public slice
{
public:

    tdd_slice()
    {
    }

    /**
     * @brief ������ ��������� �� ������ ���������
     * @param coll ���������
     * @param indexes ������ ��������
     */
    tdd_slice(const tdd_collection *coll,
              const std::vector<unsigned> &indexes=std::vector<unsigned>())
    {
        layers_.resize(1);
        layers_[0].master = coll;
        layers_[0].use_index = !indexes.empty();
        layers_[0].indexes = indexes;

        init_map();
    }

    /**
     * ������ ������� ������ ��������� �� ������ ���������
     */
    void set_source(pslice parent_slice)
    {
        tdd_slice* parent=dynamic_cast<tdd_slice*>(parent_slice.get());
        set_source(parent);
    }

    void set_source(tdd_slice const* parent)
    {
        assert(parent);
        layers_=parent->layers_;
        minmax_map_.merge(parent->get_minmax_map());

        init_map();
    }

    /**
     * ������ ������� ������ ��������� �� ������ ���������
     * � ������ ������� ��������.
     */
    void set_indexed_source(pslice parent_slice, const std::vector<unsigned> &indexes)
    {
        set_source(parent_slice);
        set_indexes(indexes);
    }

    void set_indexes(const std::vector<unsigned> &indexes)
    {
        if (indexes.size())
        {
            // ��������� ������ �� ��������� �� ������� ������������� ���������
            assert(indexes[indexes.size()-1] < size());
        }

        foreach(tdd_layer& la, layers_)
        {
            la.use_index=true;
            bool modify = !la.indexes.empty();

            std::vector<unsigned> new_la(indexes.size());
            for (unsigned j=0; j<indexes.size(); ++j)
            {
                if (modify) new_la[j] = la.indexes[ indexes[j] ];
                else        new_la[j] = indexes[j];
                assert(new_la[j] < la.master->size());
            }
            std::swap(new_la, la.indexes);
        }
    }

    void add_layer(tdd_collection* col, const std::vector<unsigned>& indexes = std::vector<unsigned>())
    {
        assert(col);

#ifndef NDEBUG
        if (layers_.size())
            if (indexes.size()) assert(indexes.size() == size());
            else            assert(col->size() == size());
        if (indexes.size()) assert(indexes[indexes.size()-1] < col->size());
#endif

        for(unsigned index=0; index < indexes.size(); ++index)
        {
            assert(indexes[index] < col->size() );
        }

        layers_.push_back( tdd_layer() );
        layers_.back().master = col;
        layers_.back().use_index = indexes.size() > 0;
        layers_.back().indexes = indexes;

        init_map();
    }

    /**
     * ��������� ������ @ref tdd_slice::map_, �.�. �������
     * ������������� ������� ���������� ������������ ������ ������.
     */
    void init_map()
    {
        map_index idx; idx.layer = -1;
        map_.resize(256, idx);

        std::vector<aera::chars> common;
        std::vector<aera::chars> channels;
        foreach(tdd_layer &la, layers_)
        {   ++idx.layer;

            idx.iscommon = 1;
            la.master->get_common_types(common);
            for (unsigned index=0; index<common.size(); ++index)
            {
                idx.index = index;
                map_[ static_cast<int>(common[index]) ] = idx;
            }

            idx.iscommon = 0;
            la.master->get_channel_types(channels);
            for (unsigned index=0; index<channels.size(); ++index)
            {
                idx.index = index;
                map_[ static_cast<int>(channels[index]) ] = idx;
            }

            la.init_channels();
        }
    }

    /**
     * ������� ������ ����� ������
     */
    pslice clone() const
    {
        tdd_slice *newby = new tdd_slice;
        newby->set_source(this);
        return pslice(newby);
    }

    // overriden slice functions

    /**
     * ���������� �������, ������������ � ������
     */
    unsigned size() const
    {
        if (layers_.empty())
            return 0;

        tdd_layer const &la = layers_[0];
        if (la.use_index)
            return la.indexes.size();
        else
            return la.master->size();
    }


    static const double& nan()
    {
        static double nan[] = {
            std::numeric_limits<double>::quiet_NaN(),
            std::numeric_limits<double>::quiet_NaN()
            };
        return nan[0];
    }

    /**
     * @brief ���������� �������� ���������� �������� � ������ �� ��������� �� ������
     * @param record ��������� �� ������ ������
     * @param typ ��� ������������� ����������
     * @param channel ����� ������ (��� ��������� ����������)
     *
     */
    const double &get_value(unsigned index, aera::chars typ, int channel) const
    {
        const map_index idx = map_[ static_cast<int>(typ) ];
        if (idx.layer >= layers_.size())
        {
            return nan();
        }

        tdd_layer const &la = layers_[idx.layer];
        if (la.use_index)
        {
            assert(index<la.indexes.size());
            index = la.indexes[index];
        }

        const double* record
                =la.master->get_record(index);
        if (idx.iscommon)
            return record[ idx.index ];
        else
        {
            unsigned channel_index=la.channel_map.at(channel);
            return record[ channel_index + idx.index ];
        }
    }

    /**
     * ���������� ��� ������
     */
    int get_type(unsigned index) const
    {
        return TDD;
    }

    /**
     * ���������� ����� ������� �� ������� ���� ������
     */
    unsigned get_channel_count() const
    {
        if (layers_.empty())
            return 0;

        return layers_[0].master->get_channel_count();
    }

    /**
     * @brief ���������� ������ ���������� �� ������� ���� ������
     */
    std::vector<aera::chars> get_chars() const
    {
        std::vector<aera::chars> result, temp;
        foreach(tdd_layer const &la, layers_)
        {
            la.master->get_common_types(temp);
            result.insert(result.end(), STL_II(temp));

            la.master->get_channel_types(temp);
            result.insert(result.end(), STL_II(temp));
        }
        return result;
    }

private:
    /**
     * ���������� �������, ������ ������� ����� ����������, ��������
     * �������� ���� C_Duration ����� ������ �� �������� map_[C_Duration]
     * - ���� �������� � map_ �������� >=0 �� ��� ������������ ��� ������
     * - ���� <0 �� ������������ ������ ��������� ������
     * - ���� == -1, �� ������
     */
    struct map_index
    {
        unsigned layer:8;
        unsigned iscommon:8;
        unsigned index:16;
    };
    std::vector<map_index> map_;

    std::vector<tdd_layer> layers_;
};

inline ptdd_slice slice::clone_tdd() const
{
    return boost::dynamic_pointer_cast<data::tdd_slice>(clone());
}

}
