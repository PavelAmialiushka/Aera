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
 * @brief ������ �������� ������ �����
 *
 *
 */
class hitset: fastmem::allocator
{
public:

    ~hitset();

    void check();

    /**
     * @brief ������� ���������
     */
    void clear();

    /**
     * @brief ����������� ����� ��� ��������
     */
    void reserve(unsigned n);

    /**
     * @brief ��������� ������
     */
    void append(const hitref &);

    /**
     * @brief ��������� ������, ������������ �� ktne
     */
    void append(data::ae_record);

    /**
     * @brief ������� ���
     */
    void remove(const hitref &);

    /**
     * @brief ������� ��� �� ��������� �� ������
     */
    void remove(data::ae_record);

    /**
     * @brief ����� ���� �� ������
     * ���� ������ �� ������� ������������ 0
     */
    hitref *get(data::ae_record) const;

    /**
     * @brief ���������, ���� �� ������ � ���������
     */
    bool contains(double const*) const;

    /**
     * @brief ���������� �������� �����
     */
    unsigned size() const;

    /**
     * @brief �������������� � ������, ����� ��� ������������
     **/
    std::string to_string() const;

    /**
     * @brief ��������������� ��������� �� ������ - ��� ������������
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
