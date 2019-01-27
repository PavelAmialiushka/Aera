#include "stdafx.h"

#include "PlotMaker.h"
#include "BitmapMaker.h"
#include "plot/screen.h"
#include "utilites/Localizator.h"

BitmapMaker::BitmapMaker(plot_maker *model, bool inverted)
{
    config_.dimensions_ = CSize(2, 2);
    config_.inverted_ = inverted;

    set_parent(model);
    restart();
}

BitmapMaker::~BitmapMaker()
{
    detach_host();
}


void BitmapMaker::set_dimensions(CSize sz)
{
    if (sz.cx>0 && sz.cy>0)
    {
        config_.dimensions_=sz;
        restart_using(config_);
    }
}

CSize BitmapMaker::get_dimensions() const
{
    return config_.dimensions_;
}


void BitmapMaker::set_visibility(bool new_state)
{
    host::set_visibility(new_state);
}


struct BitmapMaker::processor
        : process::processor_t<
            BitmapMaker::config,            
            bitmap_result,
            base_plot_result>
{
    bool process()
    {
        int cx = config->dimensions_.cx;
        int cy = config->dimensions_.cy;

        CDC desktop_dc=::GetWindowDC( 0 );
        result->bitmap_=shared_ptr<CBitmap>( new CBitmap(
                                         ::CreateCompatibleBitmap( desktop_dc, cx, cy)) );
        BITMAPINFO bi=
        {
            {
                sizeof(BITMAPINFOHEADER), cx, cy,
                1, 32, BI_RGB
            }
        };
        int *data;

        CDC dc( CreateCompatibleDC( desktop_dc ) );

        result->bitmap_->Attach( ::CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void **)(&data), 0, 0) );
        vector2<int> vector(data, cx, cy*cx);

        if (cx > 1 && cy > 1 )
        {
            dc.SelectBitmap(*result->bitmap_);
            dc.FillSolidRect(CRect(CPoint(0,0), CSize(cx, cy)),
                             config->inverted_ ? clrNegBkColor : clrBkColor);
        }

        plot::darray const *source_array = source->screen->get_buffer();
        assert(source_array->height() > 1 && source_array->width() > 1);

        if (cx < 1 || cy < 1)
            return true;

        static int _selected_color = colors::by_chanel(colors::selected);
        static int _k=0;
        if (_k==0)
        {
            char *tx=reinterpret_cast<char *>(&_selected_color);
            std::swap(tx[0], tx[2]);
            _k=1;
        }


        for(unsigned idy=0; idy< source_array->height(); ++idy)
        {
            const int *row=&source_array->at(0, idy);
            int qy=scale(idy, 0u, source_array->height()-1, 0, (int)cy-1);
            for (unsigned idx=0; idx<source_array->width(); ++idx)
            {
                if (int ix=row[idx])
                {
                    int qx=scale(idx, 0u, source_array->width()-1, 0, (int)cx-1);

                    bool noalign=qx < (int)cx-1 && qy < (int)cy-1;

                    COLORREF c=colors::by_chanel(ix);

                    if (config->inverted_)
                    {
                        switch (c)
                        {
                        case clrBkColor:    c=clrNegBkColor; break;
                        case clrNegBkColor: c=clrBkColor; break;
                        }
                        if (ix>=colors::mixture) c=clrBkColor;
                    }

                    char *tx=reinterpret_cast<char *>(&c);
                    std::swap(tx[0], tx[2]);

                    int &p0 = vector[qy][qx];    if (p0 != _selected_color) p0 = c;

                    if (noalign)
                    {
                        int &p1 = vector[qy+1][qx]; if (p1!=_selected_color) p1=c;
                        int &p2 = vector[qy][qx+1]; if (p2!=_selected_color) p2=c;
                        int &p3 = vector[qy+1][qx+1]; if (p3!=_selected_color) p3=c;
                    }
                }
            }
            if (!check_status(idy, source_array->height()))
                return false;
        }
        return true;
    }
};

process::processor *BitmapMaker::create_processor()
{
    return new BitmapMaker::processor;
}

pbitmap_result BitmapMaker::create_default_model()
{
    pbitmap_result result( new bitmap_result );

    int cx = config_.dimensions_.cx;
    int cy = config_.dimensions_.cy;

    CDC desktop_dc=::GetWindowDC( 0 );
    result->bitmap_=shared_ptr<CBitmap>( new CBitmap(
                                     ::CreateCompatibleBitmap( desktop_dc, cx, cy)) );

    result->screen.reset( new plot::screen());
    result->screen->set_buffer(buffer_width, buffer_height);

    result->screen.reset( new plot::screen() );

    return result;
}

pbitmap_result BitmapMaker::get_result()
{
    return process::get_result<bitmap_result>(this);
}

void BitmapMaker::restart()
{
    restart_using(config_);
}

void BitmapMaker::setup(process::hostsetup & setup)
{
    setup.name = _ls("bitmap#Bitmap preparation");
    setup.weight = 0.01;
}

void bitmap_result::inherite_from(process::prslt r)
{
    if (pbase_plot_result result= boost::dynamic_pointer_cast<base_plot_result>(r))
    {
        screen = result->screen;
        timemarks = result->timemarks;
    }
}
