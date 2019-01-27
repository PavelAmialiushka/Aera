#ifndef STATUSBAR_E3FB4A02_7439_4620_A1D2_59F3DB1A835F
#define STATUSBAR_E3FB4A02_7439_4620_A1D2_59F3DB1A835F

//////////////////////////////////////////////////////////////////////////
/*
// Получаем указатель на главное окно.
CMainFrame *pFrame = dynamic_cast<CMainFrame *>(AfxGetMainWnd());

// Находим объект строки состояния.
CStatusBar &sb = pFrame->m_wndStatusBar;

// Определяем прямоугольник, в котором будет размещаться индикатор прогресса.
// В нашем примере он будет занимать всю первую панель строки состояния.
CRect rect;
sb.GetItemRect(0, rect);

// Создаём индикатор прогресса.
CProgressCtrl pc;
pc.Create(WS_CHILD | WS_VISIBLE, rect, &sb, 0);
pc.SetRange(0, 100);
pc.SetPos(0);
pc.SetStep(1);
*/

struct aePaneWindow
{
    int   width_;
    int   id_;
    HWND  hwnd_;
};


//////////////////////////////////////////////////////////////////////////

typedef CWinTraits<WS_CHILD|WS_BORDER|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|CBS_DROPDOWN, 0> CTextProgressBarCtrlTraits;

class CTextProgressBarCtrl
        : public CWindowImpl<CTextProgressBarCtrl, CProgressBarCtrl >
{
public:
    DECLARE_WND_SUPERCLASS(_T("TextProgressBar"), GetWndClassName())

    typedef CWindowImpl<CTextProgressBarCtrl, CProgressBarCtrl > _baseClass;

    std::string text_;
public:

    BOOL SubclassWindow(HWND hWnd);

    BEGIN_MSG_MAP(CTextProgressBarCtrl)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
    END_MSG_MAP()

    void SetText(std::string s);

    HRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &/*bHandled*/);
};

//////////////////////////////////////////////////////////////////////////

class aeStatusBar :
    public WTL::CMultiPaneStatusBarCtrlImpl<aeStatusBar>
{
public:
    DECLARE_WND_SUPERCLASS(_T("WTL_MultiPaneStatusBar"), GetWndClassName())

    typedef WTL::CMultiPaneStatusBarCtrlImpl<aeStatusBar> _baseClass;

    BOOL PreTranslateMessage(MSG *pMsg)
    {
        pMsg;
        return FALSE;
    }

    BEGIN_MSG_MAP(aeStatusBar)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        CHAIN_MSG_MAP(_baseClass)
    END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//  LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//  LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//  LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)


private:
    enum {ID_DEFAULT=ID_DEFAULT_PANE, IDR_PROGRESS_BAR_SUB, IDR_PROGRESS_BAR_MAIN, IDR_HITNUMBER, IDR_PROGRESSTIME};

    std::deque<aePaneWindow>  panes_;

    CTextProgressBarCtrl m_progressCtrlMain;
    CTextProgressBarCtrl m_progressCtrlSub;
    std::vector<int> m_ids;

public:

    void Prepare();
    void SetProgress(int prog, int p2, std::string status);
    void SetHitText(std::string title);
    void SetProgressTime(std::string time);

private:

    void SetPaneWidths(int *arrWidths, int nPanes);
    void AppendPane(HWND hwnd, int id, int width);
    void RearrangePanes();

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
    LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled);

    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);


};

#endif // STATUSBAR_E3FB4A02_7439_4620_A1D2_59F3DB1A835F
