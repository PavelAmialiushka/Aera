#pragma once

#include "traits.h"
#include "rawrecord.h"
#include "hitref.h"

#include "minmax_map.h"

namespace data
{

class slice;
typedef shared_ptr<slice> pslice;

class ae_slice;
typedef shared_ptr<ae_slice> pae_slice;

class tdd_slice;
typedef shared_ptr<tdd_slice> ptdd_slice;


/**
 * срезка может быть одной из четырех видов, ае данные,
 * сырые данные и ТДД данные.
 */
enum
{
    AE =1,
    RAW=2,
    TDD=3,
    AE_SUBHITS=4,
};

/**
 * @brief обеспечивает доступ к выборке данных.
 *
 * абстрактный класс
 */

class slice
{
public:

    DEFINE_ERROR(not_implemented, "not_implemented");

    virtual pslice clone() const
    {
        throw not_implemented(HERE);
    }

    pae_slice clone_ae() const;
    ptdd_slice clone_tdd() const;

    pslice merge(pslice b) const;
    pslice clone_indexed(std::vector<unsigned>const&indexes) const;
    static pslice merge(std::vector<pslice> const&);

    virtual ~slice() {}

    /**
     * @brief количество хранимых данных
     */
    virtual unsigned size() const=0;

    /**
     * @brief возвращает значение переменной по индексу
     * @param index - номер записи
     * @param name - тип запрашиваемой переменной
     * @param channel - канал (для тдд данных)

     * Работает для всех всех записей.
     */
    virtual const double &get_value(unsigned index, aera::chars name=aera::C_Time, int channel=0) const=0;

    virtual int get_type(unsigned index) const
    {
        throw not_implemented(HERE);
    }

    // fat interface

    // ae only

    virtual unsigned get_subhit_count(unsigned) const
    {
        return 0;
    }

    virtual hits::hitref get_subhit(unsigned, unsigned) const
    {
        throw not_implemented(HERE);
    }

    // ae & td
    virtual std::vector<aera::chars> get_chars() const=0;

    virtual std::string get_unit_name(aera::chars c) const
    {
        return aera::traits::get_unit_name(c);
    }

    // raw
    virtual raw_record get_raw_record(unsigned index) const
    {
        throw not_implemented(HERE);
    }

    // tdd
    virtual unsigned get_channel_count() const
    {
        return 0;
    }

    // minmax
    void add_minmax_map(minmax_map const& v)
    {
        minmax_map_.merge(v);
    }

    minmax_map const& get_minmax_map() const
    {
        return minmax_map_;
    }

    lohi_value get_minmax_limits(aera::chars var) const
    {
        const lohi_value * val = minmax_map_.find(var);
        if (val) return *val;
        return lohi_value();
    }


protected:
    minmax_map minmax_map_;
};

}
