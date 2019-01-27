#pragma once

#include "data/traits.h"
#include "data/allocator.h"
#include "data/aerecord.h"

//////////////////////////////////////////////////////////////////////////

namespace data
{

/**
 * @brief Хранилище аэ данных.
 *
 * хранит аэ записи, типы хранимых характеристик.
 * является примитивным хранилищем, которое хранит в себе указатели на
 * последовательные ае события.
 */
class ae_collection : fastmem::allocator
{
public:
    ae_collection();

    /**
     * @brief очищает хранилище
     */
    void clear();

    /**
     * @brief set_typestring устанавливает набор хранимых характеристик
     * @param chars указатель на начало буфера
     * @param end указатель на конец буфера
     */
    void set_typestring(const aera::chars *chars, unsigned count);

    /**
     * @brief get_typestring
     * @param result возвращает
     */
    void get_typestring(std::vector<aera::chars> &result) const;

    /**
     * @brief добавляет в коллекцию запись
     * @param указатель на запись
     *
     * при добавлении данные копируются в внутренний выделенный буфер
     */
    void copy_append_record(const double * rec);

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
     * @brief возвращает указатель на запись по ее номеру
     * @param no номер записи
     */
    const double *get_record(unsigned no) const;

    /**
     * @brief количество данных, хранящихся в хранилище
     */
    unsigned size() const;

    bool sorted() const;

private:
    std::vector<aera::chars> chars_;
    std::vector<double *>    data_;
    bool                     sorted_;
    double                   prevtime_;
};

}
