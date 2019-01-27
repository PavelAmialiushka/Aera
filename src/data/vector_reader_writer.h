#ifndef VECTOR_READER_WRITER_H
#define VECTOR_READER_WRITER_H

#include "data/Traits.h"

namespace data
{

//////////////////////////////////////////////////////////////////////////

namespace traits { enum _floatSelector { flt=20, }; }
enum _i48selector { __int48 };
enum _i24selector { __int24 };

//////////////////////////////////////////////////////////////////////////

class  vector_reader
{
private:

    DEFINE_ERROR(exhausted, "vector buffer have been exhausted");

public:

    vector_reader(const std::vector<char> &vec) : vector(vec), ptr(0) {}
    template<class I>
    const I &read(const I &)
    {
        assert(vector.size()-ptr>=sizeof(I));
        const I *p=reinterpret_cast<const I *>( ptr+&vector[0] );
        ptr+=sizeof(I);
        return *p;
    }

    void skip(size_t x)
    {
        ptr += x;
    }

    double read(_i48selector)
    {
        double lo=read(unsigned __int32());
        double hi=read(unsigned __int16());
        return lo+hi*4294967296;
    }

    double read(_i24selector)
    {
        double lo=read(unsigned __int16());
        double hi=read(unsigned __int8());
        return lo+hi*65536;
    }

    const char *pointer()
    {
        return &vector[0]+ptr;
    }
    const char *end()
    {
        return &vector[0]+vector.size();
    }

private:
    const std::vector<char> &vector;
    size_t ptr;
};

//////////////////////////////////////////////////////////////////////////

class vector_writer
{
public:
    vector_writer(std::vector<char> &vector) : vector_(vector) {}

    template<class I> void write(const I &i)
    {
        vector_.resize(vector_.size()+sizeof(I));
        memcpy(&vector_[vector_.size()-sizeof(I)], &i, sizeof(I));
    }

    void write(std::size_t sz, void *buf)
    {
        vector_.resize(vector_.size()+sz);
        memcpy(&vector_[vector_.size()-sz], buf, sz);
    }

    void write(double value, unsigned sz)
    {
        switch (sz)
        {
        case 1:
            write((unsigned __int8)value);
            break;
        case 2:
            write((unsigned __int16)value);
            break;
        case 3:
            write(static_cast<unsigned __int16>(((unsigned __int32)value) & 0xFFFF));
            write(static_cast<unsigned __int8>(((unsigned __int32)value) >> 16 ));
            break;
        case 4:
            write((unsigned __int32)value);
            break;
        case 6:
            write(static_cast<unsigned __int32>(((__int64)value) & 0xFFFFFFFF));
            write(static_cast<unsigned __int16>(((__int64)value) >> 32 ));
            break;
        case traits::flt:
            write(static_cast<float>(value));
            break;
        }
    }

private:
    std::vector<char> &vector_;
};


}

#endif // VECTOR_READER_WRITER_H
