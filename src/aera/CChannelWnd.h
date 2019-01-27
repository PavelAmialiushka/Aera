#pragma once

#include "resource.h"
#include "others\docking\DockMisc.h"
#include "others\docking\DockingWindow.h"
#include "others\docking\DockingBox.h"
#include "others\docking\ExtDockingWindow.h"

#include "controls/PlotPool.h"

#include "data/processes.h"

#include "utilites/serl/Archive.h"

enum
{
    WM_FOCUSONACTIVEVIEW=WM_USER+10
};

#include "data/channel_sheme.h"


using namespace dockwins;


class CChannelWnd
    : public CTitleDockingWindowImpl<CChannelWnd, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits>
    , public serl::serializable
    , public process::host_listener
    , public plot_listener
{
    typedef dockwins::CTitleDockingWindowImpl<CChannelWnd, CWindow,dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
public:
    DECLARE_WND_CLASS("aeraCChannelWnd")

    BOOL PreTranslateMessage(MSG *pMsg)
    {
        pMsg;
        return FALSE;
    }

    BEGIN_MSG_MAP_EX(CChannelWnd)
    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)

    MSG_WM_SIZE(OnSize);
    MSG_WM_SIZING(OnSizing);
    MSG_WM_MOVING(OnMoving);
    MSG_WM_GETMINMAXINFO(OnGetMinMaxInfo);

    MSG_WM_SETFOCUS(OnSetFocus);
    MSG_WM_KILLFOCUS(OnLostFocus);

    MSG_WM_DRAWITEM(DrawItem);
    MSG_WM_ERASEBKGND(OnEraseBackground);

    MSG_WM_MOUSEWHEEL(OnMouseWheel);

    MSG_WM_VSCROLL(OnVScroll);

    COMMAND_ID_HANDLER_EX(IDC_EDIT_INVERT, PostToParent)
    COMMAND_ID_HANDLER_EX(IDC_EDIT_ALL,    PostToParent)

    COMMAND_CODE_HANDLER_EX(BN_CLICKED, OnClicked);
    COMMAND_CODE_HANDLER_EX(BN_DBLCLK, OnClicked);

    CHAIN_MSG_MAP(baseClass);

    ALT_MSG_MAP(2)

    END_MSG_MAP()

    CChannelWnd();
    ~CChannelWnd();

    HWND Create(HWND hparent);
    void CreateButton(int i, int g);

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnDestroy();

    LRESULT OnSize(UINT, CSize sz);
    LRESULT OnSizing(UINT, LPRECT sz);
    LRESULT OnMoving(UINT, LPRECT lpRect);
    LRESULT OnGetMinMaxInfo(LPMINMAXINFO);

    LRESULT OnVScroll(int, short, HWND);
    LRESULT OnSetFocus(HWND);
    LRESULT OnLostFocus(HWND);

    LRESULT DrawItem(int, LPDRAWITEMSTRUCT);

    LRESULT OnClicked(UINT, int, HWND);
    LRESULT OnRButton(UINT, int, HWND);

    LRESULT OnMouseWheel(UINT, short, CPoint);

    LRESULT OnEraseBackground(HDC);

    LRESULT PostToParent(UINT, int, HWND);

    void safe_on_finish(process::prslt);
    void safe_on_restart();
    void select_channel(int ch);

    //////////////////////////////////////////////////////////////////////////

    void    ClickAll();
    void    ClickInvert();
    void    OnChangeLanguage();

    void    serialization(serl::archiver &);

private:

    CSize   AdjustSize(CSize, UINT=WMSZ_BOTTOMRIGHT);
    CRect   AdjustRect(CRect rc) const;
    CSize   GetButtonSize() const;
    void    update_nodes();
    void    InitiateButtons();
    void    DestroyButtons();
    void    InvalidateButtons();

    void    AdjustButtonLayout();
    void    AdjustScrollBar();


private:

    DFDOCKPOSEX   serial_dock_pos_;

    CFont         spin_buttons_font_;
    unsigned      hiddenrows_;
    unsigned      row_;
    unsigned      dy_;

    CToolBarCtrl  toolbar_;
    CScrollBar    scroll_;

    enum {B_NAME=512};
    struct Button
    {
        CButton wnd;
        int id;
        int group;

        Button(int id, int group);
    };

    std::vector<Button>  buttons_;
    boost::array<bool, 256>     checks_;
    boost::array<bool, 256>     activec_;

    channel_sheme        sheme_;
};
