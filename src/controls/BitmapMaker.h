#ifndef BITMAPMAKER_H
#define BITMAPMAKER_H

#include "PlotMaker.h"

#include "data/processes.h"

enum
{
    clrSRBorderAH   =0x0000FF,
    clrSRBorderAL   =0xFFFFFF,
    clrSRBorderIH   =0x0000BF,
    clrSRBorderIL   =0x7F7F7F,

    clrSRHatch      =0xFF0000,

    clrTimemark     =0x007F7F,
};


enum
{
    clrBkColor      =0x000000,
    clrNegBkColor   =0xFFFFFF,

    clrFrColor	    =0xFFFFFF,
    clrNegFrColor   =0x000000,

    clrTitles       =0x000000,
    clrActiveTitles =0x0000FF,

    clrMarks        =0x000000,
    clrActiveMarks  =0x0000FF,
};



template<class T> class vector2
{
    unsigned width_;
    unsigned size_;
    T *array_;
public:
    vector2()
        : array_(0), size_(0), width_(0)
    {
    }

    vector2(T *array, unsigned width, unsigned size=0)
        : width_(width), array_(array), size_(size/width)
    {}

    vector2(std::vector<double> &array, unsigned width)
        : width_(width), array_(&array[0]), size_(array.size()/width) {}

    T *operator[](unsigned index) const
    {
        return array_+index*width_;
    }
    unsigned size() const
    {
        return size_;
    }
    unsigned width() const
    {
        return width_;
    }
};

class plot_maker;

MAKE_SHARED_STRUCT(bitmap_result);
struct bitmap_result : base_plot_result
{
    shared_ptr<CBitmap> bitmap_;

    void inherite_from(process::prslt);

    static pbitmap_result create_default();
};

class BitmapMaker
        : public process::host_t<bitmap_result>
{
public:
    BitmapMaker(plot_maker *model, bool inverted = false);
    ~BitmapMaker();

    void set_dimensions(CSize sz);
    CSize get_dimensions() const;

    void set_visibility(bool new_state);

    void setup(process::hostsetup&);
    process::processor* create_processor();

    pbitmap_result create_default_model();
    pbitmap_result get_result();

    void restart();

private:

    struct processor;

    struct config : process::config
    {
        CSize         dimensions_;
        bool          inverted_;
    } config_;

};


#endif // BITMAPMAKER_H
