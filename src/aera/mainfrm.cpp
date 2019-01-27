#include "stdafx.h"

#include "mainfrm.h"

#include "dialogs.h"

#include "aboutdlg.h"
#include "CreateDlg.h"
#include "NagDlg.h"
#include "PropertyDlg.h"
#include "registrationDlg.h"
#include "SettingsDlg.h"
#include "RenameDlg.h"

#include "utilites/help.h"

#include "controls/PlotPool.h"
#include "controls/plotWindow.h"
#include "controls/listWindow.h"

#include "TileView/arrangeWnd.h"
#include "others/docking/dbstate.h"

#include "utilites/foreach.hpp"
#include "utilites/tokenizer.h"
#include "utilites/CmdLine.h"
#include "utilites/serl/formats.h"
#include "utilites/serl/serl_registry.h"
#include "utilites/security.h"
#include "utilites/application.h"

#include "wtladdons.h"

#include "data/slice.h"
#include "data/nodefactory.h"
#include "data/processes.h"

#include "FileVersionInfo.h"
#include "httpReader.h"

#ifdef _WIN64
#include "cryptopp70/crc.h"
#include "cryptopp70/sha.h"
#include "cryptopp70/hex.h"
#else
#include "cryptopp/crc.h"
#include "cryptopp/sha.h"
#include "cryptopp/hex.h"
#endif

enum { zaza = 'zaza', xz='xz'};

void buumbabah(HWND);
static void momba(HWND h)
{
    SendMessage(h, WM_TIMER, xz, xz);
}

static std::string complete_filename(std::string name)
{
    if (!fs::is_complete(name))
        name=fs::system_complete( name );
    return name;
}

static void mySetActiveWindow (HWND hWnd)
{
    // http://rsdn.ru/article/qna/ui/wndsetfg.xml

    ::ShowWindow(hWnd, SW_RESTORE);
    ::SendMessage(::GetDesktopWindow(), WM_SYSCOMMAND, (WPARAM) SC_HOTKEY, (LPARAM)hWnd);
    ::SetForegroundWindow(hWnd);
}

static BOOL CALLBACK ActivateWindowByPin(HWND hWnd, LPARAM lParam)
{
    DWORD pin=(DWORD)lParam;

    DWORD process;
    ::GetWindowThreadProcessId(hWnd, &process);

    if (process==pin)
    {
        mySetActiveWindow(hWnd);
    }

    return TRUE;
}

static void ActivateProcess(DWORD pin)
{
    EnumWindows( &ActivateWindowByPin, (LPARAM)pin);
}

static BOOL CALLBACK IsWindowByPinExists(HWND hwnd, LPARAM lParam)
{
    DWORD *pair=(DWORD *)lParam;

    DWORD pin=pair[0];
    DWORD process;
    ::GetWindowThreadProcessId(hwnd, &process);

    if (process==pin)
    {
        std::string text=MakeWindowText( hwnd );
        if (text.find("Aera")!=std::string::npos)
        {
            pair[1]=true;
        }
    }

    return TRUE;
}

static bool ProcessExists(DWORD pin)
{
    DWORD pair[]= {pin, 0};
    EnumWindows( &IsWindowByPinExists, (LPARAM)&pair);
    return pair[1];
}


static std::string only_ext(std::string src)
{
    size_t x=src.find_last_of(".");
    return x!=std::string::npos
           ? src.substr(x+1, src.size())
           : "";
}

static std::string no_ext(std::string src)
{
    size_t x=src.find_last_of(".");
    return x!=std::string::npos
           ? src.substr(0, x)
           : src;
}


class CMutex
{
public:
    CMutex(bool b, const char *name)
    {
        hmutex=::CreateMutex(0, b, name);
    }

    void lock()
    {
        ::WaitForSingleObject(hmutex, INFINITE);
    }

    void unlock()
    {
        ::ReleaseMutex(hmutex);
    }

    ~CMutex()
    {
        unlock();
        ::CloseHandle(hmutex);
    }

    HANDLE hmutex;
};

static bool aquire_use(std::string name, bool activate=true)
{
    CMutex mutex(true, "Aera-filesyncro");

    std::string file_name=utils::get_app_path() + "\\inuse.log";
    std::ifstream file( file_name.c_str() );
    std::string text;
    std::string line;

    std::string cpin=boost::lexical_cast<std::string>(::GetCurrentProcessId());

    name=strlib::to_lower(name);
    while (std::getline(file, line))
    {
        line=strlib::to_lower(line);
        std::string pin = strlib::head(line, "=");
        DWORD ppin = boost::lexical_cast<DWORD>(pin);

        if (pin==cpin) continue;
        if (!ProcessExists(ppin)) continue;

        text += pin + "=" + line + "\n";
        if (line==name)
        {
            // авария
            if (activate)
            {
                ActivateProcess( ppin );
            }

            return false;
        }
    }
    file.close();

    if (!name.empty())
        text+= cpin + "=" + name + "\n";

    std::ofstream out( file_name.c_str() );
    std::copy(STL_II(text), std::ostreambuf_iterator<char>( out ));
    return true;
}


HWND CMainFrame::create_subwindow(HWND parent)
{
    CTVSelectView *view=new CTVSelectView;
    view->AppendWindowType( _lcs("Plot"), new CTVCreatorImpl<CPlotView>() );
    view->AppendWindowType( _lcs("List"), new CTVCreatorImpl<CListWindow>() );

    return view->Create(parent, CWindow::rcDefault, NULL, WS_CHILD|WS_VISIBLE);
}

static void update_progress(HWND m_hWnd)
{
    ::PostMessage(m_hWnd, WM_UPDATEPROGRESS, (WPARAM)0, (LPARAM)0);
}

LRESULT CMainFrame::OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    destroing_ = false;

    dock_win_state_=0xFF;
    debug::structured_exception::install();

    ::LoadLibrary(CRichEditCtrl::GetLibraryName());

    projects_.read_from_registry("Software\\AETestingTools\\Projects");

    InitalizeLocalizator();

    // create command bar window
    HWND hWndCmdBar = m_CmdBar.Create(m_hWnd, rcDefault, NULL, ATL_SIMPLE_CMDBAR_PANE_STYLE);
    // attach menu
    m_CmdBar.AttachMenu(GetMenu());

    // load command bar images
    m_CmdBar.LoadImages(IDR_MAINFRAME);
    // remove old menu
    SetMenu(NULL);

    HWND hWndToolBar = CreateSimpleToolBarCtrl(m_hWnd, IDR_MAINFRAME, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);
    HWND hPlotToolbar = CreateSimpleToolBarCtrl(m_hWnd, IDR_PLOT_TOOLBAR, FALSE, ATL_SIMPLE_TOOLBAR_PANE_STYLE);

    CreateSimpleReBar(ATL_SIMPLE_REBAR_STYLE);
    AddSimpleReBarBand(hWndCmdBar);
    AddSimpleReBarBand(hWndToolBar, NULL, TRUE, 0, true);
    AddSimpleReBarBand(hPlotToolbar, NULL, FALSE, 0, true);
    ::SendMessage(m_hWndToolBar, RB_MINIMIZEBAND, (WPARAM)1, 0);

    CreateSimpleStatusBar();
    statusBar.SubclassWindow(m_hWndStatusBar);
    statusBar.Prepare();

    UIAddToolBar(hWndToolBar);
    UIAddToolBar(hPlotToolbar);
    UIAddMenuBar(hWndCmdBar);
    UISetCheck(ID_VIEW_TOOLBAR, 1);
    UISetCheck(ID_VIEW_PLOT_TOOLBAR, 1);
    UISetCheck(ID_VIEW_STATUS_BAR, 1);
    UISetCheck(IDC_VIEW_CHANNELS, 1);
    UISetCheck(IDC_VIEW_STAGES, 1);
    UISetCheck(IDC_PLOT_NORMAL, 1);

    // register object for message filtering and idle updates
    CMessageLoop *pLoop = _Module.GetMessageLoop();
    ATLASSERT(pLoop != NULL);
    pLoop->AddMessageFilter(this);
    pLoop->AddIdleHandler(this);

    InitializeDockingFrame();

    m_ChWnd.Create(m_hWnd);
    m_StWnd.Create(m_hWnd);
    m_LkWnd.Create(m_hWnd);

    m_tabbed.SetReflectNotifications(true);
    m_tabbed.SetTabStyles(CTCS_TOOLTIPS | CTCS_DRAGREARRANGE | CTCS_BOTTOM);
    m_hWndClient = m_tabbed.Create(m_hWnd, rcDefault);

    CreateNewTab(_lcs("Untitled"));
    CreateNewTab(_lcs("Untitled"));

    SetTitle();

    process::server::instance()->add_signal_update_progress(
        bind(&update_progress, m_hWnd),
        reinterpret_cast<int>(this) );

    SetTimer(0, 50);

    UpdateMenu();

    LoadDockingWindowsPosition();
    FixMenuIfRegistered();

    ProcessCmdLine();
    ShowWindow(SW_SHOWMAXIMIZED);
    Document().SetModified(false);

#ifndef DEMO
    Nag();
#endif

    return 0;
}

void CMainFrame::LoadDockingWindowsPosition()
{
    sstate::CDockWndMgrEx mgrDockWnds(m_hWnd);
    mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CStagesWnd> (m_StWnd));
    mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CChannelWnd>(m_ChWnd));
    mgrDockWnds.Add(sstate::CDockingWindowStateAdapter<CLocWnd>(m_LkWnd));

    m_stateMgr.Initialize("SOFTWARE\\AETestingTools\\Layouts\\DockWindows", m_hWnd);

    m_stateMgr.Add(mgrDockWnds);

    m_stateMgr.Restore();
    UpdateLayout();
}

CTileViewFrame *CMainFrame::CreateNewTab(std::string name)
{
    CTileViewFrame *tileView=new CTileViewFrame();

    tileView->Create(m_tabbed, rcDefault, name.c_str(),
                     WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE);

    m_tabbed.DisplayTab(*tileView);
    m_tabbed.UpdateLayout();

    m_tileViews.push_back(shared_ptr<CTileViewFrame>(tileView));

    tileView->SetCreateWindowCallback( bind(&CMainFrame::create_subwindow, this, _1) );
    tileView->AppendWindows(1);
    tileView->UpdateLayout();
    return tileView;
}

void CMainFrame::ProcessCmdLine()
{
    CmdLine line;
    if (line.file_size())
    {
        std::string name=line.file(0);
        LoadSomething(name);
    } else
    {
        LoadDefaultTemplate();
    }
}

void CMainFrame::FixMenuIfRegistered()
{
    // нельзя сохранять файлы
    if (security::manager().no_save_allowed())
    {
        UIEnable(ID_SAVE_PROJECT, false);
        UIEnable(ID_SAVE_PROJECT_AS, false);
        UIEnable(ID_FILE_SAVE, false);
        UIEnable(ID_SAVE_LAYOUT, false);
    }

    CMenuHandle menu=m_CmdBar.GetMenu();
    CMenuHandle help=menu.GetSubMenu( menu.GetMenuItemCount()-1 );

    for (int index=0; index<help.GetMenuItemCount(); ++index)
    {
        if (help.GetMenuItemID(index)==IDC_REGISTER
                && !security::manager().can_be_registered())
        {
            // ударяем регистрацию
            help.DeleteMenu(index, MF_BYPOSITION);
            --index; continue;
        }

        if (help.GetMenuItemID(index)==IDC_ORDER_SOFTWARE
                && !security::manager().is_unregistered())
        {
            // удаляем сделать заказ
            help.DeleteMenu(index, MF_BYPOSITION);
            help.DeleteMenu(index, MF_BYPOSITION);
        }
    }

    UpdateLayout();
}


LRESULT CMainFrame::OnChangeLanguage(int wNotifyCode, int wID, HWND ctrl, BOOL & /*bHandled*/)
{
    int index=wID-ID_DEFAULTENGLISH;

    if ((unsigned)index < langs_.size())
    {
        localizator::instance()->set_language( langs_[index].ref );
        projects_.set_language( langs_[index].ref );
    }

    CMenuHandle menu=m_CmdBar.GetMenu();

    _lm(menu, IDR_MAINFRAME);

    m_ChWnd.OnChangeLanguage();
    m_StWnd.OnChangeLanguage();
    m_LkWnd.OnChangeLanguage();

    bool temp=GetModified();

    // anew 'entire test' string
    stages::instance()->changed();

    nodes::factory().set_sel_location(
        nodes::factory().get_sel_location());

    SetModified(temp);

    m_CmdBar.AttachMenu(menu);
    SetCheckedLanguage(m_CmdBar.GetMenu());

    projects_.save_to_registry();

    return 0;
}

void CMainFrame::InsertLanguageMenuItem(CMenuHandle langs, language_info const &lang)
{
    int id = ID_DEFAULTENGLISH + langs.GetMenuItemCount();
    langs.AppendMenu(
        MF_STRING, id, lang.name.c_str());

    if (lang.ref == Localizator().get_current_language())
        PostMessage(WM_COMMAND, id, 0);
}

void CMainFrame::InitalizeLocalizator()
{
    std::string lang = projects_.get_language();
    if (lang.size())
        localizator::instance()->set_language(lang);
    langs_ =  localizator::instance()->get_langs();

    // выбираем установленные языки
    CMenuHandle menu=GetMenu();
    CMenuHandle settings=menu.GetSubMenu(menu.GetMenuItemCount()-3);
    CMenuHandle langs=settings.GetSubMenu(0);

    while (langs.GetMenuItemCount())
        langs.RemoveMenu(0, MF_BYPOSITION);

    foreach(language_info const & linfo, langs_)
    {
        InsertLanguageMenuItem(langs, linfo);
    }

}

void CMainFrame::SetCheckedLanguage(CMenuHandle menu)
{
    CMenuHandle settings=menu.GetSubMenu(menu.GetMenuItemCount()-3);
    CMenuHandle langs=settings.GetSubMenu(0);

    MENUITEMINFO mii= {0};
    mii.cbSize=sizeof(mii);
    for (int index=0; index<langs.GetMenuItemCount(); ++index)
    {
        mii.fMask=MIIM_STATE;
        mii.fState=0;
        langs.SetMenuItemInfo(index, true, &mii);
    }

    std::string li = localizator::instance()->get_current_language();
    for (unsigned index=0; index < langs_.size(); ++index)
    {
        language_info const &linfo = langs_[index];
        if (linfo.ref == li)
        {
            mii.fState=MFS_CHECKED;
            langs.SetMenuItemInfo(index, true, &mii);
            break;
        }
    }
}

LRESULT CMainFrame::OnSetFocus(HWND)
{
    if (GetCurTileView())
        GetCurTileView()->SetFocus();
    SetMsgHandled(false);
    return 0;
}

LRESULT CMainFrame::OnLostFocus(HWND)
{
    SetMsgHandled(false);
    return 0;
}

static int on_crt_debug_on_destroy_part( int reportType, char *message, int *returnValue )
{
    ::OutputDebugStringA("Something bad happens");

    *returnValue = 0;
    return TRUE;
}


LRESULT CMainFrame::OnDestroy()
{
    destroing_ = true;
    _CrtSetReportHook(on_crt_debug_on_destroy_part);
    _set_error_mode(_OUT_TO_STDERR);

    if (m_ChWnd) m_ChWnd.DestroyWindow();
    if (m_StWnd) m_StWnd.DestroyWindow();
    if (m_LkWnd) m_LkWnd.DestroyWindow();

    statusBar.Detach();

    SetMsgHandled(false);
    return 0;
}

LRESULT CMainFrame::OnSelDel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    nodes::factory().apply_delete_selection();
    SetTitle();
    return 0;
}

LRESULT CMainFrame::OnSelInvert(UINT, int, HWND)
{
    nodes::factory().modify_selection(selection_invert);
    return 0;
}

LRESULT CMainFrame::OnResizeWindowX(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/)
{
    MoveWindow(CRect(0, 0, 740, 500));
    return 0;
}

LRESULT CMainFrame::OnUpdateProgress(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    // from other thread

    int p;
    std::string status;
    double progress=nodes::factory().get_progress(p, status)*100;
    statusBar.SetProgress( (int)progress, p, status );

    if (process::server::instance()->is_failure())
    {
        nodes::factory().stop_all();

        std::string failure = process::server::instance()->take_failure();
        MessageBox(strlib::strf("Internal error: \"%s\"\nProgram will be closed", failure).c_str(), "Faral error", MB_OK|MB_ICONEXCLAMATION);
        exit(-1);
    }

    std::string title=_ls("N/A");
    std::string time= _ls("N/A");

    nodes::node *node=nodes::factory().get_active_node().get();
    nodes::presult r = process::get_result<nodes::result>(node, false);
    if (r)
    {
        std::string str = "%d " + _ls("hits");
        int size = r->ae->size();
        title = strlib::strf(str.c_str(), size);

        double value = nodes::factory().get_process_time();

        std::string f1 = _ls("s");

        if (value > 2000)
            f1 = "%.0f " + f1;
        else
            f1 = "%.1f " + f1;

        time = strlib::strf(f1.c_str(), value/1000);

        if (size)
        {
            std::string f2 = _ls("ms/hit") + ")";
            if (value / size > 2)
                f2 = " (%.0f " + f2;
            else
                f2 = " (%.1f " + f2;
            time = time + strlib::strf(f2.c_str(), value / size);
        }
    }

    if (statusBar)
    {
        statusBar.SetHitText(title);
        statusBar.SetProgressTime(time);
        statusBar.UpdateWindow();
    }
    return TRUE;
}

LRESULT CMainFrame::OnViewToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    static BOOL bVisible = TRUE;  // initially visible
    bVisible = !bVisible;
    CReBarCtrl rebar = m_hWndToolBar;
    int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 1); // toolbar is 2nd added band
    rebar.ShowBand(nBandIndex, bVisible);
    UISetCheck(ID_VIEW_TOOLBAR, bVisible);
    UpdateLayout();
    return 0;
}

LRESULT CMainFrame::OnViewPlotToolBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    static BOOL bVisible = TRUE;  // initially visible
    bVisible = !bVisible;
    CReBarCtrl rebar = m_hWndToolBar;
    int nBandIndex = rebar.IdToIndex(ATL_IDW_BAND_FIRST + 2); // toolbar is 3nd added band
    rebar.ShowBand(nBandIndex, bVisible);
    UISetCheck(ID_VIEW_PLOT_TOOLBAR, bVisible);
    UpdateLayout();
    return 0;
}

LRESULT CMainFrame::OnViewChannelBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    m_ChWnd.Toggle();
    UISetCheck(IDC_VIEW_CHANNELS, m_ChWnd.IsWindowVisible());
    UpdateLayout();
    return 0;
}

LRESULT CMainFrame::OnViewStagesBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    m_StWnd.Toggle();
    UISetCheck(IDC_VIEW_STAGES, m_StWnd.IsWindowVisible());
    UpdateLayout();
    return 0;
}

LRESULT CMainFrame::OnViewLocationBar(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    m_LkWnd.Toggle();
    UISetCheck(IDC_VIEW_STAGES, m_LkWnd.IsWindowVisible());
    UpdateLayout();
    return 0;
}


LRESULT CMainFrame::OnViewAllBars(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    int a=m_ChWnd.IsWindowVisible();
    int b=m_StWnd.IsWindowVisible();
    if (a || b)
    {
        dock_win_state_=0;
        if (a) dock_win_state_+=1, m_ChWnd.Toggle();
        if (b) dock_win_state_+=2, m_StWnd.Toggle();
    }
    else
    {
        if (dock_win_state_&1) m_ChWnd.Toggle();
        if (dock_win_state_&2) m_StWnd.Toggle();
    }

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

LRESULT CMainFrame::OnFileOpen(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    if (true /*SaveIfNeeded()*/ )
    {
        try
        {
            CFileDialog dlg(true,
                            "*.dta", NULL, 0,
                            "Any data files\0*.dta;*.data;*.naf\0"
                            "DiSP data files (*.dta)\0*.dta;*.data\0"
                            "NAF data files (*.naf)\0*.naf\0"
                            "All files (*.*)\0*.*\0",
                            m_hWnd);

            if (dlg.DoModal()==IDOK)
            {
                OpenFile(dlg.m_szFileName);
                Document().SetModified();
                SetTitle();
            }
        } StopAndLog;
    }
    return 0;
}

void CMainFrame::set_project_filename(std::string string)
{
    if (!fs::is_complete(string))
        the_project_=fs::system_complete( string );
    else
        the_project_ = string;
}

void CMainFrame::set_data_filename(std::string string)
{
    if (!fs::is_complete(string))
        the_datafile_=fs::system_complete( string );
    else
        the_datafile_=string;
}

LRESULT CMainFrame::OnFileReload(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    ReloadFile();
    return 0;
}

void CMainFrame::ReloadFile()
{
    try
    {
        nodes::factory().reload();
    } StopAndLog;
}

void CMainFrame::OpenFile(std::string string)
{
    try
    {
        if (string.size())
        {
            set_data_filename(string);
            string=the_datafile_;

            if (fs::exists(the_datafile_))
            {
                nodes::factory().open( string );
            }
            else
            {
                std::string file = fs::try_to_locate(the_datafile_, the_project_);
                if (!file.empty())
                {
                    OpenFile(file);
                    return;
                }

                std::string text=
                    _ls("notfound1#File is not found")
                        + "\n\""+string+"\"";
                MessageBox(text.c_str(), _lcs("Warning"), MB_OK|MB_ICONWARNING);
                throw std::logic_error("");
            }
        }
        else
        {
            throw std::logic_error("");
        }
    }
    catch (std::logic_error &)
    {
        set_data_filename("");
        nodes::factory().open("");
    };
    GetCurTileView()->Invalidate();
    SetTitle();
}

LRESULT CMainFrame::OnFileSave(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
#ifndef DEMO
    if (security::manager().is_unregistered())
    {
        Nag();
        return 0;
    }

    try
    {
        CFileDialog dlg(false,
                        "*.dta", NULL, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
                        "Any data files\0*.dta;*.data;*.naf\0"
                        "DiSP data files (*.dta)\0*.dta;*.data\0"
                        "NAF data files (*.naf)\0*.naf\0"
                        "All files (*.*)\0*.*\0",
                        m_hWnd);

        if (dlg.DoModal()==IDOK)
        {
            nodes::factory().get_active_node_by_location(object_id())->save_data(dlg.m_szFileName);
        }
    } StopAndLog;
#endif
    return 0;
}

LRESULT CMainFrame::OnFileExit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    PostMessage(WM_CLOSE);
    return 0;
}

LRESULT CMainFrame::OnHelpIndex(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    //help(m_hWnd, "start");
    ShellExecute(0, 0, "http://aetestingtools.com/help", 0, 0 , SW_SHOW );
    return 0;
}

LRESULT CMainFrame::OnEditAll(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    m_ChWnd.ClickAll();
    return 0;
}

LRESULT CMainFrame::OnEditInvert(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    m_ChWnd.ClickInvert();
    return 0;
}

LRESULT CMainFrame::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    CAboutDlg dlg;
    dlg.DoModal();

    FixMenuIfRegistered();

    return 0;
}

LRESULT CMainFrame::OnRegister(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    CRegistrationDlg dlg;
    dlg.DoModal();

    FixMenuIfRegistered();
    SetTitle();

    return 0;
}

LRESULT CMainFrame::OnOrder(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    //help(m_hWnd, "order");
    return 0;
}

LRESULT CMainFrame::OnFileNew(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    return 0;
}

LRESULT CMainFrame::OnFiltering2(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    Dialogs::FilterHistory();
    SetTitle();

    return 0;
}

LRESULT CMainFrame::OnFiltering(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    Dialogs::QuickFilter();
    SetTitle();

    return 0;
}

LRESULT CMainFrame::OnMonpacing(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    Dialogs::Monpac();
    SetTitle();

    return 0;
}

LRESULT CMainFrame::OnViewStages(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    Dialogs::Stages();
    SetTitle();

    return 0;
}

void CMainFrame::serialization(serl::archiver &ar)
{
    // serialization of data
    nodes::factory().set_serial_mode(
        ordinary_serial_mode_ ? nodes::ordinary_mode : nodes::layout_mode);
    ar.serial("nodes", nodes::factory());

    if (ordinary_serial_mode_)
        serial_datafile(ar);

    // serialization of view
    serial_frame(ar);
}

void CMainFrame::serial_frame(serl::archiver &ar)
{
    // serialization of tab
    //
    scoped_ptr<serl::archiver> sub( ar.clone("frame") );

    // serialization of tab names
    unsigned count=m_tabbed.GetTabCtrl().GetItemCount();
    std::vector<std::string> tabs;
    for (unsigned index=0; index<count; ++index)
    {
        tabs.push_back( m_tabbed.GetTabCtrl().GetItem(index)->GetTextRef() );
    }
    sub->serial_container("tabs", tabs);

    // serialization of tab contents
    if (sub->is_loading())
    {
        while (m_tileViews.size())
        {
            m_tileViews.back()->DestroyWindow();
            m_tileViews.pop_back();
        }
        while (m_tabbed.GetTabCtrl().GetItemCount())
        {
            m_tabbed.GetTabCtrl().DeleteItem((size_t)0);
        }
    }
    unsigned index=0;
    foreach(std::string name, tabs)
    {
        CTileViewFrame *frame;
        if (sub->is_loading())
        {
            frame =CreateNewTab(name);
            CWindowText(frame->m_hWnd)=name;
        }
        else
        {
            // находим вид соответвтующий номеру таба
            HWND hwnd=m_tabbed.GetView(index);
            foreach(shared_ptr<CTileViewFrame> view, m_tileViews)
            {
                if (*view==hwnd)
                {
                    frame=view.get();
                    break;
                }
            }
        }

        sub->serial("tab#"+boost::lexical_cast<std::string>(index), *frame);
        if (sub->is_loading())
            m_tabbed.DisplayTab(*frame);
        ++index;
    }

    if (sub->is_loading())
        m_tabbed.DisplayTab(*m_tileViews[0]);
}

void CMainFrame::serial_datafile(serl::archiver &ar)
{
    std::string data=fs::complete(the_datafile_, the_project_);
    ar.serial("datafile", data);

    if (ar.is_loading())
    {
        std::string dx=data;
        the_datafile_=
            fs::is_complete(dx) ? dx :
            fs::complete(dx, strlib::rhead(the_project_, "\\"));
        OpenFile( the_datafile_ );
    }
}

void CMainFrame::LoadProject(std::string filename)
{
    try
    {
        shared_ptr<process::suspend_lock> lock =
                nodes::factory().block_execution();

        std::string opr=the_project_;
        set_project_filename(filename);

        ordinary_serial_mode_=true;
        bool loading =
                serl::load_from_file( the_project_, serl::format_data, *this)
                ||
                serl::load_from_file( the_project_, serl::format_text, *this)
                ;

        if (loading)
        {
            projects_.add_to_list( the_project_ );
            UpdateMenu();
        }
        else
        {
            the_project_=opr;
        }

        SetModified(false);
        SetTitle();

    } LogException("LoadProject");
}

void CMainFrame::SaveProject(std::string filename)
{
#ifndef DEMO
    try
    {
        std::string opr=the_project_;

        DWORD attribs=GetFileAttributes( filename.c_str() );
        if (attribs!=INVALID_FILE_SIZE &&
                (attribs&FILE_ATTRIBUTE_READONLY)!=0)
        {
            SendMessage(WM_COMMAND, ID_SAVE_PROJECT_AS, 0);
            return;
        }

        set_project_filename(filename);
        ordinary_serial_mode_=true;

        int format = serl::format_data;
        if (filename.find(".aera_text")!=std::string::npos)
            format = serl::format_text;

        if (serl::save_to_file( the_project_, format, *this))
        {
            projects_.add_to_list( the_project_ );
            UpdateMenu();
        }
        else
        {
            set_project_filename(opr);
            MessageBox(_lcs("Error writing file"), NULL, MB_ICONERROR);
            SendMessage(WM_COMMAND, ID_SAVE_PROJECT_AS, 0);
            return;
        }

        SetTitle();

    } LogException("SaveProject");
#endif
}


LRESULT CMainFrame::OnLoadTemplate(WORD, WORD, HWND, BOOL &)
{
    CFileDialog dlg(true,
                    "*.lyt", NULL, 0,
                    "Aera template files(*.lyt)\0*.lyt\0",
                    m_hWnd);

    if (dlg.DoModal()==IDOK)
    {
        LoadSomething(dlg.m_szFileName);
    }

    return 0;
}


void CMainFrame::LoadSomething(std::string filename)
{
    std::string ext = strlib::to_lower(only_ext(filename));
    if (std::string(".dta.data.naf").find(ext) != std::string::npos)
    {
        LoadDefaultTemplate();
        OpenFile(filename);
        Document().SetModified();
        SetTitle();
    }
    else if (std::string(".lyt").find(ext) != std::string::npos)
    {
        std::string file = the_datafile_;
        CreateUsingTemplate(filename);
        OpenFile(file);
        SetTitle();
    }
    else if (aquire_use( complete_filename(filename)))
    {
        LoadProject(filename);
    } else
    {
        SendMessage(WM_CLOSE, 0, 0);
    }
}


LRESULT CMainFrame::OnLoadProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    try
    {
        CFileDialog dlg(true,
                        "*.aera", NULL, OFN_HIDEREADONLY,
                        "Any project or data files\0*.dta;*.data;*.naf;*.aera;*.aera_text;*.lyt\0"
                        "Aera project files (*.aera)\0*.aera;*.aera_text\0"
                        "Aera template files (*.lyt)\0*.lyt\0"
                        "DiSP data files (*.dta)\0*.dta;*.data\0"
                        "NAF data files (*.naf)\0*.naf\0"
                        "All files (*.*)\0*.*\0",

                        m_hWnd);

        if (dlg.DoModal()==IDOK)
        {
            LoadSomething(dlg.m_szFileName);
        }

    } StopAndLog;
    return 0;
}

LRESULT CMainFrame::OnSaveProject(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL &bHandled)
{
    if (security::manager().is_unregistered())
    {
        Nag();
        return 0;
    }

    if (the_project_.empty())
    {
        return SendMessage(WM_COMMAND, ID_SAVE_PROJECT_AS, 0);
    }

    if (Document().GetModified())
    {
        SaveProject( the_project_ );
    }
    return 0;
}

LRESULT CMainFrame::OnSaveProjectAs(WORD, WORD, HWND, BOOL &)
{
    if (security::manager().is_unregistered())
    {
        Nag();
        return 0;
    }

    try
    {
        std::string dir=(
                            the_project_.empty()
                            ? strlib::rhead(the_datafile_, ".")
                            : the_project_);

        CFileDialog dlg(false,
                        "*.aera",
                        dir.c_str(), OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
                        "Aera project files(*.aera)\0*.aera\0"
#ifndef NDEBUG
                        "Aera text project files (*.aera_text)\0*.aera_text\0"
#endif
                        "All files (*.*)\0*.*\0",
                        m_hWnd);

        for (;;)
        {
            if (dlg.DoModal()!=IDOK) break;
            if (aquire_use( complete_filename(dlg.m_szFileName), false))
            {
                SaveProject(dlg.m_szFileName);
                break;
            }
            else
            {
                MessageBox(_lcs("MF-file-inuse#Cannot write to file.\nFile is opened by another instance of Aera"), "Warning", MB_OK|MB_ICONWARNING);
            }
        }

    } StopAndLog;
    return 0;
}

LRESULT CMainFrame::OnMRUProject(int wNotifyCode, int wID, HWND ctrl, BOOL & /*bHandled*/)
{
    try
    {
        int id=wID-ID_MRU_PROJECTS1;
        std::string name=projects_.get_item(id);
        if (SaveIfNeeded())
        {
            if (aquire_use( complete_filename( name )))
                LoadProject( name );
            else
                SendMessage(WM_CLOSE, 0, 0);
        }
    } StopAndLog;
    return 0;
}

//////////////////////////////////////////////////////////////////////////

std::string CMainFrame::default_ini_path()
{
    return (utils::get_exefile_path("default.lyt") );
}

void CMainFrame::LoadDefaultTemplate()
{
    try
    {
        shared_ptr<process::suspend_lock> lock =
                nodes::factory().block_execution();

        if (fs::exists(default_ini_path()))
        {
            CreateUsingTemplate(default_ini_path().c_str());
        }
        else
        {
            CreateUsingTemplate(fs::get_exefile_path("default.lyt").c_str());
        }
    } StopAndLog;    
}

namespace
{

struct serial_frame_helper
{
    serial_frame_helper(CMainFrame *self) : self(self) {}

    void serialization(serl::archiver &ar)
    {
        nodes::factory().set_serial_mode(nodes::layout_mode);
        ar.serial("nodes", nodes::factory());
        self->serial_frame(ar);
    }

private:
    CMainFrame *self;
};

}

void CMainFrame::CreateUsingTemplate(std::string filename)
{
    ordinary_serial_mode_=false;

    bool loading=filename.empty()
                 || serl::load_from_file(filename, serl::format_data, *this)
                 || serl::load_from_file(filename, serl::format_text, *this);
    if (loading)
    {
        OpenFile("");

        while ( stages::instance()->size() )
        {
            stages::instance()->remove(0);
        } stages::instance()->changed();

        the_project_=std::string();
        nodes::factory().create_new();
    }

    SetModified(false);
}


void CMainFrame::SaveTemplate(std::string filename)
{
#ifndef DEMO
    try
    {
        ordinary_serial_mode_=false;
        if (serl::save_to_file(filename, serl::format_data, *this))
        {
            // ok
        }

    } LogException("save_ini");
#endif
}


LRESULT CMainFrame::OnCreateUsingTemplate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    if (SaveIfNeeded())
    {
        aquire_use( "" );
        try
        {
            CCreateDlg dlg;

            switch (dlg.DoModal())
            {
            case IDC_CR_DEFAULT:
                LoadDefaultTemplate();
                break;
            case IDC_CR_CURRENT:
                CreateUsingTemplate("");
                break;
            case IDC_CR_OTHER:
            {
                BOOL r;
                OnLoadTemplate(0,0,0,r);
            } break;
            }

        } StopAndLog;
    }
    return 0;
}

LRESULT CMainFrame::OnSaveTemplate(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    try
    {
        CFileDialog dlg(false,
                        "*.lyt",
                        NULL,
                        OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
                        "Aera template files(*.lyt)\0*.lyt\0All files (*.*)\0*.*\0",
                        m_hWnd);

        if (dlg.DoModal()==IDOK)
        {
            SaveTemplate(dlg.m_szFileName);
        }

    } StopAndLog;
    return 0;
}

//////////////////////////////////////////////////////////////////////////

LRESULT CMainFrame::OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
    return GetCurTileView()->SendMessage(uMsg, wParam, lParam);
}

void CMainFrame::SetTitle()
{
    std::string project=strlib::rtail(the_project_, "\\");
    std::string file =  strlib::rtail(the_datafile_, "\\");

    title_= "";
#ifndef DEMO
    if (GetModified())
        title_ += "* ";
#endif
    title_+= project + (project.empty() ? _ls("Unititled") : "") +" - " ;
    title_+= "Aera";
#if _WIN64
    title_ += " x64";
#endif
    title_+= file.empty() ? "" : (" - [ " +file+ " ]");

    if (security::manager().is_unregistered())
    {
        title_ += " - "+_ls("Unregistered-version#unregistered version");
    }
#ifdef DEMO
    title_ += " - "+_ls("demo-version#demo version");
#endif

    SetWindowText(title_.c_str());
}

CTileViewFrame *CMainFrame::GetCurTileView() const
{
    HWND active=m_tabbed.GetActiveView();
    foreach(shared_ptr<CTileViewFrame> view, m_tileViews)
    {
        if (*view==active) return view.get();
    }

    return 0;
}

LRESULT CMainFrame::OnNewPage(UINT, int, HWND)
{
    CreateNewTab(_lcs("Untitled"));
    Document().SetModified();
    return 0;
}

LRESULT CMainFrame::OnRenamePage(UINT, int, HWND)
{
    HWND  view=m_tabbed.GetActiveView();
    std::string name=CWindowText(view);

    CRenameDlg dlg;
    if (dlg.DoModal(GetActiveWindow(), (LPARAM)&name)==IDOK)
    {
        CWindowText tx(view); tx=name;
        m_tabbed.UpdateTabText(view, name.c_str());
        Document().SetModified();
    }
    return 0;
}

LRESULT CMainFrame::OnDeletePage(UINT, int, HWND)
{
    if (m_tabbed.GetTabCtrl().GetItemCount()>1)
    {
        HWND hwnd=m_tabbed.GetActiveView();
        m_tabbed.RemoveTab(hwnd);

        for (unsigned index=0; index<m_tileViews.size(); ++index)
        {
            shared_ptr<CTileViewFrame> view=m_tileViews[index];
            if (*view==hwnd)
            {
                m_tileViews.erase(
                    std::remove(STL_II(m_tileViews), view),
                    m_tileViews.end());
                view->DestroyWindow();
                Document().SetModified();
                break;
            }
        }
    }

    return 0;
}

LRESULT CMainFrame::OnNextPage(UINT, int, HWND)
{
    int index=m_tabbed.GetTabCtrl().GetCurSel();
    ++index;
    if (index>=m_tabbed.GetTabCtrl().GetItemCount()) index=0;
    m_tabbed.GetTabCtrl().SetCurSel(index);
    return 0;
}

LRESULT CMainFrame::OnPrevPage(UINT, int, HWND)
{
    int index=m_tabbed.GetTabCtrl().GetCurSel();
    if (index==0) index=m_tabbed.GetTabCtrl().GetItemCount();
    --index;
    m_tabbed.GetTabCtrl().SetCurSel(index);
    return 0;
}

LRESULT CMainFrame::OnNotify(UINT, WPARAM wParam, LPARAM lParam)
{
    int id=(int)wParam;
    LPNMHDR pnmh = (LPNMHDR)lParam;

    if (pnmh->code >= TVFN_FIRST && pnmh->code <=TVFN_LAST)
    {
        return OnTVNotify(pnmh);
    }
    else
    {
        switch (pnmh->code)
        {
        case CTCN_SELCHANGE:
            id=0;
            break;
        }
    }
    SetMsgHandled(false);
    return 0;
}

bool CMainFrame::SaveIfNeeded()
{
#ifndef DEMO
    if (security::manager().is_unregistered())
    {
        Nag();
        return true;
    }

    if (!Document().GetModified())
    {
        return true;
    }
    else
    {
        std::string text=
            _lcs("Your current project has not been saved\nSave your project now?");
        int reslt=0;
        int res=MessageBox(_lcs(text), _lcs("Warning"), MB_YESNOCANCEL|MB_ICONWARNING);
        switch (res)
        {
        case IDYES:
            OnSaveProject(0, 0, 0, reslt);
        case IDNO:
            return true;
        default:
            return false;
        }
    }
#else
    return true;
#endif
}

LRESULT CMainFrame::OnClose()
{
    aquire_use( "" );

    m_stateMgr.Store();

    if (SaveIfNeeded())
    {
        SetMsgHandled(false);
    }
    return 0;
}

void CMainFrame::SetModified(bool flag)
{
    modified_=flag;
    SetTitle();
}

bool CMainFrame::GetModified() const
{
    return modified_;
}

void CMainFrame::Exclamation(std::string string)
{
    /** called from other thread */

    static char buffer[1024];
    strncpy(buffer, string.c_str(), SIZEOF(buffer));

    ::PostMessage(m_hWnd, WM_EXCLAMATION, (WPARAM)buffer, (LPARAM)std::min(SIZEOF(buffer), string.size()));
}

LRESULT CMainFrame::OnExclamation(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
    OpenFile("");

    std::string text=std::string((char *)wParam, (int)lParam);
    MessageBox(text.c_str(), _lcs("Warning"), MB_OK|MB_ICONWARNING);
    return 0;
}

LRESULT CMainFrame::OnTVNotify(LPNMHDR hdr)
{
    if (hdr->code==TVFN_WINDOWADDED ||
            hdr->code==TVFN_WINDOWREMOVED)
    {
        Document().SetModified(true);
    }
    return 0;
}

LRESULT CMainFrame::OnOptions(UINT, int, HWND)
{
    Nag();

    CSettingsDlg dlg;
    dlg.DoModal();

    return 0;
}

LRESULT CMainFrame::OnArrangeWindows(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    GetCurTileView()->ShowArrangeWindow();
    return 0;
}

LRESULT CMainFrame::OnAddArrangeWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    GetCurTileView()->AddArrangeWindow();
    return 0;
}

LRESULT CMainFrame::OnTimer(UINT, TIMERPROC)
{
    ::SendMessage(m_hWnd, WM_UPDATEPROGRESS, (WPARAM)0, (LPARAM)0);

    CheckForUpdate();

    return 0;
}

void CMainFrame::ShowHint(std::string text)
{
    CToolInfo info(
        TTF_TRACK|TTF_ABSOLUTE , m_hWnd, 0, 0,
        (char *)text.c_str());

    tooltip_.Create(m_hWnd);
    tooltip_.AddTool(info);

    CRect rc; statusBar.GetWindowRect(rc);

    tooltip_.TrackPosition(rc.left, rc.top);
    tooltip_.TrackActivate(info, true);
}

void CMainFrame::HideHint()
{
    if (tooltip_)
    {
        tooltip_.TrackActivate(CToolInfo(0, m_hWnd, 0), false);
        tooltip_.DestroyWindow();
    }
}

LRESULT CMainFrame::OnFileProperties(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL & /*bHandled*/)
{
    CPropertyDlg dlg;
    dlg.DoModal();
    return 0;
}

static void buumbabah(HWND h)
{
    //Document().SetModified(false);
    //PostMessage(h, WM_CLOSE, zaza, zaza);

    function0<void> fn(bind(&momba, h)); fn();
}

BOOL CMainFrame::OnIdle()
{
    if (destroing_) return FALSE;

    UISetCheck(IDC_VIEW_CHANNELS, m_ChWnd.IsWindowVisible());
    UISetCheck(IDC_VIEW_STAGES, m_StWnd.IsWindowVisible());
    UISetCheck(IDC_VIEW_LOCATION, m_LkWnd.IsWindowVisible());

    UISetCheck(IDC_PLOT_NORMAL, !PlotManager::inst().IsZoomMode() &&
                                !PlotManager::inst().IsPanMode());
    UISetCheck(IDC_PLOT_PAN,    PlotManager::inst().IsPanMode());
    UISetCheck(IDC_PLOT_ZOOMIN, PlotManager::inst().IsZoomMode());

    UISetState(IDC_PLOT_ZOOMOUT,PlotManager::inst().CanZoomBack() ? UPDUI_ENABLED : UPDUI_DISABLED);

    UIUpdateToolBar();

    return FALSE;
}

LRESULT CMainFrame::OnHotKey(int id, UINT, UINT)
{
    switch (id)
    {
    case VK_TAB:
        SendMessage(WM_COMMAND, IDC_VIEW_ALLTAB, 0);
        break;
    }

    return 0;
}

LRESULT CMainFrame::OnFocusOnActiveView(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    ::SetFocus(m_tabbed.GetActiveView());
    return 0;
}

void CMainFrame::Nag()
{
#ifndef DEMO
    CNagDlg().DoModal();
#endif
}

LRESULT CMainFrame::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "start");
    return 0;
}

LRESULT CMainFrame::OnPlotNormal(UINT, int id, HWND)
{
    PlotManager::inst().SetPanMode(false);
    return 0;
}

LRESULT CMainFrame::OnPlotPan(UINT, int id, HWND)
{
    PlotManager::inst().SetPanMode(true);
    return 0;
}

LRESULT CMainFrame::OnPlotZoom(UINT, int id, HWND)
{
    PlotManager::inst().SetZoomMode(true);
    return 0;
}

LRESULT CMainFrame::OnPlotZoomOut(UINT, int, HWND)
{
    PlotManager::inst().ZoomBack();
    return 0;
}

LRESULT CMainFrame::OnWindows800x600(UINT, int, HWND)
{
    MoveWindow(0, 0, 800, 600);
    return 0;
}

LRESULT CMainFrame::OnWindows1024x768(UINT, int, HWND)
{
    MoveWindow(0, 0, 1024, 768);
    return 0;

}

static bool IsTimeForCheckForUpdate()
{
    // в эту функцию можно войти только один раз за запуск программы
    static bool only_once = 0;
    if (only_once) return false;
    only_once = 1;

    time_t check_time = 0;
    serl::load_archiver(
        new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data"))
    .serial("checkForUpdate", check_time);

    tm c0 = *localtime(&check_time);

    check_time = time(0);
    tm c1 = *localtime(&check_time);

    if ( c0.tm_year == c1.tm_year
         && c0.tm_yday == c1.tm_yday)
    {
        return false;
    }

    serl::save_archiver(
        new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data"))
    .serial("checkForUpdate", check_time);

    return true;
}

static std::string calculateCRC(const std::string& data)
{
   std::string result;
   CryptoPP::CRC32 hash;
   CryptoPP::StringSource(data,true,
        new CryptoPP::HashFilter(hash,
        new CryptoPP::HexEncoder(
        new CryptoPP::StringSink(result))));
   return result;
}

static std::string check_update_url;
static void DoCheckForUpdate()
{
    CHTTPReader rd("aetestingtools.com");
    rd.SetDataBuffer(1048576);

    if (rd.Get(check_update_url.c_str()))
    {
        // проверять версию научимся позже, когда доделаем сайт
//        std::string data = rd.GetData();
//        OutputDebugString(data.c_str());
    } else
    {
        rd.ReportError();
    }
}

void CMainFrame::CheckForUpdate()
{
    if (!IsTimeForCheckForUpdate())
        return;

    FileVersionInfo vi(utils::get_exefile_name());
    std::string ver=vi["ProductVersion"];
    ver=strlib::replace(ver, " ", "");
    ver=strlib::replace(ver, ",", "_");

    std::string info=security::manager().get_registration_info();
    std::string user=calculateCRC(info);

    check_update_url = strlib::strf("/check_for_update?cv=%s&u=%s",
                                    ver, user);

    boost::thread(boost::function0<void>(&DoCheckForUpdate));
}
