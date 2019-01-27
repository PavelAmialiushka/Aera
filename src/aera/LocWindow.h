#pragma once

#include "others\docking\DockMisc.h"
#include "others\docking\DockingWindow.h"
#include "others\docking\DockingBox.h"
#include "others\docking\ExtDockingWindow.h"

#include "listViewCtrlOwnerDraw.h"

#include "resource.h"

#include "utilites/serl/Archive.h"

using namespace dockwins;

class CLocWnd :
    public CTitleDockingWindowImpl<CLocWnd, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits>
    , public serl::serializable
{
    typedef dockwins::CTitleDockingWindowImpl<CLocWnd, CWindow,dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
public:
    DECLARE_WND_CLASS("aeraCLocWnd")

    BOOL PreTranslateMessage(MSG *pMsg)
    {
        pMsg;
        return FALSE;
    }

    BEGIN_MSG_MAP_EX(CLocWnd)

    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)

    MESSAGE_HANDLER_EX(WM_SIZE,             OnSizeChanged);
    MESSAGE_HANDLER_EX(WM_WINDOWPOSCHANGED, OnSizeChanged);

    MSG_WM_ERASEBKGND(OnEraseBackground);

    COMMAND_ID_HANDLER_EX(IDC_LOCWIN_PROPERTY, OnProperty);
    COMMAND_ID_HANDLER_EX(IDC_LOCWIN_INSERT, OnInsertPos);
    COMMAND_ID_HANDLER_EX(IDC_LOCWIN_DELETE, OnDeletePos);

    NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnItemChanged);
    NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnItemDblClick);

    CHAIN_MSG_MAP_MEMBER(listctrl_)
    CHAIN_MSG_MAP(baseClass);
    END_MSG_MAP()

    CLocWnd();

    HWND Create(HWND hparent);

    // mesages

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnDestroy();

    LRESULT OnEraseBackground(HDC);
    LRESULT OnSizeChanged(UINT, WPARAM, LPARAM);

    // notification

    LRESULT OnItemChanged(LPNMHDR);
    LRESULT OnItemDblClick(LPNMHDR);

    // commands

    LRESULT OnProperty(UINT, int, HWND);
    LRESULT OnInsertPos(UINT, int, HWND);
    LRESULT OnDeletePos(UINT, int, HWND);

    //////////////////////////////////////////////////////////////////////////

    void    BuildList();
    void    UpdateList();
    void    OnChangeLanguage();

    void    serialization(serl::archiver &);

private:

    bool					ignore_;
    DFDOCKPOSEX   serial_dock_pos_;
    bool          registry_loaded_;

    CListViewCtrlOwnerDraw listctrl_;
    CToolBarCtrl	toolbar_;
};
