#include "stdafx.h"

#include "LinearSensorWidget.h"

#include "utilites/foreach.hpp"


using namespace location;

CLinearSensorWidget::CLinearSensorWidget()
    : loc_(0)
    , circular_(0)
{
}

std::pair<double, double> CLinearSensorWidget::GetRange()
{
    double minimum, maximum;
    widen_range_begin(minimum, maximum);

    if (!sensors_.empty())
    {
        minimum = sensors_.begin()->second;
        maximum = sensors_.begin()->second;

        typedef std::map<int, double>::value_type value_type;
        foreach(value_type value, sensors_)
        {
            widen_range(minimum, maximum,
                        unit_value(value.second, loc_->get_unit()).get_distance(m));
        }
    }

    if (circular_)
    {
        widen_range(minimum, maximum, circular_);
    }

    widen_range_end(minimum, maximum);
    return std::make_pair(minimum, maximum);
}

LRESULT CLinearSensorWidget::OnPaint(HDC hdc)
{
    CPaintDC paintDC(m_hWnd);
    CDCHandle dc0(hdc ? hdc : paintDC);

    if (!loc_) return 0;

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
    frame.DeflateRect(15, 10, 15, 10);
    int v_center = frame.top + frame.Height()/2 + 20;

    double maximum, minimum;
    tie(minimum, maximum) = GetRange();

    typedef std::map<int, double>::value_type value_type;
    foreach(value_type value, sensors_)
    {
        double abscissa=unit_value(value.second, loc_->get_unit()).get_distance(m);

        int x=scale(abscissa, minimum, maximum, frame.left, frame.right);

        dc.MoveTo(x-10, v_center);
        dc.LineTo(x+10, v_center);

        dc.MoveTo(x, v_center-10);
        dc.LineTo(x, v_center+10);

        CRect text_rect(x, v_center-40, x, v_center-40);
        std::string text=strlib::strf("%d", value.first);

        CSize sz;
        dc.GetTextExtent(text.c_str(), text.size(), &sz);
        text_rect.InflateRect(sz.cx/2, 0, sz.cx/2, sz.cy);

        dc.DrawText(text.c_str(), text.size(), text_rect, DT_VCENTER|DT_CENTER);

        text_rect.InflateRect(2, 2);
        dc.MoveTo(text_rect.TopLeft());
        dc.LineTo(text_rect.right, text_rect.top);
        dc.LineTo(text_rect.BottomRight());
        dc.LineTo(text_rect.left, text_rect.bottom);
        dc.LineTo(text_rect.TopLeft());
    }


    dc.SelectPen( oldp );

    //////////////////////////////////////////////////////////////////////////

    dc0.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

    return 0;
}

void CLinearSensorWidget::setLocation(LinearSetup * s)
{
    loc_ = s;

    sensors_=loc_->get_sensors();

    circular_ = loc_->get_circular_length().get_distance(m);

    Invalidate();
}
