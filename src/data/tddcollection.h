#pragma once

#include "data/traits.h"
#include "data/allocator.h"
#include "data/tddrecord.h"

namespace data
{

class tdd_slice;

//////////////////////////////////////////////////////////////////////////

/**
 * @brief ��������� ��� ������.
 *
 * ������ ��� ������, � ���� �������� �������������.
 * �������� ����������� ����������, ������� ������ � ���� ��������� ��
 * ���������������� ��� �������.
 */
class tdd_collection : fastmem::allocator
{
public:
    tdd_collection();

    /**
     * @brief ������� ���������
     */
    void clear();

    /**
     * @brief ������������� ����� ����������
     * @param chars ������ ������ � �������� ����� ����������
     * @param count ���������� ����� ����������
     */
    void set_common_types(const aera::chars *chars, unsigned count);
    /**
     * @brief ���������� �������� ����� ����������
     */
    void get_common_types(std::vector<aera::chars> &result) const;

    /**
     * @brief ������������� ���������� �������
     * @param chars ������ ������ � �������� ���������� �������
     * @param count ���������� ���������� �� �������
     */
    void set_channel_types(const aera::chars *chars, unsigned count);

    /**
     * @brief ���������� �������� ���������� �� �������
     */
    void get_channel_types(std::vector<aera::chars> &result) const;

    /**
     * @brief ��������� � ��������� ������
     * @param ��������� �� ������
     *
     * ��� ���������� ������ ���������� � ���������� ���������� �����
     */
    void copy_append_record(const double * rec, unsigned count);

    /**
     * @brief �������� ��������� ��� ��������� ������
     * @param sz ������, ���������� � ���������� double
     * @return ��������� �� ���������� �������
     */
    double *allocate_record(unsigned sz);

    /**
     * @brief append_allocated_record
     * @param rec ��������� �� ���������� �������
     *
     * ��������. �� �������������� ��������, ��� ���������
     * ������� ������ ������� (�������� allocate_record)
     * ���� ��� �� ���, �� �� ������������� ��� ����� ����� ���������
     * ����� ��������������� ������� ����� ������� ae_collection.
     */
    void append_allocated_record(double * rec);

    /**
     * @brief ���������� �������� ������
     */
    unsigned size() const;

    /**
     * @brief ���������� ��������� �� tdd ������ �� �� ������
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
