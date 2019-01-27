#include "Stdafx.h"

#include "TVBallView.h"

#include "aera/wtladdons.h"

serl::mark_as_rtti< CTVCreatorImpl<CTVBallView> > INSTANCE;


LRESULT CTVBallView::OnCreate(LPCREATESTRUCT)
{
    SetTimer(0, 100);
    return 0;
}

void CTVBallView::OnFinalMessage(HWND)
{
    delete this;
}

CTVBallView::CTVBallView()
{
}

BOOL CTVBallView::PreTranslateMessage(MSG *pMsg)
{
    pMsg;
    return FALSE;
}

LRESULT CTVBallView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    CPaintDC dc(m_hWnd);

    CBrush br( ::CreateSolidBrush(0x7f7f7f)  );

    CRect rc; GetClientRect(rc);
    dc.FillRect(rc, br);

    if (GetFocus()==m_hWnd)
    {
        rc.DeflateRect(2,2);
        dc.DrawFocusRect(rc);
    }

    a.Paint(dc);
    b.Paint(dc);
    c.Paint(dc);

    return 0;
}

LRESULT CTVBallView::OnFocus(HWND)
{
    Invalidate();
    return 0;
}

LRESULT CTVBallView::OnEraseBackground(HDC)
{
    return 0;
}

LRESULT CTVBallView::OnLButtonDown(UINT f, CPoint pt)
{
    SetFocus();
    return ParentLButtonDown(f, pt);
}

LRESULT CTVBallView::OnRButtonDown(UINT f, CPoint pt)
{
    SetFocus();

    CMenu menu( ::CreatePopupMenu() );

    menu.AppendMenu(MF_SEPARATOR);
    menu.AppendMenu(MF_STRING, 1, "Delete window");

    ClientToScreen(&pt);
    int res=menu.TrackPopupMenu(
                TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
                pt.x, pt.y, m_hWnd);

    if (res==1)
    {
        DeleteFromParent();
    }

    return 0;
}


LRESULT CTVBallView::OnTimer(UINT, TIMERPROC)
{
    CRect rc; GetClientRect(rc);

    a.Move(rc);
    b.Move(rc);
    c.Move(rc);

    Invalidate();
    return 0;
}

void CTVBallView::serialization(serl::archiver &ar)
{
    ar.serial("acolor", serl::makeint(a.color_));
    ar.serial("bcolor", serl::makeint(b.color_));
    ar.serial("ccolor", serl::makeint(c.color_));
}

LRESULT CTVBallView::OnLButtonDblClk(UINT, CPoint pt)
{
    ClientToScreen(&pt);
    StartDrag(pt);
    return 0;
}