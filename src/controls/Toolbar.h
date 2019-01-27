#pragma once

//////////////////////////////////////////////////////////////////////////
#include "aera/wtladdons.h"

enum { TBN_GETBUTTONSTATE=TBN_FIRST-19, };

struct NMTOOLBARSTATE
{
    NMHDR			hdr;
    TBBUTTON	btn;
};

class Toolbar
{
public:

    void	Create(HWND parent);

    void SetButtons(UINT, int);
    void SetStrings(std::string);

    void AddButtons(TBBUTTON *, unsigned sz);

    void ShowWindow(int);

    void SetState(int id, int state, int state2);

public:

    void	OnPaint(HDC);
    bool	OnLButtonDown(UINT, CPoint);
    bool  OnLButtonUp(UINT, CPoint);
    bool  OnMouseMove(UINT, CPoint);
    void  OnSize(UINT, CSize);

private:

    CRect GetIconRect(unsigned idnex);
    void  DrawButton(HDC, unsigned);

private:

    bool                toolbar_visible_;
    int                 pressed_button_;

    std::vector<std::string> strings_;
    CImageList							 imagelist_;
    CToolTipCtrl        tooltip_;

    std::vector<TBBUTTON>	 buttons_;

    HWND								hparent_;

};