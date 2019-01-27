#pragma once

#include "page.h"

class CTileViewFrame;

enum
{
    AM_SETNUMBER=WM_USER,
};

class CArrangeWnd : public CWindowImpl<CArrangeWnd>
{
public:
    DECLARE_WND_CLASS(NULL);

    BEGIN_MSG_MAP_EX(CArrangeWnd)
    MSG_WM_CREATE(OnCreate);

    MSG_WM_ERASEBKGND(OnEraseBkGnd);
    MSG_WM_PAINT(OnPaint);

    MSG_WM_KEYDOWN(OnKeyDown);
    MSG_WM_KILLFOCUS(OnKillFocus);

    MSG_WM_MOUSEMOVE(OnMouseMove);
    MSG_WM_LBUTTONDOWN(OnLButtonDown);

    MSG_WM_HELP(OnHelp);

    MESSAGE_HANDLER_EX(AM_SETNUMBER, OnSetNumber);
    END_MSG_MAP()

    CArrangeWnd(CTileViewFrame *base, bool inc);

private:

    virtual void OnFinalMessage(HWND);

    LRESULT OnCreate(LPCREATESTRUCT);

    LRESULT OnSetNumber(UINT, LPARAM, WPARAM);

    LRESULT OnKeyDown(TCHAR, UINT, UINT);
    LRESULT OnKillFocus(HWND);

    LRESULT OnEraseBkGnd(HDC);
    LRESULT OnPaint(HDC);

    LRESULT OnMouseMove(UINT, CPoint);
    LRESULT OnLButtonDown(UINT, CPoint);

    LRESULT OnHelp(LPHELPINFO);

private:

    void DrawImage(CDCHandle dc, CPoint pt, int image);
    CRect GetImageRect(int index);

private:

    int window_count_;
    int pattern_count_;
    bool	increase_;
    rect *rects_;
    Page *base_;
    CTileViewFrame *frame_;
};