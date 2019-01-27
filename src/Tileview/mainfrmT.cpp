// MainFrm.cpp : implmentation of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainFrmT.h"

#include "TVBallView.h"
#include "TVSquareView.h"

BOOL CMainFrame::PreTranslateMessage(MSG *pMsg)
{
    if (CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg))
        return TRUE;

    return m_view.PreTranslateMessage(pMsg);
}

BOOL CMainFrame::OnIdle()
{
    UIUpdateToolBar();
    return FALSE;
}


HWND CMainFrame::create_ball_subwindow()
{
    CTVBallView *view=new CTVBallView;
    return view->Create(m_view, rcDefault);
}

HWND CMainFrame::create_square_subwindow()
{
    CTVSquareView *view=new CTVSquareView;
    return view->Create(m_view, rcDefault);
}

HWND CMainFrame::create_subwindow()
{
    CTVSelectView *view=new CTVSelectView;

    view->AppendWindowType( "Ball window",  new CTVCreatorImpl<CTVBallView>());
    view->AppendWindowType( "Square window",  new CTVCreatorImpl<CTVSquareView>());

    return view->Create(m_view, CWindow::rcDefault, NULL, WS_CHILD|WS_VISIBLE);
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    CreateSimpleToolBar();

    CreateSimpleStatusBar();

    m_hWndClient = m_view.Create(m_hWnd, rcDefault, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);

    m_view.SetCreateWindowCallback( bind(&CMainFrame::create_subwindow, this) );
    m_view.AppendWindows(5);
    m_view.UpdateLayout();

    UIAddToolBar(m_hWndToolBar);
    UISetCheck(ID_VIEW_TOOLBAR, 1);
    UISetCheck(ID_VIEW_STATUS_BAR, 1);

    // register object for message filtering and idle updates
    CMessageLoop *pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);


    return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    PostMessage(WM_CLOSE);
    return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    // TODO: add code to initialize document

    return 0;
}

LRESULT CMainFrame::OnFileNewWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    ::PostThreadMessage(_Module.m_dwMainThreadID, WM_USER, 0, 0L);
    return 0;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    BOOL bVisible = !::IsWindowVisible(m_hWndToolBar);
    ::ShowWindow(m_hWndToolBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_TOOLBAR, bVisible);
    UpdateLayout();
    return 0;
}

LRESULT CMainFrame::OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    BOOL bVisible = !::IsWindowVisible(m_hWndStatusBar);
    ::ShowWindow(m_hWndStatusBar, bVisible ? SW_SHOWNOACTIVATE : SW_HIDE);
    UISetCheck(ID_VIEW_STATUS_BAR, bVisible);
    UpdateLayout();
    return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    CAboutDlg dlg;
    dlg.DoModal();
    return 0;
}

LRESULT CMainFrame::OnRearrange(UINT, INT, HWND)
{
    m_view.ShowArrangeWindow();
    return 0;
}