#pragma once

#include "others\docking\DockMisc.h"
#include "others\docking\DockingWindow.h"
#include "others\docking\DockingBox.h"
#include "others\docking\ExtDockingWindow.h"

#include "utilites/serl/Archive.h"
#include "listViewCtrlOwnerDraw.h"

#include "resource.h"

using namespace dockwins;

class CStagesWnd
    : public CTitleDockingWindowImpl<CStagesWnd, CWindow, dockwins::COutlookLikeTitleDockingWindowTraits>
    , public serl::serializable
{
    typedef dockwins::CTitleDockingWindowImpl<CStagesWnd, CWindow,dockwins::COutlookLikeTitleDockingWindowTraits> baseClass;
public:
    DECLARE_WND_CLASS("aeraCStagesWnd")

    BOOL PreTranslateMessage(MSG *pMsg)
    {
        pMsg;
        return FALSE;
    }

    BEGIN_MSG_MAP_EX(CStagesWnd)
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)

        MESSAGE_HANDLER_EX(WM_SIZE,             OnSizeChanged);
        MESSAGE_HANDLER_EX(WM_WINDOWPOSCHANGED, OnSizeChanged);

        MSG_WM_ERASEBKGND(OnEraseBackground);

        NOTIFY_CODE_HANDLER_EX(LVN_ITEMCHANGED, OnItemChanged);
        NOTIFY_CODE_HANDLER_EX(NM_DBLCLK, OnItemDblClick);

        COMMAND_ID_HANDLER_EX(ID_STAGE_PROPERTY, OnStageProperty);

        CHAIN_MSG_MAP_MEMBER(listctrl_)
        CHAIN_MSG_MAP(baseClass);
    END_MSG_MAP()

    CStagesWnd();

    HWND Create(HWND hparent);

    // mesages

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnDestroy();

    LRESULT OnEraseBackground(HDC);
    LRESULT OnSizeChanged(UINT, WPARAM, LPARAM);

    // notification

    LRESULT OnItemChanged(LPNMHDR);
    LRESULT OnItemDblClick(LPNMHDR);

    // command

    LRESULT OnStageProperty(UINT, int, HWND);

    //////////////////////////////////////////////////////////////////////////

    void    LoadPositionFromRegistry();
    void    SavePositionToRegistry();

    void    BuildList();
    void    UpdateList();
    void    OnChangeLanguage();

    void    serialization(serl::archiver &);

private:

    DFDOCKPOSEX   serial_dock_pos_;
    bool          registry_loaded_;

    CListViewCtrlOwnerDraw listctrl_;
    CToolBarCtrl  toolbar_;
};
