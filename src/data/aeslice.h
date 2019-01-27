#pragma once

#include "slice.h"
#include "aecollection.h"

#include "hitset.h"
#include "minmax_map.h"

#include "utilites/foreach.hpp"

namespace data
{

//////////////////////////////////////////////////////////////////////////


struct ae_layer
{
    /**
     * use_index_ = true - если используем индексы, то массив индексов предоставляет порядок и
     * количество адресуемых запией
     * use_index_ = false - если не используем индексы, то доступ используется
     * непосредственно к хранимым в хранилище значениям
     */
    bool                  use_index;

    /**
     * массив используемых индексов
     */
    std::vector<unsigned> indexes;

    /**
     * коллекция, из которой извлекаются данные
     */
    const ae_collection *master;

};


/**
 * @brief реализует доступ к ае данным.
 *
 * сами данные должны храниться в @ref ae_collection, доступ к ним происходит
 * либо напрямую либо через набор индексов, что позволяет сортировать и
 * отфильтровывать данные.
 */
class ae_slice : public slice
{
public:

    ae_slice()
        : contains_subhits_(0)
    {
    }

    /**
     * @brief срезка создается на основе коллекции
     * @param coll коллекция
     * @param indexes список индексов
     */
    ae_slice(const ae_collection *coll,
             const std::vector<unsigned> &indexes=std::vector<unsigned>()
             )
        : contains_subhits_(0)

    {
        layers_.resize(1);
        layers_[0].use_index = !indexes.empty();
        layers_[0].indexes = indexes;
        layers_[0].master = coll;

        init_map();
    }

    /**
     * данные текущей срезки создаются на основе указанной
     */
    void set_source(pslice parent_slice)
    {
        ae_slice *parent=dynamic_cast<ae_slice *>(parent_slice.get());
        set_source(parent);
    }

    void set_source(ae_slice const* parent)
    {
        assert(parent);
        layers_=parent->layers_;

        minmax_map_.merge(parent->get_minmax_map());

        if (parent->contains_subhits_)
            mark_as_subhit_slice();

        init_map();
    }

    /**
     * данные текущей срезки создаются на основе указанной,
     * с учетом массива индексов.
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
            // последний индекс не ссылается за пределы родительского диапазона
            assert(indexes[indexes.size()-1] < size());
        }

        foreach(ae_layer &la, layers_)
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

    void add_layer(ae_collection* col, const std::vector<unsigned>& indexes = std::vector<unsigned>())
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

        layers_.push_back( ae_layer() );
        layers_.back().master = col;
        layers_.back().use_index = indexes.size() > 0;
        layers_.back().indexes = indexes;

        init_map();
    }

    /**
     * заполняем вектор @ref ae_slice::map_, т.е. создаем
     * ускорительные индексы переменных используемых внутри срезки.
     * в массив попадают только используемые типы переменных,
     * а при необходимости также данные локации
     */
    void init_map()
    {
        map_index idx; idx.layer = -1;
        map_.resize(256, idx);

        std::vector<aera::chars> common;
        foreach(ae_layer &la, layers_)
        {
            ++idx.layer;

            la.master->get_typestring(common);
            for (unsigned index=0; index<common.size(); ++index)
            {
                idx.index = index;
                map_[ static_cast<int>(common[index]) ]=idx;
            }
        }
    }

    /**
     * создаем точную копию срезки
     */
    pslice clone() const
    {
        ae_slice* newby= new ae_slice;
        newby->set_source( this );
        return pslice(newby);
    }

    // overriden slice functions

    virtual unsigned get_subhit_count(unsigned index) const
    {
        return static_cast<unsigned>(
                    get_value(index, aera::C_SubHitCount)
                    + get_value(index, aera::C_IgnoredHitCount));
    }

    virtual hits::hitref get_subhit(unsigned index, unsigned subindex) const
    {
        const double* subhit = &get_value(index, aera::C_FirstSubHit);
        return hits::hitref(subhit + subindex * 2);
    }

    /**
     * возвращает имя переменной по ее типу
     */
    std::string get_unit_name(aera::chars c) const
    {
        return aera::traits::get_unit_name(c);
    }

    /**
     * количество записей, содержащихся в срезке
     */
    unsigned size() const
    {
        return layers_.empty() ? 0 :
               layers_[0].use_index ? layers_[0].indexes.size() :
               layers_[0].master->size();
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
     * @brief возвращает значение переменной хранимой в срезке по указателю на запись
     * @param index номер записи
     * @param typ тип запрашиваемой переменной
     * @param channe    l (для ae_slice не используется)
     */
    const double &get_value(unsigned index, aera::chars typ, int channel=0) const
    {
        const map_index idx = map_[ static_cast<int>(typ) ];
        if (idx.layer >= layers_.size())
        {
            return nan();
        }

        ae_layer const &la = layers_[idx.layer];        
        if (la.use_index)
        {
            assert(index<la.indexes.size());
            index = la.indexes[index];
        }

        const double* record
                =la.master->get_record(index);
        return record[idx.index];
    }

    /**
     * возвращает тип срезки
     */
    int get_type(unsigned index) const
    {
        return contains_subhits_ ? AE_SUBHITS : AE;
    }

    void mark_as_subhit_slice()
    {
        contains_subhits_ = true;
    }

    /**
     * @brief возвращает список переменных по которым есть данные
     */
    std::vector<aera::chars> get_chars() const
    {
        std::vector<aera::chars> result, temp;
        foreach(ae_layer const &la, layers_)
        {
            la.master->get_typestring(temp);
            result.insert( result.end(), STL_II(temp) );
        }
        return result;
    }

protected:
    /**
      * иногда создается срезках для вторичных хитов
      */
    bool contains_subhits_;

    /**
     * ускоритель доступа, хранит индексы типов переменных
     */
    struct map_index
    {
        unsigned layer:16;
        unsigned index:16;
    };
    std::vector<map_index> map_;
    std::vector<ae_layer>  layers_;
};

inline pae_slice clone_ae(pslice slice)
{
    return boost::dynamic_pointer_cast<data::ae_slice>(slice->clone());
}

inline pae_slice slice::clone_ae() const
{
    return data::clone_ae(clone());
}

//////////////////////////////////////////////////////////////////////////

}
