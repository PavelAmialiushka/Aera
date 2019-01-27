#pragma once

#include "utilites/hash.h"

#include "allocator.h"

namespace hits
{

/**
 * @brief �������������� ��������� ���
 *
 * ������������ ��� ��������� � �������� �� ��������� ���
 * ��� ����� �������� ����� ������������� ���� � �����.
 * ��� ����� ������� �������� ���������� ��������� �� ����������
 */
struct hitref {
    double time;
    double chnl;

    hitref()
        : time(0), chnl(0)
    {
    }

    /**
     * @brief ��������������� ����� ae ������ @ref data::ae_record
     * @param data
     */
    hitref(const double *data)
        : time(data[0]), chnl(data[1])
    {
    }

    /**
     * @brief ��������������� ����� ���������������� �������� ���������� ����
     * @param hit_time - ����� ����
     * @param hit_channel - �����
     */
    hitref(double hit_time, double hit_channel)
        : time(hit_time), chnl(hit_channel)
    {
    }

    bool operator<(const hitref &other) const
    {
        return time == other.time
               ? chnl < other.chnl
               : time < other.time;
    }

    bool operator==(const hitref &other) const
    {
        return time == other.time && chnl == other.chnl;
    }

    struct comparer
    {
        bool operator()(const hitref *self, const hitref *other) const
        {
            return *self < *other;
        }
    };

    struct equal
    {
        bool operator()(const hitref *self, const hitref *other) const
        {
            return *self == *other;
        }
    };

    struct hasher
    {
        size_t operator()(const hitref *self) const
        {
            return SuperFastHash(
                       reinterpret_cast<const char *>(&self->time),
                       sizeof(hitref));
        }
    };
};

}
