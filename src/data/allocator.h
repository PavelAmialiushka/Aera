//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "data_error.h"

namespace fastmem
{

/**
 * @brief предоставляет интерфейс для быстрого выделения кусков памяти.
 *
 * Память захватывается большими кусками и освобождается только одновременно
 * либо при разрушении объекта allocator, либо при вызове @ref allocator::deallocate_all
 */

class allocator
{
public:

    DEFINE_ERROR_OF_TYPE(data::format_error, size_too_big, "The size of block is too big");

public:

    allocator() {}
    ~allocator();

    void *allocate(size_t sz);
    void deallocate_all();

private:

    struct block
    {
        enum { chunk_size=300000, };
        block() : fill_(0), buffer_(new char[chunk_size]) {}
        void *allocate(size_t n);
    private:
        int fill_;
        boost::scoped_array<char> buffer_;
    };

    std::deque<shared_ptr<block> >  blocks_;

};

template<class T> void *alloc(allocator *all, T* = 0)
{
    void *pointer=all->allocate(sizeof(T));
    return pointer;
}

//
// hit_ref* =new (fastmem::alloc<hit_ref>(allocator)) hit_ref();
// hit_ref* =FASTMEM_ALLOC(allocator, hit_ref, (record));
//

#define FASTMEM_ALLOC(allocator, type, constructor) \
  new (fastmem::alloc<type>(allocator)) type ## constructor

}

