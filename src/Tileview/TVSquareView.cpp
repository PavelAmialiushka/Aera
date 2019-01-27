#include "Stdafx.h"

#include "TVSquareView.h"

#include "aera/wtladdons.h"

serl::mark_as_rtti< CTVCreatorImpl<CTVSquareView> > INSTANCE;


LRESULT CTVSquareView::OnCreate(LPCREATESTRUCT)
{
    SetTimer(0, 100);
    return 0;
}

void CTVSquareView::OnFinalMessage(HWND)
{
    delete this;
}


CTVSquareView::CTVSquareView()
{
}

BOOL CTVSquareView::PreTranslateMessage(MSG *pMsg)
{
    pMsg;
    return FALSE;
}

LRESULT CTVSquareView::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    CPaintDC dc(m_hWnd);

    CBrush br( ::CreateSolidBrush(0x000000)  );

    CRect rc; GetClientRect(rc);
    dc.FillRect(rc, br);

    if (GetFocus()==m_hWnd)
    {
        rc.DeflateRect(2,2);
        dc.DrawFocusRect(rc);
    }

    a.Paint(dc);

    return 0;
}

LRESULT CTVSquareView::OnFocus(HWND)
{
    Invalidate();
    return 0;
}

LRESULT CTVSquareView::OnEraseBackground(HDC)
{
    return 0;
}

LRESULT CTVSquareView::OnLButtonDown(UINT f, CPoint pt)
{
    SetFocus();
    return ParentLButtonDown(f, pt);
}

LRESULT CTVSquareView::OnRButtonDown(UINT f, CPoint pt)
{
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

    SetFocus();
    debug::Assert<fault>(GetFocus()==m_hWnd, HERE);
    Invalidate();

    return 0;
}


LRESULT CTVSquareView::OnTimer(UINT, TIMERPROC)
{
    CRect rc; GetClientRect(rc);

    a.Move(rc);

    Invalidate();
    return 0;
}