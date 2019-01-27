// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__E0BCEF90_0E03_4074_9DAE_C387A66FD589__INCLUDED_)
#define AFX_MAINFRM_H__E0BCEF90_0E03_4074_9DAE_C387A66FD589__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "statusbar.h"

#include "resource.h"

#include "CChannelWnd.h"
#include "StagesWindow.h"
#include "LocWindow.h"

#include "mrulist.h"
#include "utilites/security.h"

#include "others/docking/DockingFrame.h"
#include "others/docking/sstate.h"

#include "TabFrame.h"

#include "tileview/TVFrame.h"

#include "utilites/Localizator.h"

#include "utilites/document.h"

enum
{
    WM_UPDATEPROGRESS=WM_USER+5,
    WM_EXCLAMATION,    
};

using namespace dockwins;

class CMainFrame :
    public CDockingFrameImpl<CMainFrame>,
    public CUpdateUI<CMainFrame >,
    public CMessageFilter,
    public CIdleHandler,
    public DocumentImpl
{
    typedef CDockingFrameImpl<CMainFrame> baseClass;
public:

    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)

    virtual BOOL PreTranslateMessage(MSG *pMsg)
    {
        if (CDockingFrameImpl<CMainFrame>::PreTranslateMessage(pMsg))
            return TRUE;

        //return m_view.PreTranslateMessage(pMsg);
        return FALSE;
    }

    virtual BOOL OnIdle();

    MRUList projects_;

    void UpdateMenu()
    {
        projects_.save_to_registry();
        projects_.UpdateMenu(m_CmdBar.GetMenu(), ID_MRU_PROJECTS1);
    }

    BEGIN_UPDATE_UI_MAP(CMainFrame)
        UPDATE_ELEMENT(ID_VIEW_TOOLBAR,     UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_PLOT_TOOLBAR,UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(ID_VIEW_STATUS_BAR,  UPDUI_MENUPOPUP)

        UPDATE_ELEMENT(IDC_VIEW_CHANNELS, UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(IDC_VIEW_STAGES,   UPDUI_MENUPOPUP)
        UPDATE_ELEMENT(IDC_VIEW_LOCATION, UPDUI_MENUPOPUP)

        UPDATE_ELEMENT(IDC_PLOT_NORMAL, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
        UPDATE_ELEMENT(IDC_PLOT_PAN,    UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
        UPDATE_ELEMENT(IDC_PLOT_ZOOMIN, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)

        UPDATE_ELEMENT(IDC_PLOT_ZOOMOUT, UPDUI_MENUPOPUP|UPDUI_TOOLBAR)

        UPDATE_ELEMENT(ID_SAVE_PROJECT,     UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_SAVE_PROJECT_AS,  UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_FILE_SAVE,        UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
        UPDATE_ELEMENT(ID_SAVE_LAYOUT,      UPDUI_MENUPOPUP|UPDUI_TOOLBAR)
    END_UPDATE_UI_MAP()

    BEGIN_MSG_MAP_EX(CMainFrame)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MSG_WM_DESTROY(OnDestroy);

        MSG_WM_CLOSE( OnClose );

        MSG_WM_SETFOCUS(OnSetFocus);
        MSG_WM_KILLFOCUS(OnLostFocus);

        MESSAGE_HANDLER(WM_UPDATEPROGRESS, OnUpdateProgress);        
        MESSAGE_HANDLER(WM_EXCLAMATION, OnExclamation);
        MESSAGE_HANDLER_EX(WM_FOCUSONACTIVEVIEW, OnFocusOnActiveView);

        MESSAGE_HANDLER_EX(WM_NOTIFY, OnNotify)

        MSG_WM_HOTKEY(OnHotKey);

        MSG_WM_TIMER(OnTimer);

        MSG_WM_HELP(OnHelp);

        // file
        COMMAND_ID_HANDLER(ID_FILE_SAVE, OnFileSave)
        COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
        COMMAND_ID_HANDLER(IDC_RELOAD_FILE, OnFileReload)

        COMMAND_ID_HANDLER(IDC_PROPERTIES, OnFileProperties);

        COMMAND_ID_HANDLER(ID_SAVE_PROJECT, OnSaveProject)
        COMMAND_ID_HANDLER(ID_SAVE_PROJECT_AS, OnSaveProjectAs)
        COMMAND_ID_HANDLER(ID_LOAD_PROJECT, OnLoadProject)
        COMMAND_RANGE_HANDLER(ID_MRU_PROJECTS1, ID_MRU_PROJECTSN, OnMRUProject);

        COMMAND_ID_HANDLER(ID_SAVE_LAYOUT, OnSaveTemplate)
        COMMAND_ID_HANDLER(ID_LOAD_TEMPLATE, OnLoadTemplate)
        COMMAND_ID_HANDLER(ID_LOAD_LAYOUT, OnCreateUsingTemplate)

        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)

        // edit
        COMMAND_ID_HANDLER(IDC_EDIT_ALL, OnEditAll)
        COMMAND_ID_HANDLER(IDC_EDIT_INVERT, OnEditInvert);

        // view
        COMMAND_ID_HANDLER(ID_VIEW_TOOLBAR, OnViewToolBar)
        COMMAND_ID_HANDLER(ID_VIEW_PLOT_TOOLBAR, OnViewPlotToolBar)
        COMMAND_ID_HANDLER(ID_VIEW_STATUS_BAR, OnViewStatusBar)
        COMMAND_ID_HANDLER(IDC_VIEW_CHANNELS, OnViewChannelBar)
        COMMAND_ID_HANDLER(IDC_VIEW_STAGES, OnViewStagesBar)
        COMMAND_ID_HANDLER(IDC_VIEW_LOCATION, OnViewLocationBar)
        //COMMAND_ID_HANDLER(IDC_VIEW_ALLTAB, OnViewAllBars)
        COMMAND_ID_HANDLER(ID_ARRANGEWINDOWS, OnArrangeWindows)
        COMMAND_ID_HANDLER(ID_ADDARRANGEWINDOW, OnAddArrangeWindow)

        // selection
        COMMAND_ID_HANDLER(IDC_SEL_DEL, OnSelDel)
        COMMAND_ID_HANDLER_EX(IDC_SEL_INVERT, OnSelInvert)

        // processing
        COMMAND_ID_HANDLER(ID_FILTER_SETTINGS, OnFiltering)
        COMMAND_ID_HANDLER(ID_FILTER_SETTINGS2, OnFiltering2)
        COMMAND_ID_HANDLER(ID_MONPAC_SETTINGS, OnMonpacing)
        COMMAND_ID_HANDLER(IDC_VIEW_STAGES1, OnViewStages)

        // settings
        COMMAND_RANGE_HANDLER(ID_DEFAULTENGLISH,
                              ID_DEFAULTENGLISH+20, OnChangeLanguage)

        COMMAND_ID_HANDLER_EX(IDC_SETTINGS_OPTIONS, OnOptions);

        // pages
        COMMAND_ID_HANDLER_EX(IDC_PAGES_NEW_PAGE, OnNewPage);
        COMMAND_ID_HANDLER_EX(IDC_PAGES_DELETE_PAGE, OnDeletePage);
        COMMAND_ID_HANDLER_EX(IDC_PAGES_RENAME_PAGE, OnRenamePage);
        COMMAND_ID_HANDLER_EX(IDC_PAGES_NEXT_PAGE, OnNextPage);
        COMMAND_ID_HANDLER_EX(IDC_PAGES_PREV_PAGE, OnPrevPage);

        // help
        COMMAND_ID_HANDLER(ID_HELP_INDEX, OnHelpIndex)
        COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)
        COMMAND_ID_HANDLER(IDC_REGISTER, OnRegister)
        COMMAND_ID_HANDLER(IDC_ORDER_SOFTWARE, OnOrder)
        COMMAND_ID_HANDLER_EX(IDM_RESIZEWINDOWX, OnResizeWindowX);

        COMMAND_ID_HANDLER_EX(IDC_WINDOWS_800_600, OnWindows800x600);
        COMMAND_ID_HANDLER_EX(IDC_WINDOWS_1024_768, OnWindows1024x768);

        COMMAND_ID_HANDLER_EX(IDC_PLOT_NORMAL, OnPlotNormal);
        COMMAND_ID_HANDLER_EX(IDC_PLOT_PAN, OnPlotPan);
        COMMAND_ID_HANDLER_EX(IDC_PLOT_ZOOMIN, OnPlotZoom);
        COMMAND_ID_HANDLER_EX(IDC_PLOT_ZOOMOUT, OnPlotZoomOut);

        // должно быть в конце
        MESSAGE_HANDLER(WM_COMMAND, OnCommand)

        if (!destroing_ || uMsg==WM_DESTROY)
        {
            CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
            CHAIN_MSG_MAP(baseClass)

            REFLECT_NOTIFICATIONS()
        }
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

    LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/);

    LRESULT OnChangeLanguage(int wNotifyCode, int wID, HWND ctrl, BOOL & /*bHandled*/);

    void    InitalizeLocalizator();
    void    InsertLanguageMenuItem(CMenuHandle langs, language_info const &lang);
    void    SetCheckedLanguage(CMenuHandle menu);

    LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);

    LRESULT OnSetFocus(HWND);
    LRESULT OnLostFocus(HWND);

    LRESULT OnDestroy();
    LRESULT OnClose();

    LRESULT OnHotKey(int, UINT, UINT);

    LRESULT OnUpdateProgress(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnExclamation(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/);
    LRESULT OnFocusOnActiveView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/);

    LRESULT OnFileProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnHelpIndex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnEditAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnEditInvert(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewPlotToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewStatusBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewChannelBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewStagesBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewLocationBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewAllBars(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnArrangeWindows(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnAddArrangeWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnSelInvert(UINT, int, HWND);
    LRESULT OnOptions(UINT, int, HWND);

    LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnRegister(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnOrder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnNewPage(UINT, int, HWND);
    LRESULT OnDeletePage(UINT, int, HWND);
    LRESULT OnRenamePage(UINT, int, HWND);
    LRESULT OnNextPage(UINT, int, HWND);
    LRESULT OnPrevPage(UINT, int, HWND);

    LRESULT OnWindows800x600(UINT, int, HWND);
    LRESULT OnWindows1024x768(UINT, int, HWND);

    LRESULT OnPlotNormal(UINT, int, HWND);
    LRESULT OnPlotPan(UINT, int, HWND);
    LRESULT OnPlotZoom(UINT, int, HWND);
    LRESULT OnPlotZoomOut(UINT, int, HWND);

    LRESULT OnNotify(UINT, WPARAM, LPARAM);
    LRESULT OnTVNotify(LPNMHDR);

    LRESULT OnHelp(LPHELPINFO);

    //////////////////////////////////////////////////////////////////////////
    //
    //  Project files

    std::string default_ini_path();
    void LoadDefaultTemplate();
    void CreateUsingTemplate(std::string filename);
    void SaveTemplate(std::string filename);

    LRESULT OnCreateUsingTemplate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnLoadTemplate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnSaveTemplate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnTimer(UINT, TIMERPROC=0);

    //////////////////////////////////////////////////////////////////////////
    //
    //  Saving project

    void LoadSomething(std::string filename);
    void LoadProject(std::string filename);
    void SaveProject(std::string filename);

    LRESULT OnLoadProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnSaveProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnSaveProjectAs(WORD, WORD, HWND, BOOL &);
    LRESULT OnMRUProject(int wNotifyCode, int wID, HWND ctrl, BOOL & /*bHandled*/);
    bool    SaveIfNeeded();

    //////////////////////////////////////////////////////////////////////////
    //
    //  Toolbar

    LRESULT OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnFileReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    void    ReloadFile();

    LRESULT OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    void    OpenFile(std::string);

    LRESULT OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);


    LRESULT OnFiltering(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnFiltering2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnMonpacing(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);
    LRESULT OnViewStages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    LRESULT OnResizeWindowX(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/);

    LRESULT OnSelDel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/);

    void ProcessCmdLine();
    void    FixMenuIfRegistered();

    //////////////////////////////////////////////////////////////////////////

    virtual void SetModified(bool);
    virtual bool GetModified() const;
    virtual void Exclamation(std::string);
    virtual void ShowHint(std::string);
    virtual void HideHint();
    virtual void Nag();

    //////////////////////////////////////////////////////////////////////////

    void SetTitle();
    void CheckForUpdate();


    CTileViewFrame *CreateNewTab(std::string);

    HWND create_subwindow(HWND);

    void serialization(serl::archiver &);
    void serial_datafile(serl::archiver &);
    void serial_frame(serl::archiver &);

    CTileViewFrame *GetCurTileView() const;

private:

    void LoadDockingWindowsPosition();

    void set_project_filename(std::string);
    void set_data_filename(std::string);

private:

    std::string the_project_;
    std::string the_datafile_;

    int     dock_win_state_;
    int     ordinary_serial_mode_;

    // GUI
    std::string title_;

    std::vector<shared_ptr<CTileViewFrame> > m_tileViews;
    CTabbedChildWindowX                      m_tabbed;

    aeStatusBar statusBar;

    CCommandBarCtrl m_CmdBar;

    // tooltips
    CToolTipCtrl    tooltip_;

    // docking windows
    sstate::CWindowStateMgr m_stateMgr;
    CChannelWnd     m_ChWnd;
    CStagesWnd      m_StWnd;
    CLocWnd         m_LkWnd;

    bool            modified_;
    bool            destroing_;

    std::vector<language_info> langs_;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__E0BCEF90_0E03_4074_9DAE_C387A66FD589__INCLUDED_)
