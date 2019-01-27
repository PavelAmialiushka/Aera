#include "stdafx.h"

#include "VesselSensorWidget.h"

#include "data/traits.h"
#include "PlanarSensorWidget.h"

#include "utilites/foreach.hpp"

CVesselSensorWidget::CVesselSensorWidget()
{
}

struct ItemShape
{
    double height;
    double width;
    CRect  rc;
};

inline int sgn(double h)
{
    return h > 0 ? 1 :
           h < 0 ? -1 : 0;
}

struct h2v
{
    bool enable;
    h2v(bool enable)
        : enable(enable)
    {
    }

    CPoint decode(int x, int y)
    {
        if (!enable) return CPoint(x, y);
        return CPoint(y, x);
    }

    CRect encode(CRect rc)
    {
        if (!enable) return rc;
        return CRect(rc.top, rc.right, rc.bottom, rc.left);
    }
};


class BaseShaper
{
protected:
    double scale;
    int    axe;
    std::vector<ItemShape> items;
    VesselStructure formula;
    const VesselStructure *structure;
    const VesselSensors* sensors;
    CRect  rc;

    bool   horizontal;
    double total_height;
    double max_width;

public:
    BaseShaper(VesselStructure f,  const VesselSensors* sensors)
    : formula(f)
    , structure(&formula)
    , sensors(sensors)
    {
    }

    virtual ItemShape getShape(VesselPart p)
    {
        ItemShape s;
        s.width = std::max(p.diameter1, p.diameter2);
        s.height = p.height;
        return s;
    }

    void build()
    {
        total_height = 0;
        max_width = 0;
        foreach(VesselPart part, formula.get_parts())
        {
            ItemShape item = getShape(part);
            items.push_back(item);

            total_height += item.height;
            max_width = std::max(max_width, item.width);
        }
    }

    void draw(CDCHandle dc)
    {
        assert(formula.size() == items.size());
        for(unsigned index=0; index<formula.size(); ++index)
        {
            drawItem(dc, items[index].rc, index, formula[index], horizontal);
            drawSensorOfPart(dc, items[index].rc, index, formula[index], horizontal);
        }
    }

    virtual void drawSensorOfPart(CDCHandle dc,
                          CRect rc,
                          int part_index, VesselPart const& part,
                          bool horizontal,
                          VesselCoords const& crds);


    virtual void drawItem(CDCHandle dc,
                          CRect rc,
                          int part_index, VesselPart const& part,
                          bool horizontal);


    void drawSensor(CDCHandle dc, CPoint point, int number, bool otherside=false)
    {
        CRect text_rect(point, point);
        std::string text = strlib::strf("%d", number);

        CSize sz;
        dc.GetTextExtent(text.c_str(), text.size(), &sz);
        text_rect.InflateRect(sz.cx/2, sz.cy/2, sz.cx/2, sz.cy/2);

        if (!otherside)
        {
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
        } else {
            // сдвгаем вправо вверх там будет текст
            text_rect.OffsetRect(9, -9);
            dc.DrawText(text.c_str(), text.size(), text_rect, DT_VCENTER|DT_CENTER);
            CPoint lt(text_rect.left, text_rect.bottom);

            // коробочка
            text_rect.SetRect(point, point);
            text_rect.InflateRect(3, 3);
            dc.MoveTo(text_rect.TopLeft());
            dc.LineTo(text_rect.right, text_rect.top);
            dc.LineTo(text_rect.BottomRight());
            dc.LineTo(text_rect.left, text_rect.bottom);
            dc.LineTo(text_rect.TopLeft());

            // крестик
            text_rect.InflateRect(5, 5);
            dc.MoveTo(text_rect.right, text_rect.CenterPoint().y);
            dc.LineTo(text_rect.left, text_rect.CenterPoint().y);
            dc.MoveTo(text_rect.CenterPoint().x, text_rect.top);
            dc.LineTo(text_rect.CenterPoint().x, text_rect.bottom);
        }
    }

    void drawSensorOfPart(CDCHandle dc,
                          CRect rc,
                          int part_index, VesselPart const& part,
                          bool horizontal)
    {
        foreach(VesselCoords const& crds, *sensors)
        {
            if (crds.lxy.element_index == part_index)
            {
                drawSensorOfPart(dc, rc, part_index, part, horizontal, crds);
            }
        }
    }


    void adjustItemsRect()
    {
        int height = rc.Height();
        int bottom = rc.bottom;
        foreach(ItemShape& item, items)
        {
            int top = bottom - sgn(height) * int(scale * item.height);
            int width = int(scale * item.width);

            item.rc = CRect(axe - width/2, top, axe + width/2, bottom);
            bottom = top;
        }
    }

    void adjust1(CRect frame,
                 bool h, double s, int a,
                 int rc_height)
    {
        h2v hv(horizontal);

        scale = s;
        horizontal = h;
        axe = a;

        int bottom = rc_height > 0
                ? frame.left + int(rc_height/2 + total_height*scale/2)
                : frame.right + int(rc_height/2 - total_height*scale/2);
        int width = int(max_width*scale);
        int height = sgn(rc_height)*int(total_height*scale);

        rc.SetRect(axe - width/2, bottom - height,
                   axe + width/2, bottom);
        adjustItemsRect();
    }

    friend void adjustRects(CRect frame, bool horizontal,
                            BaseShaper& a, BaseShaper& b)
    {
        h2v hv(horizontal);
        frame = hv.encode(frame);

        a.build();
        b.build();

        int height = frame.Height();
        int width = frame.Width();        

        double w = 1.05 * (a.max_width + b.max_width);
        double h = std::max(a.total_height, b.total_height);

        double scale = std::min(h ? fabs(height) / h : 0,
                                w ? fabs(width) / w : 0);
        int extra_width = width - int(w * scale);

        int axea = frame.left + int(a.max_width*scale/2) + extra_width / 3;
        int axeb = frame.right - int(b.max_width*scale/2) - extra_width / 3;

        a.adjust1(frame, horizontal, scale, axea, height);
        b.adjust1(frame, horizontal, scale, axeb, height);
    }
};

class NetShaper : public BaseShaper
{
public:
    NetShaper(VesselStructure f, const VesselSensors* sensors)
        : BaseShaper(f, sensors)
    {
    }

    virtual ItemShape getShape(VesselPart p)
    {
        ItemShape s;
        s.width = M_PI * std::max(p.diameter1, p.diameter2);
        s.height = p.get_height_y();
        return s;
    }

    void drawSensorOfPart(CDCHandle dc,
                          CRect rc,
                          int part_index, VesselPart const& part,
                          bool horizontal,
                          VesselCoords const& crds);

    virtual void drawItem(CDCHandle dc,
                          CRect rc,
                          int part_index, VesselPart const& part,
                          bool horizontal);
};

LRESULT CVesselSensorWidget::OnPaint(HDC hdc)
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
    frame.DeflateRect(10, 10, 10, 10);

    BaseShaper shaper1(formula_, &sensors_);
    NetShaper shaper2(formula_, &sensors_);
    adjustRects(frame, !formula_.get_is_vertical(), shaper1, shaper2);

    shaper1.draw((HDC)dc);
    shaper2.draw((HDC)dc);

    dc.SelectPen( oldp );

    //////////////////////////////////////////////////////////////////////////

    dc0.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

    return 0;
}

void CVesselSensorWidget::setLocation(VesselSetup *s)
{
    loc_ = s;
    sensors_ = loc_->get_sensors();
    formula_ = loc_->get_vessel();
    formula_.compile();

    Invalidate();
}

void BaseShaper::drawItem(CDCHandle dc, CRect rc, int part_index, const VesselPart &part, bool horizontal)
{
    h2v hv(horizontal);

    switch(part.type)
    {
    case VesselPart::Cylinder:
        dc.MoveTo(hv.decode(rc.left, rc.top));
        dc.LineTo(hv.decode(rc.right, rc.top));
        dc.LineTo(hv.decode(rc.right, rc.bottom));
        dc.LineTo(hv.decode(rc.left, rc.bottom));
        dc.LineTo(hv.decode(rc.left, rc.top));

        break;
    case VesselPart::Ellipsoid:
    case VesselPart::Sphere:
    {
        CPoint center;
        if (!part.is_upper_bottom())
        {
            dc.MoveTo(hv.decode(rc.left, rc.top));
            dc.LineTo(hv.decode(rc.right, rc.top));
            center = CPoint(rc.left/2 + rc.right/2, rc.top);
        }
        else
        {
            dc.MoveTo(hv.decode(rc.left, rc.bottom));
            dc.LineTo(hv.decode(rc.right, rc.bottom));
            center = CPoint(rc.left/2 + rc.right/2, rc.bottom);
        }

        int dx = rc.Width()/2;
        int dy = sgn(rc.Height()) * (part.type == VesselPart::Sphere ? dx : dx/2);
        int dx_max = part.diameter1
                ? part.diameter2 * dx / part.diameter1
                : 0;

        for(double alpha = 0; alpha <= M_PI; alpha+=M_PI/128)
        {
            double cx = dx * cos(alpha);
            double cy = dy * sin(alpha);

            if (fabs(cx) < dx_max)
                continue;

            if (part.is_upper_bottom()) cy = -cy;
            dc.LineTo(hv.decode(center.x + cx, center.y + cy));
        }
        break;
    }
    case VesselPart::Cone:
    {
        CPoint center;
        int dx, dy;

        if (!part.is_upper_bottom())
        {
            dc.MoveTo(hv.decode(rc.left, rc.top));
            dc.LineTo(hv.decode(rc.right, rc.top));
            center = CPoint(rc.left/2 + rc.right/2, rc.top);
        }
        else
        {
            dc.MoveTo(hv.decode(rc.left, rc.bottom));
            dc.LineTo(hv.decode(rc.right, rc.bottom));
            center = CPoint(rc.left/2 + rc.right/2, rc.bottom);
        }
        dx = rc.Width()/2;
        dy = rc.Height();

        if (part.is_upper_bottom()) dy = -dy;

        int dxy2 = part.diameter1
                ? part.diameter2 * dx / part.diameter1
                : 0;

        dc.LineTo(hv.decode(center.x + dxy2, center.y + dy));
        dc.LineTo(hv.decode(center.x - dxy2, center.y + dy));
        dc.LineTo(hv.decode(center.x - dx,  center.y));
    }
    }
}

void NetShaper::drawItem(CDCHandle dc, CRect rc, int part_index, const VesselPart &part, bool horizontal)
{
    h2v hv(horizontal);

    PVElement element = part.get_element();
    if (!element) return;

    switch(part.type)
    {
    case VesselPart::Cylinder:
        dc.MoveTo(hv.decode(rc.left, rc.top));
        dc.LineTo(hv.decode(rc.right, rc.top));
        dc.LineTo(hv.decode(rc.right, rc.bottom));
        dc.LineTo(hv.decode(rc.left, rc.bottom));
        dc.LineTo(hv.decode(rc.left, rc.top));

        break;
    case VesselPart::Flat:
    case VesselPart::Cone:
    case VesselPart::Sphere:
    case VesselPart::Ellipsoid:
    {

        std::vector<lxy_coords> coords;
        create_net_map(element, coords);

        bool firstly = true;
        foreach(lxy_coords const& lxy, coords)
        {
            int y = rc.bottom;
            int x = rc.right;

            int dx = ::scale(lxy.x, 0., element->get_width(),
                             0, rc.Width());
            int dy = ::scale(lxy.y, 0., element->height_y,
                             0, rc.Height());

            CPoint p = hv.decode(x-dx, y-dy);

            if (firstly)
            {
                dc.MoveTo(p);
                firstly = false;
            }
            else
                dc.LineTo(p);
        }
    }   break;
    }
}

void BaseShaper::drawSensorOfPart(CDCHandle dc,
                                  CRect rc,
                                  int part_index, const VesselPart &part,
                                  bool horizontal,
                                  const VesselCoords &crds)
{
    h2v hv(horizontal);

    lfiy_coords lfiy = structure->to_lfiy(crds.global, part_index);
    double y = lfiy.y;
    double fi = lfiy.fi;
    double r0 = part.diameter1 / 2;
    double r = part.get_element()->get_radius_by_ly(y);

    double x = r * sin(fi);
    bool otherside = cos(fi) < 0;

    int xx = ::scale(x, -r0, +r0, rc.left, rc.right);
    int yy = ::scale(y, .0, part.get_height_y(), rc.bottom, rc.top);

    drawSensor(dc, hv.decode(xx, yy), crds.number, otherside);
}

void NetShaper::drawSensorOfPart(CDCHandle dc,
                                 CRect rc,
                                 int part_index, const VesselPart &part,
                                 bool horizontal,
                                 const VesselCoords &crds)
{
    h2v hv(horizontal);

    lfiy_coords lfiy = structure->to_lfiy(crds.global, part_index);
    double y = lfiy.y;
    double x = structure->to_nx(crds.global);

    double width0 = M_PI * part.diameter1;

    int xx = ::scale(x, .0, width0, rc.left, rc.right);
    int yy = ::scale(y, .0, part.get_height_y(), rc.bottom, rc.top);

    drawSensor(dc, hv.decode(xx, yy), crds.number);
}
