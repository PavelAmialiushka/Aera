#ifndef ___plotscreen__aera_h___
#define ___plotscreen__aera_h___

#include "Buffer.h"
#include "Range.h"

#include "plotcolors.h"

namespace plot
{

class screen
{
    enum { notinitialized=0x7fffffff, };
public:
    screen() :
        buffer_(2, 2),
        oldx_(notinitialized),
        oldy_(notinitialized)
    {
    }

    void set_buffer(unsigned x, unsigned y)
    {
        buffer_=darray(x, y);
    }

    void set_xrange(range r)
    {
        xrange_=r;
    }

    range get_xrange() const
    {
        return xrange_;
    }

    void set_yrange(range r)
    {
        yrange_=r;
    }

    range get_yrange() const
    {
        return yrange_;
    }

    void set_dot(double x, double y, int color)
    {
        set_dot(
            xrange_.map_to(x, buffer_.width()-1),
            yrange_.map_to(y, buffer_.height()-1),
            color);
    }

    void reset()
    {
        oldx_=oldy_=notinitialized;
    }

    void move_to(double x, double y);
    void line_to(double x, double y, int color);

    const darray *get_buffer() const
    {
        return &buffer_;
    }

    void clear()
    {
        std::fill_n(buffer_.get(), buffer_.size(), 0);
    }
private:

    void set_dot(unsigned x, unsigned y, int color);

private:
    range xrange_;
    range yrange_;
    darray buffer_;
    int oldx_, oldy_;
};

}

#endif
