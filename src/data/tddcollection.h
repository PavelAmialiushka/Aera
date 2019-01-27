#pragma once

#include "data/traits.h"
#include "data/allocator.h"
#include "data/tddrecord.h"

namespace data
{

class tdd_slice;

//////////////////////////////////////////////////////////////////////////

/**
 * @brief Хранилище тдд данных.
 *
 * хранит тдд записи, и типы хранимых характеристик.
 * является примитивным хранилищем, которое хранит в себе указатели на
 * последовательные тдд события.
 */
class tdd_collection : fastmem::allocator
{
public:
    tdd_collection();

    /**
     * @brief очищает хранилище
     */
    void clear();

    /**
     * @brief устанавливает общие переменные
     * @param chars начало буфера с перечнем общих переменных
     * @param count количество общих переменных
     */
    void set_common_types(const aera::chars *chars, unsigned count);
    /**
     * @brief возвращает хранимые общие переменные
     */
    void get_common_types(std::vector<aera::chars> &result) const;

    /**
     * @brief устанавливает переменные каналов
     * @param chars начало буфера с перечнем переменных каналов
     * @param count количество переменных по каналам
     */
    void set_channel_types(const aera::chars *chars, unsigned count);

    /**
     * @brief возвращает перечень переменных по каналам
     */
    void get_channel_types(std::vector<aera::chars> &result) const;

    /**
     * @brief добавляет в коллекцию запись
     * @param указатель на запись
     *
     * при добавлении данные копируются в внутренний выделенный буфер
     */
    void copy_append_record(const double * rec, unsigned count);

    /**
     * @brief выделяет указатеьл для очередной записи
     * @param sz размер, выраженный в количестве double
     * @return указатель на выделенную область
     */
    double *allocate_record(unsigned sz);

    /**
     * @brief append_allocated_record
     * @param rec указатель на выделенную область
     *
     * ВНИМАНИЕ. не осуществляется проверка, что указатель
     * выделен данным классом (функцией allocate_record)
     * если это не так, то не гарантируется что время жизни указателя
     * будет соответствовать времени жизни объекта ae_collection.
     */
    void append_allocated_record(double * rec);

    /**
     * @brief количество хранимых данных
     */
    unsigned size() const;

    /**
     * @brief возвращает указатель на tdd запись по ее номеру
     */
    tdd_record get_record(unsigned) const;

    void set_channel_count(unsigned c);
    unsigned get_channel_count() const;

private:
    std::vector<aera::chars> common_types_;
    std::vector<aera::chars> channel_types_;
    std::vector<double *>    data_;
    unsigned                 channel_count_;


    friend class tdd_slice;
    friend struct tdd_layer;
};
//////////////////////////////////////////////////////////////////////////

}
