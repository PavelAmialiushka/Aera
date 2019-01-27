#include "stdafx.h"

#include "statusbar.h"


void aeStatusBar::Prepare()
{
    SetSimple(false);
    SetWindowLong(GWL_STYLE, GetWindowLong(GWL_STYLE)|WS_CLIPCHILDREN);

    m_progressCtrlSub.Create(m_hWnd, rcDefault, NULL, PBS_SMOOTH|WS_VISIBLE|WS_CHILD);
    m_progressCtrlSub.SetRange(0, 100);

    m_progressCtrlMain.Create(m_hWnd, rcDefault, NULL, PBS_SMOOTH|WS_VISIBLE|WS_CHILD);
    m_progressCtrlMain.SetRange(0, 100);

    AppendPane(0, ID_DEFAULT_PANE, 0);    
    AppendPane(m_progressCtrlSub, IDR_PROGRESS_BAR_SUB, 300);
    AppendPane(m_progressCtrlMain, IDR_PROGRESS_BAR_MAIN, 150);
    AppendPane(0, IDR_HITNUMBER, 100);
    AppendPane(0, IDR_PROGRESSTIME, 110);

    RearrangePanes();
}

LRESULT aeStatusBar::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    Prepare();
    return 0;
}

LRESULT aeStatusBar::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL &bHandled)
{
    while (!panes_.empty())
    {
        ::DestroyWindow(panes_.front().hwnd_);
        panes_.pop_front();
    }
    bHandled=false;
    return 1;
}

LRESULT aeStatusBar::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    _baseClass::OnSize(uMsg, wParam, lParam, bHandled);

    int cx=LOWORD(lParam);
    int cy=HIWORD(lParam);
    if (cx==0 && cy==0)
    {
        int borders[3];
        GetBorders(borders);

        CRect rc;
        ::GetClientRect(GetParent(), rc);
        rc.top=rc.bottom-::GetSystemMetrics(SM_CYHSCROLL);
        ::MoveWindow(m_hWnd, rc.left, rc.top, rc.Width(), rc.Height(), TRUE);
    }

    HDWP hDwp = ::BeginDeferWindowPos( panes_.size() );
    for (unsigned index=0; index<panes_.size(); ++index)
    {
        CRect rc;
        GetPaneRect(panes_[index].id_, rc);

        if (panes_[index].hwnd_)
        {
            ::DeferWindowPos(hDwp, panes_[index].hwnd_, m_hWnd,
                             rc.left, rc.top, rc.Width(), rc.Height(),
                             SWP_NOZORDER);
        }
    }
    ::EndDeferWindowPos( hDwp );

    return 0;
}

void aeStatusBar::AppendPane(HWND hwnd, int id, int width)
{
    aePaneWindow pane;
    pane.width_=width;
    pane.id_=id;
    pane.hwnd_=hwnd;
    panes_.push_back(pane);
    m_ids.push_back(id);
}

void  aeStatusBar::SetPaneWidths(int *arrWidths, int nPanes)
{
    // find the size of the borders
    int arrBorders[3];
    GetBorders(arrBorders);

    // calculate right edge of default pane (0)
    arrWidths[0] += arrBorders[2];
    for (int i = 1; i < nPanes; i++)
        arrWidths[0] += arrWidths[i];

    // calculate right edge of remaining panes (1 thru nPanes-1)
    for (int j = 1; j < nPanes; j++)
        arrWidths[j] += arrBorders[2] + arrWidths[j - 1];

    // set the pane widths
    SetParts(m_nPanes, arrWidths);
}

void aeStatusBar::RearrangePanes()
{
    if (m_ids.size())
    {
        SetPanes(&m_ids[0], m_ids.size());
        std::vector<int> widths(m_ids.size());
        for (unsigned i=0; i<widths.size(); ++i)
        {
            widths[i]=panes_[i].width_;
        }
        SetPaneWidths(&widths[0], widths.size());

        UpdatePanesLayout();
    }
}

void aeStatusBar::SetProgress(int prog, int prog2, std::string status)
{
    if (prog2 != 100) status += "...";

    m_progressCtrlMain.SetPos(prog);
    m_progressCtrlMain.SetText(strlib::strf("%s%%", prog));
    m_progressCtrlMain.Invalidate();

    m_progressCtrlSub.SetPos(prog2);
    m_progressCtrlSub.SetText(status);
    m_progressCtrlSub.Invalidate();

//    status = "\t\t" + status;
//    SetPaneText(ID_DEFAULT_PANE, status.c_str(), 0);
}

void aeStatusBar::SetHitText(std::string title)
{
    SetText(IDR_HITNUMBER, title.c_str());
}

void aeStatusBar::SetProgressTime(std::string time)
{
    SetText(IDR_PROGRESSTIME, time.c_str());
}

BOOL CTextProgressBarCtrl::SubclassWindow(HWND hWnd)
{
    BOOL bRet = _baseClass::SubclassWindow(hWnd);
    return bRet;
}

void CTextProgressBarCtrl::SetText(std::string s)
{
    text_ = s;
    Invalidate();
}

HRESULT CTextProgressBarCtrl::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &)
{
//    LRESULT lResult;
//    _baseClass::ProcessWindowMessage(m_hWnd, uMsg, wParam, lParam, lResult, 0);

    _baseClass::DefWindowProc(uMsg, wParam, lParam);

    CWindowDC dc(m_hWnd);
    CRect rc; GetClientRect(&rc);

    dc.SetTextColor(0x0);
    dc.SetBkMode(TRANSPARENT);

    CFont font; font.CreatePointFont(80, "Tahoma");
    CFontHandle f1 = dc.SelectFont(font);

    dc.DrawTextA(text_.c_str(), text_.size(), rc, DT_CENTER|DT_SINGLELINE|DT_VCENTER);

    dc.SelectFont(f1);
    return 0;
}
