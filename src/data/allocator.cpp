#include "stdafx.h"

#include "data/allocator.h"

using namespace fastmem;

void *allocator::block::allocate(size_t n)
{
    n = (n+15) & 0xfffffff0;
    if (n+fill_<=chunk_size)
    {
        void *ptr=&buffer_[fill_];
        fill_+=n;
        return ptr;
    }
    else if ( n >= chunk_size && fill_==0)
    {
        buffer_.reset(new char[n]);
        fill_=n;
        return &buffer_[0];
    }
    return NULL;
}


void *allocator::allocate(size_t n)
{    
    while(1)
    {
        if (!blocks_.empty())
        {
            void *ptr = blocks_.back()->allocate(n);
            if (ptr)
                return ptr;
        }

// new_block
        blocks_.push_back(shared_ptr<block>(new block));
    }
}

void allocator::deallocate_all()
{
    blocks_.clear();
}

allocator::~allocator()
{
    deallocate_all();
}

//////////////////////////////////////////////////////////////////////////
