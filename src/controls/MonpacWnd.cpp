#include "stdafx.h"

#include "monpacWnd.h"

#include "data/monpac.h"
#include "data/channel.h"

#include "utilites/Localizator.h"

#include "BitmapMaker.h"

using namespace monpac;

static std::string cvt(int x)
{
    char buffer [30]; itoa(x, buffer, 10);
    return buffer;
}

CMonpacWindow::CMonpacWindow()
{
    xrange_=plot::range(1, 10, true);
    yrange_=plot::range(10, 10000, true);
    show_axes_captions_=true;
}

void CMonpacWindow::OnFinalMessage(HWND)
{
    delete this;
}

LRESULT	CMonpacWindow::OnCreate(LPCREATESTRUCT)
{
    tooltip_.Create(m_hWnd);
    tooltip_.AddTool(CToolInfo(TTF_TRACK|TTF_ABSOLUTE|TTF_TRANSPARENT , m_hWnd));

    caption_=_ls("MPZIP#MONPAC intensivity chart");
    ycaption_=_ls("Severity#Severity");
    xcaption_=_ls("MPHI#Hist. index");
    return 0;
}

void CMonpacWindow::DrawLines(CDCHandle dc)
{
    //////////////////////////////////////////////////////////////////////////
    const double A=2.042;   // Ax ** 0.65
    const double B=2.847;
    const double C=3.205;
    const double D=xrange_.get_right(); // right edge

    const double a=26.51;   // 210.6 * ax ** 0.45
    const double b=49.47;
    const double c=74.72;
    const double d0=167.35;
    const double d=287.69;
    const double e=593.55;
    const double f=yrange_.get_right(); // top
    //////////////////////////////////////////////////////////////////////////

    CFont font; font.CreatePointFont(120, "Tahoma");
    CFontHandle oldf=dc.SelectFont(font);
    dc.SetTextColor(copyclip_ ? clrNegFrColor : clrFrColor);
    dc.SetBkMode(TRANSPARENT);

    CRect reca;
    reca=CRect(map_to(0, 0), map_to(1, b), map_to(0, A), map_to(1, a));
    dc.DrawText("A", 1, reca, DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

    reca=CRect(map_to(0, A), map_to(1, b), map_to(0, B), map_to(1, a));
    dc.DrawText("B", 1, reca, DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

    reca=CRect(map_to(0, B), map_to(1, c), map_to(0, D), map_to(1, a));
    dc.DrawText("C", 1, reca, DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

    reca=CRect(map_to(0, B), map_to(1, d), map_to(0, D), map_to(1, c));
    dc.DrawText("D", 1, reca, DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

    reca=CRect(map_to(0, B), map_to(1, f), map_to(0, D), map_to(1, d));
    dc.DrawText("E", 1, reca, DT_CENTER|DT_VCENTER|DT_NOCLIP|DT_SINGLELINE);

    dc.SelectFont(oldf);

    CPen pen( ::CreatePen(PS_SOLID, 1, copyclip_ ? clrBkColor : clrNegBkColor) );
    CPenHandle oldp=dc.SelectPen(pen);

    std::vector<CPoint> points;
    points.push_back( CPoint(map_to(0, 0), map_to(1, a)) );
    points.push_back( CPoint(map_to(0, D), map_to(1, a)) );
    dc.Polyline(&points[0], points.size());

    // B
    points.clear();
    points.push_back( CPoint(map_to(0, 0), map_to(1, b)) );
    points.push_back( CPoint(map_to(0, A), map_to(1, b)) );
    points.push_back( CPoint(map_to(0, A), map_to(1, a)) );
    dc.Polyline(&points[0], points.size());

    // C
    points.clear();
    points.push_back( CPoint(map_to(0, 0), map_to(1, c)) );
    points.push_back( CPoint(map_to(0, A), map_to(1, c)) );
    points.push_back( CPoint(map_to(0, B), map_to(1, b)) );
    points.push_back( CPoint(map_to(0, B), map_to(1, a)) );
    dc.Polyline(&points[0], points.size());

    // D
    points.clear();
    points.push_back( CPoint(map_to(0, 0), map_to(1, d)) );
    points.push_back( CPoint(map_to(0, A), map_to(1, d)) );
    points.push_back( CPoint(map_to(0, A), map_to(1, d0)) );
    points.push_back( CPoint(map_to(0, C), map_to(1, c)) );
    points.push_back( CPoint(map_to(0, D), map_to(1, c)) );
    dc.Polyline(&points[0], points.size());

    // E
    points.clear();
    points.push_back( CPoint(map_to(0, 0), map_to(1, e)) );
    points.push_back( CPoint(map_to(0, B), map_to(1, e)) );
    points.push_back( CPoint(map_to(0, B), map_to(1, d)) );
    points.push_back( CPoint(map_to(0, D), map_to(1, d)) );
    dc.Polyline(&points[0], points.size());

    dc.SelectPen(oldp);
}

void CMonpacWindow::MakeChannelRects(CDCHandle dc, std::vector<std::pair<int, CRect> > &rects)
{
    rects.clear();
    if (!result_) return;

    foreach(channel chn, result_->ch_info_)
    {
        if (chn.get_zip()==zip_na) continue;

        double hi=chn.h_index();
        double si=chn.s_index();

        CPoint pt=CPoint(map_to(0, hi), map_to(1, si));

        pt.x=std::max( frame_.left, std::min(frame_.right, pt.x) );
        pt.y=std::max( frame_.top, std::min(frame_.bottom, pt.y) );

        std::string txt=strlib::strf("%d", chn.get_number());

        CSize sz; dc.GetTextExtent(txt.c_str(), -1, &sz);

        CRect rect(pt, pt);
        rect.InflateRect(sz.cx/2, sz.cy/2);

        rects.push_back(std::make_pair(chn.get_number(), rect));
    }
}

void CMonpacWindow::DrawContents(CDCHandle dc, CRect frame)
{
    DrawLines(dc);

    if (!result_) return ;

    CFont font; font.CreatePointFont(	80, "Tahoma");
    CFontHandle oldf=dc.SelectFont(font);

    dc.SetBkMode(TRANSPARENT);

    foreach(channel chn, result_->ch_info_)
    {
        double hi=chn.h_index();
        double si=chn.s_index();
        CPoint pt=CPoint(map_to(0, hi), map_to(1, si));

        pt.x=std::max( frame.left, std::min(frame.right, pt.x) );
        pt.y=std::max( frame.top, std::min(frame.bottom, pt.y) );

        CRect rect(pt, pt);
        rect.InflateRect(3,3);

        zip_t crt = chn.get_zip();
        COLORREF clrM=
            copyclip_ ? clrBkColor :
            crt==zip_0 ? RGB(0, 255, 0) :
            crt==zip_a ? RGB(0, 255, 0) :
            crt==zip_b ? RGB(0, 255, 255) :
            crt==zip_c ? RGB(255, 0, 255) :
            crt==zip_d ? RGB(255, 255, 0) :
            crt==zip_e ? RGB(255, 0, 0) :
            RGB(255, 0, 0) ;

        if (crt!=zip_na)
        {
            dc.SetTextColor(clrM);
            dc.DrawText(strlib::strf("%d", chn.get_number()).c_str(), -1, rect, DT_SINGLELINE|DT_CENTER|DT_VCENTER|DT_NOCLIP);
        }
    }

    dc.SelectFont(oldf);
}

std::string CMonpacWindow::GetChannelsUnderCursor()
{
    typedef std::pair<int, CRect> val_t;
    std::vector<val_t > vector;

    CDCHandle dc( GetWindowDC() );
    CFont font; font.CreatePointFont(80, "Tahoma");
    CFontHandle oldf=dc.SelectFont(font);

    MakeChannelRects(dc, vector);

    CPoint mouse; GetCursorPos(&mouse);
    ScreenToClient(&mouse);

    std::string caption;
    foreach(val_t pair, vector)
    {
        if (pair.second.PtInRect(mouse) && !copyclip_)
        {
            caption+=cvt(pair.first)+" ";
        }
    }

    dc.SelectFont(oldf);
    return caption;
}

void CMonpacWindow::SetMonpac(shared_ptr<class monpac::monpac_result> pr)
{
    result_ = pr;
    Invalidate();
}

LRESULT CMonpacWindow::OnMouseMove(UINT, CPoint mouse)
{
    TRACKMOUSEEVENT me= {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, HOVER_DEFAULT};
    ::TrackMouseEvent(&me);

    std::string text=GetChannelsUnderCursor();

    if (text.size() )
    {
        if (text!=tip_)
        {
            tooltip_.UpdateTipText(CToolInfo(0, m_hWnd, 0, 0, (char *)text.c_str()));

            ClientToScreen(&mouse);
            tooltip_.TrackPosition(mouse.x+16, mouse.y+16);
            tooltip_.TrackActivate(CToolInfo(0, m_hWnd, 0), true);
        }
    }
    else
    {
        tooltip_.TrackActivate(CToolInfo(0, m_hWnd, 0), false);
    }
    tip_=text;

    Invalidate();
    return 0;
}

LRESULT CMonpacWindow::OnMouseLeave()
{
    tooltip_.TrackActivate(CToolInfo(0, m_hWnd, 0), false);
    Invalidate();
    return 0;
}
