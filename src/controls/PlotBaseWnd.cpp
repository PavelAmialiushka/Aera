#include "stdafx.h"

#include "PlotBaseWnd.h"

#include "Tileview\TVFrame.h"
#include "utilites/security.h"

#include "bitmapMaker.h"

serl::mark_as_rtti< CTVCreatorImpl<CPlotWndBase> > INSTANCE;

CPlotWndBase::CPlotWndBase()
    : framesize_(0, 0),
      frame_(0, 0, 0, 0),
      copyclip_(false),
      track_values_(true),
      show_axes_captions_(false)
{
}

LRESULT CPlotWndBase::OnSizeChanged(UINT uMsg, WPARAM wParam, LPARAM )
{
    if (uMsg==WM_SHOWWINDOW && wParam==FALSE) return 0;

    CRect oldframe=frame_;

    CDC dc=GetWindowDC();
    CalculateFrame( (HDC)dc );
    if (frame_==oldframe) return 0;

    OnSizeChanged();
    return 0;
}

void CPlotWndBase::CalculateFrame(CDCHandle dc)
{
    CRect rc; ::GetClientRect(m_hWnd, rc);

    frame_=MakeFrameFromRect(dc, rc);
    outframe_=MakeOutframe(frame_);
}

CRect CPlotWndBase::MakeOutframe(CRect frame)
{
    CRect outframe=frame;
    outframe.InflateRect(2, 2);
    return outframe;
}

CRect CPlotWndBase::MakeFrameFromRect(CDCHandle dc, CRect rc)
{
    CFont font; font.CreatePointFont(80, "Tahoma");
    CFontHandle oldf=dc.SelectFont(font);

    CSize sz; dc.GetTextExtent("X", 1, &sz);

    CRect outframe=rc; outframe.DeflateRect(5, 5);

    if (!show_axes_captions_)
        outframe.DeflateRect(7 * sz.cx + 3, 2*sz.cy - 2,
                                 3 * sz.cx, sz.cy + 3);
    else
        outframe.DeflateRect(sz.cy + 3, 2*sz.cy - 2,
                             3 * sz.cx, sz.cy + 3);

    CRect frame=outframe; frame.DeflateRect(2, 2);

    dc.SelectFont(oldf);
    return frame;
}


CRect CPlotWndBase::get_outframe() const
{
    return outframe_;
}

CRect CPlotWndBase::get_frame() const
{
    return frame_;
}

void CPlotWndBase::OnSizeChanged()
{
}

LRESULT CPlotWndBase::OnPaint(HDC hdc)
{
    copyclip_=false;

    CPaintDC paintDC(m_hWnd);
    CDCHandle dc0(hdc ? hdc : paintDC);

    CRect rc; GetClientRect(rc);

    CDC dc( ::CreateCompatibleDC(dc0) );
    CBitmap bitmap( ::CreateCompatibleBitmap(dc0, rc.Width(), rc.Height()) );
    dc.SelectBitmap( bitmap );

    OnPaint((HDC)dc, rc);

    dc0.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);
    return 0;
}

void CPlotWndBase::OnPaint(CDCHandle dc, CRect rc)
{
    CRect frame=MakeFrameFromRect(dc, rc);

    dc.SetBkMode(TRANSPARENT);
    dc.SetTextColor(0x0);

    dc.FillRect(rc, CBrush(::CreateSolidBrush(copyclip_ ? clrNegBkColor : GetSysColor(COLOR_3DFACE))));
    dc.FillRect(MakeOutframe(frame), CBrush(::CreateSolidBrush(copyclip_ ? clrNegBkColor : clrBkColor )));

    // screen
    int state = dc.SaveDC();

    SetCaption();

    DrawFrame((HDC)dc, frame);

    DrawTitles((HDC)dc, frame);

    DrawContents((HDC)dc, frame);

    if (security::manager().is_unregistered())
    {
        CFont font; font.CreatePointFont(280, "Tahoma");
        dc.SetTextColor( copyclip_ ? 0x007F7F : 0x007F7F );
        dc.SetBkMode(TRANSPARENT);

        CFontHandle oldf=dc.SelectFont(font);
        dc.DrawText("Demo", -1, frame, DT_RIGHT|DT_TOP);

        dc.SelectFont(oldf);
    }

    dc.RestoreDC(state);
}

int calculate_yaxe_marks(CDCHandle dc, plot::range r, unsigned height)
{
    CSize sz; dc.GetTextExtent("X", 1, &sz);
    for (int level=-3; level<30; ++level)
    {
        plot::range::marks m=r.get_marks(level);
        if (m.size()*sz.cy >= height)
        {
            return level-1;
        }
    }
    return 0;
}

int calculate_xaxe_marks(CDCHandle dc, plot::range r, unsigned width)
{
    try
    {
        CSize sz; dc.GetTextExtent("X", 1, &sz);
        for (int level=-3; level<12; ++level)
        {
            plot::range::marks m=r.get_marks(level);
            if (m.get_max_length() * sz.cx * m.size()>=width)
            {
                return level-1;
            }
        }
    } LogExceptionPath("calculate_xaxe_marks");
    return 0;
}


void CPlotWndBase::DrawFrame(CDCHandle dc, CRect frame)
{
    CRect outframe=MakeOutframe(frame);

    plot::range xaxe=xrange_;
    plot::range yaxe=yrange_;
    plot::range::marks m=xaxe.get_marks(0);

    CPen pen0( ::CreatePen(PS_SOLID, 0, clrMarks ) );
    CPenHandle oldp=dc.SelectPen(pen0);

    CFont font; font.CreatePointFont(80, "Tahoma");
    CFontHandle oldf=dc.SelectFont(font);

    m=yaxe.get_marks( calculate_yaxe_marks(dc, yaxe, frame.Height()) );
    for (unsigned index=0; index<m.size(); ++index)
    {
        double val=m.get_double_at(index);
        int y=frame.bottom-yaxe.map_to(val, frame.Height());
        DrawMarkLeft(dc, frame, y, m[index]);
    }

    m=xaxe.get_marks( calculate_xaxe_marks(dc, xaxe, frame.Width()) );
    for (unsigned index=0; index<m.size(); ++index)
    {
        double val=m.get_double_at(index);
        int x=frame.left+xaxe.map_to(val, frame.Width());
        DrawMarkDown(dc, frame, x, m[index]);
    }

    if (track_values_ && !copyclip_)
    {
        CPoint mouse; ::GetCursorPos(&mouse);
        ScreenToClient(&mouse);
        if (frame.PtInRect(mouse))
        {
            dc.SetBkMode(OPAQUE);
            dc.SetTextColor(clrActiveTitles);
            dc.SetBkColor( ::GetSysColor(COLOR_3DFACE) );

            CPen penmouse( ::CreatePen(PS_SOLID, 0, clrActiveMarks));
            dc.SelectPen( penmouse );

            DrawMarkDown(dc, mouse.x);
            DrawMarkLeft(dc, mouse.y);
        }
    }

    if (copyclip_)
    {
        // draw frame
        CPen pen( ::CreatePen(PS_SOLID, 0, clrMarks) );
        dc.SelectPen(pen);

        dc.MoveTo(outframe.left-1, outframe.top);
        dc.LineTo(outframe.right,  outframe.top);
        dc.LineTo(outframe.right,  outframe.bottom);
        dc.LineTo(outframe.left-1, outframe.bottom);
        dc.LineTo(outframe.left-1, outframe.top);
    }

    dc.SelectPen( oldp );
    dc.SelectFont( oldf );
}

void CPlotWndBase::DrawContents(CDCHandle dc, CRect frame)
{
}

void CPlotWndBase::DrawMarkLeft(CDCHandle dc, int y)
{
    DrawMarkLeft(dc, frame_, y,
                 yrange_.unmap(frame_.bottom-y, frame_.Height()));
}

void CPlotWndBase::DrawMarkLeft(CDCHandle dc, CRect frame, int y, std::string mark)
{
    CRect outframe=MakeOutframe(frame);

    if (outframe.top < y && y < outframe.bottom)
    {
        mark="  "+strlib::replace(mark, "e+0", "e+");

        CSize sz; dc.GetTextExtent(mark.c_str(), mark.length(), &sz);
        CPoint pt(outframe.left, y);
        dc.MoveTo(pt.x-1, pt.y);
        dc.LineTo(pt.x-3, pt.y);

        if (!show_axes_captions_)
        {
            pt-=CSize(sz.cx, sz.cy/2);
            dc.TextOut(pt.x-4, pt.y, mark.c_str());
        }
    }
}

void CPlotWndBase::DrawMarkDown(CDCHandle dc, int x)
{
    DrawMarkDown(dc, frame_, x,
                 xrange_.unmap(x-frame_.left, frame_.Width()));
}

void CPlotWndBase::DrawMarkDown(CDCHandle dc, CRect frame, int x, std::string mark)
{
    CRect outframe=MakeOutframe(frame);
    mark=" "+mark+" ";
    if (outframe.left < x && x < outframe.right)
    {
        CPoint pt(x, outframe.bottom);
        CSize sz; dc.GetTextExtent(mark.c_str(), mark.length(), &sz);
        dc.MoveTo(pt.x, pt.y);
        dc.LineTo(pt.x, pt.y+3);

        if (!show_axes_captions_)
        {
            pt-=CSize(sz.cx/2, -3);
            dc.TextOut(pt.x, pt.y+2, mark.c_str());
        }
    }
}

LRESULT CPlotWndBase::OnGetDlgCode(LPMSG)
{
    return DLGC_WANTCHARS;
}

LRESULT CPlotWndBase::OnEraseBackground(HDC)
{
    return 0;
}


LRESULT CPlotWndBase::OnMouseMove(UINT, CPoint)
{
    Invalidate();
    return 0;
}

int CPlotWndBase::map_to(bool isyaxe, double value) const
{
    return isyaxe
           ? frame_.bottom-yrange_.map_to(value, frame_.Height())
           : frame_.left+xrange_.map_to(value, frame_.Width());
}

double CPlotWndBase::unmap_double(bool isyaxe, int value) const
{
    return isyaxe
           ? yrange_.unmap_double(frame_.bottom-value, frame_.Height())
           : xrange_.unmap_double(value-frame_.left, frame_.Width());
}

std::string CPlotWndBase::unmap(bool isyaxe, int value) const
{
    return isyaxe
           ? yrange_.unmap(frame_.bottom-value, frame_.Height())
           : xrange_.unmap(value-frame_.left, frame_.Width());
}

void CPlotWndBase::DrawTitles(CDCHandle dc, CRect frame)
{
    dc.SetTextColor(clrTitles);

    CFont font; font.CreatePointFont(80, "Tahoma");
    CFontHandle oldf=dc.SelectFont(font);

    CSize sz; dc.GetTextExtent(caption_.c_str(), caption_.length(), &sz);

    const int margin = 3;
    CRect rc(frame.left, frame.top-2*sz.cy-margin,
             frame.right, frame.top-margin);

    int wordbreak = sz.cx > rc.Width() ? DT_WORDBREAK : DT_SINGLELINE;
    dc.DrawText(caption_.c_str(), -1, rc, DT_BOTTOM|DT_LEFT|wordbreak);

    if (show_axes_captions_)
    {
        dc.SetBkColor( GetSysColor(COLOR_3DFACE) );

        CRect xrect(frame.left, frame.bottom+6, frame.right, frame.bottom+6+sz.cy);
        dc.DrawText((" "+xcaption_+" ").c_str(), -1, xrect, DT_BOTTOM|DT_LEFT|DT_SINGLELINE);

        LOGFONT fnt= {0};
        strcpy(fnt.lfFaceName, "Tahoma");
        fnt.lfHeight=80;
        fnt.lfOrientation=900;
        fnt.lfEscapement=900;
        fnt.lfCharSet = DEFAULT_CHARSET;
        CFont font1; font1.CreatePointFontIndirect(&fnt);
        dc.SelectFont(font1);

        CPoint ypoint(frame.left-sz.cy-5, frame.bottom);
        dc.TextOut(ypoint.x, ypoint.y, (" "+ycaption_+" ").c_str());
    }

    dc.SelectFont(oldf);
}

LRESULT CPlotWndBase::OnCopyClip(UINT, WPARAM wParam, LPARAM lParam)
{
    CDCHandle dc((HDC)wParam);
    CRect   rect((LPRECT)lParam);

    CRect oldframe=MakeFrameFromRect(dc, rect);
    copyclip_=true;

    std::swap(oldframe, frame_);

    OnPaint(dc, rect);

    std::swap(oldframe, frame_);
    copyclip_=false;

    return 0;
}


