#pragma once

#include "TVView.h"

//////////////////////////////////////////////////////////////////////////

class Ball
{
public:
    Ball(int x=0, int y=0, int cx=rand()%10+3, int cy=rand()%10+3)
        : current_(x,y),
          velocity_(cx, cy)
    {
        color_=RGB(rand()%255, rand()%255, rand()%255);
    }

    void Paint(CDC &dc)
    {
        CBrush bru( ::CreateSolidBrush( color_ ));
        dc.FillRect(CRect(current_, CSize(5,5)), bru);
    }

    void Move(CRect rc)
    {
        rc.DeflateRect(0,0,5,5);
        current_.x+=velocity_.cx;
        current_.y+=velocity_.cy;

        if (rc.left > current_.x)  velocity_.cx=+abs(velocity_.cx);
        if (current_.x > rc.right) velocity_.cx=-abs(velocity_.cx);

        if (rc.top > current_.y)    velocity_.cy=+abs(velocity_.cy);
        if (current_.y > rc.bottom) velocity_.cy=-abs(velocity_.cy);

        current_.x=std::max(rc.left, std::min(rc.right, current_.x));
        current_.y=std::max(rc.top,  std::min(rc.bottom,current_.y));
    }

    CPoint current_;
    CSize  velocity_;
    COLORREF color_;
};

//////////////////////////////////////////////////////////////////////////

class CTVBallView : public CTileViewImpl<CTVBallView>
{
public:
    DECLARE_WND_CLASS(NULL)

    BOOL PreTranslateMessage(MSG *pMsg);

    typedef CTileViewImpl<CTVBallView> baseClass;
    BEGIN_MSG_MAP_EX(CTVBallView)
    MSG_WM_CREATE(OnCreate);
    MESSAGE_HANDLER(WM_PAINT, OnPaint);

    MSG_WM_ERASEBKGND(OnEraseBackground);

    MSG_WM_LBUTTONDBLCLK(OnLButtonDblClk);
    MSG_WM_LBUTTONDOWN(OnLButtonDown);
    MSG_WM_RBUTTONDOWN(OnRButtonDown);

    MSG_WM_KILLFOCUS(OnFocus);
    MSG_WM_SETFOCUS(OnFocus);
    MSG_WM_TIMER(OnTimer);

    CHAIN_MSG_MAP( baseClass )
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    CTVBallView();

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);

    LRESULT OnFocus(HWND);
    LRESULT OnEraseBackground(HDC);

    LRESULT OnLButtonDown(UINT, CPoint);
    LRESULT OnRButtonDown(UINT, CPoint);
    LRESULT OnLButtonDblClk(UINT, CPoint);

    LRESULT OnTimer(UINT, TIMERPROC=0);

    void OnFinalMessage(HWND);

    Ball a, b, c;

    void serialization(serl::archiver &ar);
};



