#ifndef ___plotbuffer__aera_h___
#define ___plotbuffer__aera_h___

namespace plot
{

class darray
{
public:
    DEFINE_ERROR(out_of_range, "darray::out_of_range");

    darray(unsigned width, unsigned height)
        : width_(width), height_(height), buffer_(new int[width *height])
    {
        std::fill_n(buffer_.get(), size(), 0);
    }

    darray(const darray &other)
        : width_(other.width()), height_(other.height()), buffer_(new int[other.size()])
    {
        std::copy(other.get(), other.get()+other.size(), buffer_.get());
    }

    darray &operator=(darray ar)
    {
        swap(ar);
        return *this;
    }

    void swap(darray &ar)
    {
        buffer_.swap(ar.buffer_);
        std::swap(width_, ar.width_);
        std::swap(height_, ar.height_);
    }

    unsigned size() const
    {
        return width_*height_;
    }

    int *get() const
    {
        return buffer_.get();
    }

    int &at(unsigned x, unsigned y)
    {
        return buffer_[index(x,y)];
    }

    const int &at(unsigned x, unsigned y) const
    {
        return buffer_[index(x, y)];
    }

    unsigned width() const
    {
        return width_;
    }
    unsigned height() const
    {
        return height_;
    }

private:

    int index(unsigned x, unsigned y) const
    {
        assert( x < width_ && y < height_);
        return y*width_+x;
    }

private:

    unsigned width_;
    unsigned height_;

    boost::scoped_array<int> buffer_;
};

////////////////////////////////////////////////////////////////////

class buffer
{
public:

    DEFINE_ERROR(out_of_range, "plot::buffer: out_of_range");
    enum { width=100 };

public:

    buffer() : array_(new int[size()])
    {
        clear();
    }

    void clear()
    {
        ::memset(array_.get(), 0, size());
    }

    int &at(int a, int b)
    {
        return array_.get()[index(a, b)];
    }

    int *get_array() const
    {
        return array_.get();
    }

    unsigned size() const
    {
        return width*width;
    }

private:

    int index(int x, int y) const
    {
        debug::Assert<out_of_range> _1(x<width && x>=0, HERE);
        debug::Assert<out_of_range> _2(y<width && y>=0, HERE);
        return x+y*width;
    }

private:

    std::auto_ptr<int> array_;
};

}

#endif
