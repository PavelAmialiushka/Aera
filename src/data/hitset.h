#pragma once

#include "data/aerecord.h"
#include "data/allocator.h"

#include "data/hitref.h"

#include "utilites/hash.h"
#include "utilites/hash_set.h"

//////////////////////////////////////////////////////////////////////////

namespace hits
{

class hitset;
typedef boost::shared_ptr<hitset> phitset;
/**
 * @brief Хранит перечень группы хитов
 *
 *
 */
class hitset: fastmem::allocator
{
public:

    ~hitset();

    void check();

    /**
     * @brief очищает хранилище
     */
    void clear();

    /**
     * @brief резервирует место для хранения
     */
    void reserve(unsigned n);

    /**
     * @brief добавляет запись
     */
    void append(const hitref &);

    /**
     * @brief добавляет запись, конструирует на ktne
     */
    void append(data::ae_record);

    /**
     * @brief удаляет хит
     */
    void remove(const hitref &);

    /**
     * @brief удаляет хит по указателю на запись
     */
    void remove(data::ae_record);

    /**
     * @brief поиск хита по записи
     * если запись не найдена возвращается 0
     */
    hitref *get(data::ae_record) const;

    /**
     * @brief проверяет, есть ли запись в множестве
     */
    bool contains(double const*) const;

    /**
     * @brief количество хранимых хитов
     */
    unsigned size() const;

    /**
     * @brief преобразование в строку, нужно для сериализации
     **/
    std::string to_string() const;

    /**
     * @brief восстанавливает контейнер из строки - для сериализации
     */
    void make_from_string(std::string);

    /**
     * @brief serialization
     */
    void serialization(serl::archiver &);

private:

    typedef hash_set<hitref*, hitref::hasher, hitref::equal> set_t;
    set_t set_;
};

}
