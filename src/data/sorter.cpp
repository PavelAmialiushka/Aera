#include "stdafx.h"

#include "sorter.h"
#include "loader.h"

#include "blank_slice.h"
#include "aeslice.h"
#include "tddslice.h"
#include "utilites/heap.h"
#include "utilites/Localizator.h"

namespace nodes
{

sorter::sorter()
{
}

sorter::~sorter()
{
    detach_host();
}

void sorter::setup(process::hostsetup & setup)
{
    setup.name = _ls("sorter#Data sorting");
    setup.need_config = false;
    setup.weight = 1.0;
}

struct sorter::processor
        : process::processor_t<
                    process::config,
                    nodes::result,
                    nodes::loader_result>
{
    static
    bool compare_indexes(pslice slice, int a, int b)
    {
        return slice->get_value(a) > slice->get_value(b);
    }

    virtual bool process()
    {
        if (source->sorted)
            return true;

        // ����������� ������
        pslice slice = source->ae;

        // ��������� ������ ��� ����������
        std::vector<unsigned>   heap, array;
        heap.reserve(slice->size());
        array.reserve(slice->size());

        int ssize = slice -> size();

        boost::function2<bool, int, int> compare_records
                = bind(&sorter::processor::compare_indexes,
                       slice, _1, _2);

        // ��������� �� ������, ������� ���� ��������
        // � ������
        double curr_time, top_time = 0;
        unsigned first = 0;
        unsigned end = 0;

        // first - ������ ����
        // end - ����� ����

        for(int index=0; index < ssize; ++index)
        {
            // �������� � ����
            heap.push_back(index);
            utils::push_heap(heap.begin() + first,
                             heap.begin() + ++end,
                             compare_records);

            curr_time = slice->get_value(index);

            // ���� ���������� ������� �� ��������� ��������
            // �� ��������� �������� �� ����
            while (curr_time > top_time + 10
                   && first != end)
            {
                utils::pop_heap(heap.begin() + first,
                                heap.begin() + end--,
                                compare_records);
                int last = heap.back();
                heap.pop_back();

                top_time = slice->get_value( last );
                array.push_back( last );
            }

            if (!check_status(index, ssize, 0, 2))
                return false;
        }

        // �������� �������� �� ����
        while (first != end)
        {
            utils::pop_heap(heap.begin() + first,
                            heap.begin() + end--,
                            compare_records);
            int last = heap.back();
            heap.pop_back();

            top_time = slice->get_value( last );
            array.push_back( last );
        }


        data::ae_slice *as=new data::ae_slice;
        as->set_indexed_source(source->ae, array);
        result->ae.reset( as );

        return 1;
    }
};

process::processor *sorter::create_processor()
{
    return new sorter::processor;
}

}
