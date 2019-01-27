#pragma once

#include "TVView.h"

//////////////////////////////////////////////////////////////////////////

class Square
{
public:
    Square()
        : va_(rand()%10+3, rand()%10+3),
          vb_(rand()%10+3, rand()%10+3)
    {
        a_=CPoint(0,0);
        b_=CPoint(0,0);
        color_=RGB(128+rand()%127, 128+rand()%127, 128+rand()%127);
    }

    void Paint(CDC &dc)
    {
        CBrush bru( ::CreateSolidBrush( color_ ));
        dc.FillRect(CRect(a_, b_), bru);
    }

    void Move(CRect rc, CPoint &pt, CSize &vl)
    {
        pt.x+=vl.cx;
        pt.y+=vl.cy;

        if (rc.left > pt.x)  vl.cx=+abs(vl.cx);
        if (pt.x > rc.right) vl.cx=-abs(vl.cx);

        if (rc.top > pt.y)    vl.cy=+abs(vl.cy);
        if (pt.y > rc.bottom) vl.cy=-abs(vl.cy);

        pt.x=std::max(rc.left, std::min(rc.right, pt.x));
        pt.y=std::max(rc.top,  std::min(rc.bottom,pt.y));
    }

    void Move(CRect rc)
    {
        Move(rc, a_, va_);
        Move(rc, b_, vb_);
    }

    CPoint a_, b_;
    CSize  va_, vb_;
    COLORREF color_;
};

//////////////////////////////////////////////////////////////////////////

class CTVSquareView : public CTileViewImpl<CTVSquareView>
{
public:
    DECLARE_WND_CLASS(NULL)

    BOOL PreTranslateMessage(MSG *pMsg);

    typedef CTileViewImpl<CTVSquareView> baseClass;
    BEGIN_MSG_MAP_EX(CTVSquareView)
    MSG_WM_CREATE(OnCreate);
    MESSAGE_HANDLER(WM_PAINT, OnPaint);

    MSG_WM_ERASEBKGND(OnEraseBackground);

    MSG_WM_LBUTTONDOWN(OnLButtonDown);
    MSG_WM_RBUTTONDOWN(OnRButtonDown);

    MSG_WM_KILLFOCUS(OnFocus);
    MSG_WM_SETFOCUS(OnFocus);
    MSG_WM_TIMER(OnTimer);

    CHAIN_MSG_MAP( baseClass );
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    CTVSquareView();

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);

    LRESULT OnFocus(HWND);
    LRESULT OnEraseBackground(HDC);

    LRESULT OnLButtonDown(UINT, CPoint);
    LRESULT OnRButtonDown(UINT, CPoint);

    LRESULT OnTimer(UINT, TIMERPROC=0);

    void OnFinalMessage(HWND);

    void serialization(serl::archiver &ar) {}
    Square a;
};



