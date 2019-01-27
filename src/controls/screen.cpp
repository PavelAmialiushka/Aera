#include "StdAfx.h"

#include "controls\plot\screen.h"

int colors::by_chanel(int chanel)
{
    static int colors_table[]=
    {

        /*  1 */ 0x00ffff, 0x8080ff, 0xff00ff, 0xffff80, 0x00ff80, 0x808080, 0xff0080, 0x80ff00,
        /*  9 */ 0xff8000, 0x008000, 0x80ffff, 0x0080c4, 0x0080ff, 0x8000ff, 0x80ff80, 0xff8080,
        /* 17 */ 0x008080, 0xC00080, 0xffff00, 0x00ff00, 0x808000, 0xff0000, 0xff80ff, 0x8080c4,
        /* 25 */ 0xff00c4, 0xffff60, 0x00ff60, 0x808060, 0xff0060, 0x80ffc4, 0xff80c4, 0x00ffc4,
        /* 33 */ 0x8000c4, 0x80ff60, 0xff8060, 0x008060, 0x00c4ff, 0x8060ff, 0xffc480, 0x00c480,
        /* 41 */ 0x809080, 0x80c400, 0xff6000, 0x006000, 0x80c4ff, 0xff60ff, 0x0060ff, 0x80c480,
        /* 49 */ 0xff6080, 0x006080, 0xffc400, 0x00c400, 0x806000, 0x00c4c4, 0x8060c4, 0xffc460,
        /* 57 */ 0x00c460, 0x806060, 0x80c4c4, 0xff60c4, 0x0060c4, 0x80c460, 0xff6060, 0x006060,

//				0x6080ff, 0xc400ff, 0xc4ff80, 0x608080,
// 0xc40080, 0x60ff00, 0xc48000, 0x600000, 0x60ffff, 0xc480ff, 0x6000ff, 0x60ff80,
// 0xc48080, 0x600080, 0xc4ff00, 0x608000, 0xc40000, 0x6080c4, 0xc400c4, 0xc4ff60,
// 0x608060, 0xc40060, 0x60ffc4, 0xc480c4, 0x6000c4, 0x60ff60, 0xc48060, 0x600060,
// 0x6060ff, 0xc4c480, 0x606080, 0x60c400, 0xc46000, 0x60c4ff, 0xc460ff, 0x60c480,
// 0xc46080, 0xc4c400, 0x606000, 0x6060c4, 0xc4c460, 0x606060, 0x60c4c4, 0xc460c4,
// 0x60c460, 0xc46060,

//0x530071, 0x0070e0, 0x002ca9, 0xff00ff, 0x10ec6e, 0x91ab34, 0x00e321, 0xff312c,
//0x00919a, 0x00aa14, 0xff85ff, 0x12764c, 0x72d600, 0xff36ff, 0x006500, 0xf70000,
//0xe107af, 0x50ffff, 0xff0063, 0x6a2310, 0x6f8300, 0x00d1ff, 0xb3887f, 0x88b9af,
//0xff7e00, 0xffafce, 0x966fde, 0x7e00eb, 0xd94770, 0xffce00, 0x8cf898, 0xffec4e,
//
//0x530071, 0x0070e1, 0x002ca9, 0xff00ff, 0x10ec6e, 0x91ab34, 0x00e321, 0xff312c,
//0x00919a, 0x00aa14, 0xff85ff, 0x12764c, 0x72d600, 0xff36ff, 0x006500, 0xf70000,
//0xe107af, 0x50ffff, 0xff0063, 0x6a2310, 0x6f8300, 0x00d1ff, 0xb3887f, 0x88b9af,
//0xff7e00, 0xffafce, 0x966fde, 0x7e00eb, 0xd94770, 0xffce00, 0x8cf898, 0xffec4e,

        0xFFFFFF,
        0x00FF00,
        0x0000FF,
    };

    static int first=1; if (first)
    {
        first=0;

        for (unsigned index=0; index<64; ++index)
            assert( 0==std::count(
                        colors_table+index+1,
                        colors_table+64,
                        colors_table[index]) );
    }

    return colors_table[ (chanel-1) % SIZEOF(colors_table) ];
}

int colors::channel_by_color(int color)
{
    for (unsigned channel=1; channel<mixture; ++channel)
    {
        if (by_chanel(channel)==color)
            return channel;
    }
    return 0;
}


namespace plot
{

void screen::move_to(double x, double y)
{
    oldx_=xrange_.map_to(x, buffer_.width()-1);
    oldy_=yrange_.map_to(y, buffer_.height()-1);
}


void screen::line_to(double x, double y, int color)
{
    try
    {
        int xx=xrange_.map_to(x, buffer_.width()-1);
        int yy=yrange_.map_to(y, buffer_.height()-1);
        if (oldx_==notinitialized)
        {
            move_to(x, y);
        }
        int dx=xx-oldx_;
        int dy=yy-oldy_;
        if (abs(dx)>abs(dy))
        {
            int ii=xx < oldx_ ? +1 : -1;
            for (int ix=xx; ix!=oldx_; ix+=ii)
            {
                unsigned y=scale((int)ix, (int)xx, (int)oldx_, (int)yy, (int)oldy_);
                if (ix >=0 && ix < (int)buffer_.width() && y < buffer_.height())
                    set_dot(ix, y, color);
            }
        }
        else
        {
            int ii=yy < oldy_ ? +1 : -1;
            for (int iy=yy; iy!=oldy_; iy+=ii)
            {
                unsigned x=scale((int)iy, (int)yy, (int)oldy_, (int)xx, (int)oldx_);
                if (x < buffer_.width() && iy >=0 && iy < (int)buffer_.height())
                    set_dot(x, iy, color);
            }
        }
        oldx_=xx, oldy_=yy;
    } LogExceptionPath("screen::line_to");
}

void screen::set_dot(unsigned x, unsigned y, int color)
{
    int &ref=buffer_.at(x, y);

    if (ref==0) ref=color;
	else if (color==colors::selected) ref=colors::selected;
	else if (ref<colors::mixture && ref!=color) ref=colors::mixture;
}


}
