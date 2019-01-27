#pragma once

#include "data/traits.h"
#include "data/allocator.h"
#include "data/aerecord.h"

//////////////////////////////////////////////////////////////////////////

namespace data
{

/**
 * @brief ��������� �� ������.
 *
 * ������ �� ������, ���� �������� �������������.
 * �������� ����������� ����������, ������� ������ � ���� ��������� ��
 * ���������������� �� �������.
 */
class ae_collection : fastmem::allocator
{
public:
    ae_collection();

    /**
     * @brief ������� ���������
     */
    void clear();

    /**
     * @brief set_typestring ������������� ����� �������� �������������
     * @param chars ��������� �� ������ ������
     * @param end ��������� �� ����� ������
     */
    void set_typestring(const aera::chars *chars, unsigned count);

    /**
     * @brief get_typestring
     * @param result ����������
     */
    void get_typestring(std::vector<aera::chars> &result) const;

    /**
     * @brief ��������� � ��������� ������
     * @param ��������� �� ������
     *
     * ��� ���������� ������ ���������� � ���������� ���������� �����
     */
    void copy_append_record(const double * rec);

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
     * @brief ���������� ��������� �� ������ �� �� ������
     * @param no ����� ������
     */
    const double *get_record(unsigned no) const;

    /**
     * @brief ���������� ������, ���������� � ���������
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
