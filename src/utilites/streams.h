#pragma once

namespace fs
{


struct stream_page
{
    unsigned    base_offset;
    const char* base;
    const char* begin;
    const char* end;

    unsigned size() const { return end - begin; }
};

MAKE_SHARED(streambuffer);
class streambuffer
{
public:
    virtual ~streambuffer(){};

public:
    virtual unsigned size() const=0;

    virtual bool get_data(unsigned pos, stream_page&)=0;
    virtual bool get_more_data(unsigned size, stream_page&)=0;
};

class istream
{
    typedef void (istream::*bool_type)() const;
    void this_type_does_not_support_comparisons() const {}

    bool okstate_;
    stream_page page_;
    shared_ptr<streambuffer> buffer_;

    bool get_more_data(unsigned size);

public:
    istream(std::string path);
    istream(std::vector<char> const & vector);
    istream(const char* buffer_, unsigned size);

    unsigned size() const;
    unsigned offset() const;

    operator bool_type() const {
        return okstate_
                ? &istream::this_type_does_not_support_comparisons
                : 0;
    }
    bool operator!() const  {
        return !okstate_;
    }
    bool isok() const   {
        return okstate_;
    }

    void seek_back(unsigned pos);
    void seek(unsigned pos);

    const char* read_buf(unsigned size)
    {
        if (reserve(size))
        {
            const char* result = page_.begin;
            skip(size);
            return result;
        }
        return 0;
    }

    fs::istream extract(unsigned size)
    {
        if (reserve(size))
        {
            return fs::istream(page_.begin, size);
        }

        return fs::istream(0, 0);
    }

    const char* reserve(unsigned size)
    {
        if (!okstate_)
            return 0;

        unsigned curr = page_.end - page_.begin;
        if (curr < size)
        {
            if (!get_more_data(size))
            {
                okstate_ = false;
                return 0;
            }
        }

        return page_.begin;
    }

    std::string read_str(unsigned size)
    {
        if (!reserve(size))
            return std::string();

        std::string result(page_.begin, page_.begin+size);
        page_.begin += size;
        return result;
    }

    void skip(unsigned size)
    {
        if (!reserve(size))
            return;

        page_.begin += size;
    }

    template<class T>
    T read(const T &)
    {
        unsigned size = sizeof(T);
        if (!reserve(size))
            return T();

        T result;
        memmove(&result, page_.begin, size);
        page_.begin += size;
        return result;
    }

    unsigned char read_8() {
        return read(unsigned char()); }

    unsigned short read_16() {
        return read(unsigned short()); }

    unsigned int read_32() {
        return read(unsigned int()); }

    __int64 read_64() {
        return read(__int64()); }

    double read_double() {
        return read(double()); }

    double read_48()
    {
        double lo=read(unsigned __int32());
        double hi=read(unsigned __int16());
        return lo+hi*4294967296;
    }

    double read_24()
    {
        double lo=read(unsigned __int16());
        double hi=read(unsigned __int8());
        return lo+hi*65536;
    }

};

}

