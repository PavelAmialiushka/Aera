#include "stdafx.h"

#include "toolbar.h"

#include "utilites/tokenizer.h"
#include "utilites/foreach.hpp"

void Toolbar::Create(HWND parent)
{
    hparent_=parent;
    pressed_button_=-1;
    toolbar_visible_=false;

    tooltip_.Create(hparent_, CWindow::rcDefault, 0, WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP);
    tooltip_.SetWindowPos(HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void Toolbar::SetStrings(std::string str)
{
    strings_.clear();
    std::string item;
    while (1)
    {
        item=tokenizer::head(str, std::string(1, 0));
        if (item.size()) strings_.push_back(item);
        else break;
    }
}

void Toolbar::SetButtons(UINT res, int num)
{
    imagelist_.Create(res, 16, num, 0xC0C0C0);
}

void Toolbar::AddButtons(TBBUTTON *buttons, unsigned sz)
{
    buttons_.clear();
    std::copy(buttons, buttons+sz, std::back_inserter(buttons_));

    unsigned count=buttons_.size();
    for (unsigned index=0; index<count; ++index)
    {
        CRect rect=GetIconRect(index);
        int ids=buttons_[index].iString;
        const char *str=strings_.at(ids).c_str();
        CToolInfo ti(
            TTF_SUBCLASS,
            hparent_,
            buttons_[index].idCommand,
            rect,
            const_cast<char *>(str));
        tooltip_.AddTool(ti);
    }
}

CRect Toolbar::GetIconRect(unsigned index)
{
    CSize iconsize;
    imagelist_.GetIconSize(iconsize);
    iconsize.cx+=4;
    iconsize.cy+=4;

    int count=buttons_.size();

    CRect rc; GetClientRect(hparent_, rc);

    CPoint start=CPoint(rc.right-iconsize.cx*(count-index)-1, rc.top);
    return CRect(start, iconsize);
}

void Toolbar::DrawButton(HDC hdc, unsigned index)
{
    CDCHandle dc(hdc);

    CRect buttonrect=GetIconRect(index);
    COLORREF color=GetSysColor(COLOR_3DFACE);
    dc.FillRect(buttonrect, CBrush( ::CreateSolidBrush(color) ));

    buttonrect.DeflateRect(1,1);

    CPoint cursor; GetCursorPos(&cursor);
    ::ScreenToClient(hparent_, &cursor);
    enum { TBSTATE_HOT = 0x800 };

    NMTOOLBARSTATE tbs=
    {{hparent_, -1, TBN_GETBUTTONSTATE}};
    tbs.btn=buttons_[index];

    ::SendMessage(hparent_, WM_NOTIFY, -1, reinterpret_cast<LPARAM>(&tbs));

    int state=tbs.btn.fsState;

    if (buttonrect.PtInRect(cursor)) state=TBSTATE_HOT;
    if ((int)index==pressed_button_) state=TBSTATE_PRESSED;


    CPen hl, dk;
    if (state & TBSTATE_HOT)
    {
        hl.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
        dk.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
    }
    else if (state & TBSTATE_PRESSED)
    {
        dk.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DHILIGHT));
        hl.CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DDKSHADOW));
    }

    if (hl && dk)
    {
        CPenHandle oldp=dc.SelectPen(hl);

        dc.MoveTo(buttonrect.left, buttonrect.bottom);
        dc.LineTo(buttonrect.left, buttonrect.top);
        dc.LineTo(buttonrect.right, buttonrect.top);

        dc.SelectPen(dk);

        dc.LineTo(buttonrect.right, buttonrect.bottom);
        dc.LineTo(buttonrect.left, buttonrect.bottom);

        dc.SelectPen(oldp);
    }

    imagelist_.Draw(
        dc, buttons_[index].iBitmap,
        buttonrect.left+2,
        buttonrect.top+2,
        ILD_TRANSPARENT);

}

void Toolbar::OnPaint(HDC hdc)
{
    CDCHandle dc(hdc);
    if (toolbar_visible_)
    {
        unsigned count=buttons_.size();
        for (unsigned index=0; index<count; ++index)
        {
            DrawButton(dc, index);
        }
    }
}

bool Toolbar::OnLButtonDown(UINT, CPoint pt)
{
    unsigned count=buttons_.size();
    for (unsigned index=0; index<count; ++index)
    {
        if (GetIconRect(index).PtInRect(pt))
        {
            pressed_button_=index;
            ::InvalidateRect(hparent_, NULL, false);
            return true;
        }
    }
    return false;
}

bool Toolbar::OnLButtonUp(UINT, CPoint pt)
{
    if (pressed_button_>=0)
    {
        if (GetIconRect(pressed_button_).PtInRect(pt))
        {
            ::SendMessage(hparent_, WM_COMMAND, buttons_[pressed_button_].idCommand, 0);
        }
        pressed_button_=-1;
        ::InvalidateRect(hparent_, NULL, FALSE);
        return true;
    }
    return false;
}

void Toolbar::ShowWindow(int sw)
{
    toolbar_visible_=sw;
    ::InvalidateRect(hparent_, NULL, FALSE);
}

void Toolbar::OnSize(UINT, CSize)
{
    unsigned count=buttons_.size();
    for (unsigned index=0; index<count; ++index)
    {
        CRect rect=GetIconRect(index);
        tooltip_.SetToolRect(hparent_, buttons_[index].idCommand, rect);
    }
}

void Toolbar::SetState(int id, int state1, int state2)
{
    foreach(TBBUTTON &btn, buttons_)
    {
        if (btn.idCommand==id)
        {
            btn.fsState = (btn.fsState & ~state2) | state1;
        }
    }
    ::InvalidateRect(hparent_, NULL, FALSE);
}

bool Toolbar::OnMouseMove(UINT, CPoint pt)
{
    for (unsigned index=0; index<buttons_.size(); ++index)
    {
        CRect buttonrect=GetIconRect(index);
        if (buttonrect.PtInRect(pt))
            return true;
    }
    return false;
}