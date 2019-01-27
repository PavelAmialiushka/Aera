#pragma once

#include "TVFrame.h"

class CMainFrame
    : public CFrameWindowImpl<CMainFrame>,
  public CUpdateUI<CMainFrame>,
  public CMessageFilter,
  public CIdleHandler
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME_2)

    CTileViewFrame m_view;

    virtual BOOL PreTranslateMessage(MSG *pMsg);
    virtual BOOL OnIdle();

    BEGIN_UPDATE_UI_MAP(CMainFrame)
    UPDATE_ELEMENT(ID_VIEW_TOOLBAR, UPDUI_MENUPOPUP)
    UPDATE_ELEMENT(ID_VIEW_STATUS_BAR, UPDUI_MENUPOPUP)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP_EX(CMainFrame)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
    COMMAND_ID_HANDLER(ID_FILE_NEW, OnFileNew)
    COMMAND_ID_HANDLER(ID_FILE_NEW_WINDOW, OnFileNewWindow)
    COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
    COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
    COMMAND_ID_HANDLER_EX(ID_REARRANGE, OnRearrange)

    COMMAND_ID_HANDLER_EX(ID_FILE_SAVE, m_view.OnFileSave);
    COMMAND_ID_HANDLER_EX(ID_FILE_OPEN, m_view.OnFileOpen);

    CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
    CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnFileNewWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnRearrange(UINT, int, HWND);

    HWND create_subwindow();
    HWND create_ball_subwindow();
    HWND create_square_subwindow();
};

/////////////////////////////////////////////////////////////////////////////
