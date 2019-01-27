#include "stdafx.h"

#include "data/traits.h"
#include "PlanarSensorWidget.h"

#include "utilites/foreach.hpp"

CPlanarSensorWidget::CPlanarSensorWidget()
{
}

LRESULT CPlanarSensorWidget::OnPaint(HDC hdc)
{
    CPaintDC paintDC(m_hWnd);
    CDCHandle dc0(hdc ? hdc : paintDC);

    CRect rc; GetClientRect(rc);

    CDC dc( ::CreateCompatibleDC(dc0) );
    CBitmap bitmap( ::CreateCompatibleBitmap(dc0, rc.Width(), rc.Height()) );
    dc.SelectBitmap( bitmap );

    //////////////////////////////////////////////////////////////////////////

    CPen pen( CreatePen(PS_SOLID, 1, 0xFFFFFF) );
    CPenHandle oldp=dc.SelectPen( pen );

    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(0xFFFFFF);
    dc.FillRect(rc, CBrush(CreateSolidBrush(0)));
    dc.DrawEdge(rc, EDGE_SUNKEN, BF_RECT);

    CRect frame=rc;
    frame.DeflateRect(15, 10, 15, 25);

    double  minx, miny, maxx, maxy;
    loc_->get_coordinate_range(aera::C_CoordX, minx, maxx);
    loc_->get_coordinate_range(aera::C_CoordY, miny, maxy);

    double scalex = (maxx - minx) / frame.Width();
    double scaley = (maxy - miny) / frame.Height();

    if (scalex < scaley)
    {   //
        double dx = maxx - minx;
        double s = scaley / scalex - 1;

        minx -= dx * s / 2;
        maxx += dx * s / 2;
    } else
    {
        double dy = maxy - miny;
        double s = scalex / scaley - 1;

        miny -= dy * s / 2;
        maxy += dy * s / 2;
    }

    typedef std::map<int, planar_coords>::value_type value_type;
    foreach(value_type value, sensors_)
    {
        double abscissa=unit_value(value.second.x, loc_->get_unit()).get_distance(m);
        double ordinate=unit_value(value.second.y, loc_->get_unit()).get_distance(m);

        int x=scale(abscissa, minx, maxx, frame.left, frame.right);
        int y=scale(ordinate, miny, maxy, frame.bottom, frame.top);

        CRect text_rect(x, y, x, y);
        std::string text=strlib::strf("%d", value.first);

        CSize sz;
        dc.GetTextExtent(text.c_str(), text.size(), &sz);
        text_rect.InflateRect(sz.cx/2, 0, sz.cx/2, sz.cy);

        dc.DrawText(text.c_str(), text.size(), text_rect, DT_VCENTER|DT_CENTER);

        // квадратик
        text_rect.InflateRect(2, 2);
        dc.MoveTo(text_rect.TopLeft());
        dc.LineTo(text_rect.right, text_rect.top);
        dc.LineTo(text_rect.BottomRight());
        dc.LineTo(text_rect.left, text_rect.bottom);
        dc.LineTo(text_rect.TopLeft());

        // крестик
        dc.MoveTo(text_rect.right, text_rect.CenterPoint().y);
        dc.LineTo(text_rect.right + 5, text_rect.CenterPoint().y);
        dc.MoveTo(text_rect.left, text_rect.CenterPoint().y);
        dc.LineTo(text_rect.left - 5, text_rect.CenterPoint().y);
        dc.MoveTo(text_rect.CenterPoint().x, text_rect.top);
        dc.LineTo(text_rect.CenterPoint().x, text_rect.top-5);
        dc.MoveTo(text_rect.CenterPoint().x, text_rect.bottom);
        dc.LineTo(text_rect.CenterPoint().x, text_rect.bottom+5);
    }

    if (horizontal_wrap_)
    {
        int y1=scale(.0, miny, maxy, frame.top, frame.bottom);
        int y2=scale(horizontal_wrap_, miny, maxy, frame.top, frame.bottom);

        dc.MoveTo(frame.left, y1);
        dc.LineTo(frame.right, y1);

        dc.MoveTo(frame.left, y2);
        dc.LineTo(frame.right, y2);
    }
    if (vertical_wrap_)
    {
        int x1=scale(.0, minx, maxx, frame.left, frame.right);
        int x2=scale(vertical_wrap_, minx, maxx, frame.left, frame.right);

        dc.MoveTo(x1, frame.top);
        dc.LineTo(x1, frame.bottom);

        dc.MoveTo(x2, frame.top);
        dc.LineTo(x2, frame.bottom);
    }

    dc.SelectPen( oldp );

    //////////////////////////////////////////////////////////////////////////

    dc0.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

    return 0;
}

void CPlanarSensorWidget::setLocation(PlanarSetup *s)
{
    loc_ = s;

    std::map<int, planar_coords> sensors = loc_->get_sensors();
    sensors_=sensors;

    double vel = loc_->get_velocity().get(m_s);
    horizontal_wrap_ = M_PI * loc_->get_horizontal_diam().get_distance(vel);
    vertical_wrap_ =M_PI * loc_->get_vertical_diam().get_distance(vel);
    Invalidate();
}


