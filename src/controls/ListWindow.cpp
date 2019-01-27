#include "stdafx.h"

#include "aera/resource.h"

#include "ListWindow.h"
#include "ListProperties.h"

#include "Tileview/TVFrame.h"

#include "data/nodefactory.h"

#include "utilites/localizator.h"
#include "utilites/document.h"

serl::mark_as_rtti< CTVCreatorImpl<CListWindow> > INSTANCE;

CListWindow::CListWindow()
    : pageline_(0),
      lineno_(0),
      wide_(0),
      wpos_(0),
      text_extent_(0, 0),
      preffered_time_(-1),
      text_length_(0)
{
    model_.attach_listener(this);
}

CListWindow::~CListWindow()
{
    model_.detach_listener(this);

    if (m_hWnd)
        DestroyWindow();
}

//////////////////////////////////////////////////////////////////////////

int CListWindow::get_line() const
{
    return lineno_;
}

int CListWindow::get_pos() const
{
    return lineno_+pageline_;
}

int CListWindow::get_wpos() const
{
    return wpos_;
}

int CListWindow::get_page() const
{
    return pageline_;
}

int CListWindow::get_text_width() const
{
    return get_text_length()*text_extent_.cx;
}

int CListWindow::get_line_at_point(CPoint point) const
{
    int pos=text_extent_.cy
            ? point.y/text_extent_.cy-1
            : get_line();
    return std::max(0, std::min(pos, lines()-1));
}

int CListWindow::lines() const
{
    return text_extent_.cy
           ? get_rect().Height()/text_extent_.cy-1
           : 0u;
}

const CRect &CListWindow::get_rect() const
{
    return rect_;
}

int CListWindow::get_max_wpos() const
{
    return get_text_length()*text_extent_.cx;
}

int CListWindow::effwidth() const
{
    return get_rect().Width();
}

int CListWindow::effheight() const
{
    return get_rect().Height();
}

int CListWindow::get_text_char_height() const
{
    return text_extent_.cy;
}

int CListWindow::get_text_char_width() const
{
    return text_extent_.cx;
}


int CListWindow::get_text_length() const
{
    return text_length_;
}

void CListWindow::set_text_length(int x)
{
    text_length_=x;
}

void CListWindow::set_promptstring(std::string s)
{
    promptstring_=s;
}

std::string CListWindow::get_promptstring() const
{
    return promptstring_;
}


void CListWindow::check_borders()
{
    update_scrollbars();
}

LRESULT CListWindow::OnEraseBackground(HDC)
{
    return 0;
}


LRESULT CListWindow::OnPaint(HDC hdc)
{
    CRect rc(0, 0, effwidth(), effheight());
    CPaintDC dc(m_hWnd);
    return OnPaint(dc, rc, false);
}

LRESULT CListWindow::OnPaint(HDC hdc, CRect rc, bool copyclip)
{    
    try
    {
        CDCHandle real_dc(hdc);
        CDC dc( ::CreateCompatibleDC(real_dc) );
        CBitmap bitmap( ::CreateCompatibleBitmap(real_dc, rc.Width(), rc.Height()) );
        dc.SelectBitmap(bitmap);

        CFont pf; pf.CreatePointFont(100, "Courier");
        CFont oldFont( dc.SelectFont(pf) );

        dc.GetTextExtent("0", 1, &text_extent_);

        check_borders();

        dc.SetTextColor(!copyclip ? 0x505050 : 0x0);
        dc.SetBkColor(!copyclip ? 0xC0F0F0 : 0xFFFFFF);
        dc.FillSolidRect(0, 0, rc.Width(), get_text_char_height(), !copyclip ? 0xC0F0F0 : 0xFFFFFF);

        std::string header=model_.get_header();

        dc.TextOut(-get_wpos(), 0, header.c_str());
        set_text_length(header.length());

        dc.FillSolidRect(0, get_text_char_height(), rc.Width(), rc.Height(), !copyclip ? 0xDADADA : 0xFFFFFF);
        for (int line=0; line<lines(); ++line)
        {
            bool isSubHit = false;
            int index=line+get_page();

            std::string string_line = model_.line( index, isSubHit );

            int clr, bk;

            // normal line
            clr = !copyclip ? 0x804000 : 0x00000;
            bk = !copyclip ? 0xDADADA : 0xFFFFFF;

            if (line==get_line() && !copyclip) // selected line
            {
                if (has_focus())
                {
                    bk = 0x7F0000;
                    clr = isSubHit ? 0x7F7FFF : 0x00FFFF;
                }
                else
                {
                    bk = 0x7F7F7F;
                    clr = 0xFFFFFF;
                }
                dc.FillSolidRect(0, (line+1)*get_text_char_height(), rc.Width(), get_text_char_height(), bk);
            }
            else if (isSubHit)
            {
                clr = !copyclip ?  0x1000da : 0x000000;
                bk = !copyclip ? 0xb89a9a : 0xCFCFCF;

                dc.FillSolidRect(0, (line+1)*get_text_char_height(), rc.Width(), get_text_char_height(), bk);
            }

            dc.SetTextColor(clr);
            dc.SetBkColor(bk);

            dc.TextOut(-get_wpos(), (line+1)*get_text_char_height(), string_line.c_str());
        }

        if (!get_promptstring().empty())
        {
            dc.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
            dc.SetBkColor(::GetSysColor(COLOR_WINDOW));
            std::string prompt="Search: "+get_promptstring();
            CRect border(
                0,
                rc.Height()-get_text_char_height()-3,
                prompt.length()*get_text_char_width()+4,
                rc.Height());

            dc.TextOut(border.left+2, border.top+2, prompt.c_str());
            dc.DrawEdge(border, EDGE_SUNKEN, BF_RECT);
        }

        dc.MoveTo(CPoint(0, get_text_char_height()));
        dc.LineTo(CPoint(rc.Width(), get_text_char_height()));

        if (copyclip)
        {
            CPen pen( :: CreatePen(PS_SOLID, 1, 0x0) );
            CPen open=dc.SelectPen(pen);
            dc.MoveTo(CPoint(rc.right, rc.top));
            dc.LineTo(CPoint(rc.left, rc.top));
            dc.LineTo(CPoint(rc.left, rc.bottom-1));
            dc.LineTo(CPoint(rc.right-1, rc.bottom-1));
            dc.LineTo(CPoint(rc.right-1, rc.top));
            dc.SelectPen(open);
        }

        real_dc.BitBlt(rc.left, rc.top,
                       rc.Width() ,
                       rc.Height(),
                       dc, 0, 0, SRCCOPY);

//        real_dc.FillSolidRect(
//            rc.Width(), rc.Height(), get_rect().right, get_rect().bottom,
//            copyclip ? 0xFFFFFF : ::GetSysColor(COLOR_3DFACE));


    } LogException("CListWindow::paint");
    return 0;
}



//
// updates cursor position
//
void CListWindow::safe_on_finish(process::prslt)
{
    if (preffered_time_>=0)
    {
        center_on( model_.search_time(preffered_time_) );
    }

    update_scrollbars();
    set_vscroll_pos();
    Invalidate();
}

void CListWindow::safe_on_restart()
{
    Invalidate();
}

void CListWindow::locate_time()
{
    double time=atof(get_promptstring().c_str());
    center_on( model_.search_time(time) );
}

void CListWindow::update_scrollbars()
{
    set_vscroll_pos();
    set_hscroll_pos();
}

void CListWindow::set_vscroll_pos()
{
    SCROLLINFO inf= {sizeof(SCROLLINFO)};
    inf.nPage=lines();
    inf.fMask=SIF_ALL;
    inf.nMin=0;        
    inf.nMax=0;
    inf.nPos=0;

    if (model_.valid())
    {
        inf.nMax=(model_.size() >= (unsigned)lines() ? lines()-1 : 0)
                 + model_.size();
        inf.nPos=get_pos();
    }
    inf.nTrackPos=0;
    SetScrollInfo(SB_VERT, &inf);
}

void CListWindow::set_hscroll_pos()
{
    if (!model_.valid()) return;

    SCROLLINFO inf= {sizeof(SCROLLINFO)};
    inf.nPage=get_rect().Width();
    inf.fMask=SIF_ALL;
    inf.nMin=0;
    inf.nMax=0;
    inf.nPos=0;

    if (model_.valid())
    {
        inf.nMax=get_text_width()-1;
        inf.nPos=get_wpos();
    }
    inf.nTrackPos=0;
    SetScrollInfo(SB_HORZ, &inf);
}

void CListWindow::move_cursor(int pos)
{    
    if (!model_.valid()) return;

    unsigned uSize=model_.size();

    // перешли границу данных - возвращаем
    if (pageline_+lineno_+pos>=(int)uSize)
    {
        pos=uSize-pageline_-lineno_-1;
    }

    // перешли границу экрана - сдвигаем экран
    if (lineno_+pos>=lines()-1)
    {
        pageline_+=lineno_+pos-lines()+1;
        lineno_=lines()-1;
        pos=0;
    }

    // ушли в минус - возвращаем
    if (pageline_+lineno_+pos<0)
    {
        pos=-(pageline_+lineno_);
    }

    // ушли в минус экрана - сдвигаем экран
    if (lineno_+pos<0)
    {
        pageline_+=lineno_+pos;
        lineno_=0;
        pos=0;
    }
    lineno_+=pos;

    debug::Assert<movecursor_failure>(get_line()>=0, HERE);
    debug::Assert<movecursor_failure>(get_pos() >=0, HERE);
    debug::Assert<movecursor_failure>(get_line()<lines() || !lines(), HERE);
    debug::Assert<movecursor_failure>(get_pos() < (int)uSize || get_pos()==0 , HERE);

    set_vscroll_pos();
    on_pos_changed();
}

void CListWindow::center_on(int pos)
{
    if (pos>get_pos())
    {
        move_cursor(pos-get_pos()+3);
        move_cursor(pos-get_pos());
    }
    else
    {
        move_cursor(pos-get_pos()-3);
        move_cursor(pos-get_pos());
    }
}

void CListWindow::move_page(int pos)
{
    if (!model_.valid()) return;

    int uSize=model_.size();
    int pl=pageline_;

    pageline_=std::max(0,
                       std::min(
                           uSize-lines(),
                           pageline_+pos
                       )
                      );

    lineno_=std::max(
              0,
              std::min(
                  lineno_+(
                      pageline_-pl != pos
                      ? pos-(pageline_-pl)
                      : -pos
                  ),
                  std::min(
                      lines()-1,
                      uSize-pageline_-1)
              )
          );

    debug::Assert<movecursor_failure>(pageline_>=0, HERE);
    debug::Assert<movecursor_failure>(get_line()>=0, HERE);
    debug::Assert<movecursor_failure>(get_pos() >=0, HERE);
    debug::Assert<movecursor_failure>(get_line()<lines() || !lines(), HERE);
    debug::Assert<movecursor_failure>(get_pos() < (int)uSize || get_pos()==0 , HERE);

    set_vscroll_pos();
    on_pos_changed();
}

void CListWindow::move_hor(int i)
{
    if (!model_.valid()) return;

    wpos_=std::max(
              0,
              std::min(
                  get_max_wpos()-effwidth(),
                  wpos_+i
              ));
    set_hscroll_pos();
}

void CListWindow::set_width(int y)
{
    if (wpos_>0 && wide_-wpos_<y)
    {
        wpos_=std::max(wide_-y, 0);
    }
    move_hor(0);
    Invalidate();
}

void CListWindow::set_rect(const CRect &r)
{
    rect_=r;
    set_width(r.Width());
    Invalidate();
}

void CListWindow::set_text_extent(CSize te)
{
    text_extent_=te;
}

void CListWindow::on_pos_changed()
{
    double time=model_.get_time( get_pos() );
    model_.set_rel_pos(time);
    if (time>=0)
    {
        preffered_time_=time;
    }
    Invalidate();
}

void CListWindow::OnFinalMessage(HWND)
{
    delete this;
}

LRESULT CListWindow::OnSize(UINT, CSize sz)
{
    set_rect(CRect(CPoint(0, 0), sz));
    update_scrollbars();

    Invalidate();
    return 0;
}

LRESULT CListWindow::OnLButtonDown(UINT, CPoint pt)
{
    SetFocus();
    if (::GetAsyncKeyState(VK_MENU))
    {
        ParentLButtonDown(0, pt);
    }
    else
    {
        int pos=get_line_at_point(pt);

        set_promptstring("");
        move_cursor(int(pos-get_line()));
    }
    return 0;
}

LRESULT CListWindow::OnLButtonUp(UINT, CPoint pt)
{
    return 0;
}

LRESULT CListWindow::OnMButtonDown(UINT, CPoint pt)
{
    SetFocus();
    return 0;
}

LRESULT CListWindow::OnRButtonDown(HWND, CPoint pt)
{
    SetFocus();

    CMenu menu( ::CreatePopupMenu() );

    menu.AppendMenu(MFT_STRING | (model_.get_rel_mode() ? MFS_CHECKED: 0),
                    IDC_RELATIVE_TIME_MODE, _lcs("rel-time-mode#&Relative time mode\tF3"));
    menu.AppendMenu(MFT_STRING | (model_.get_nav_mode() ? MFS_CHECKED: 0),
                    IDC_NAVIGATION_MODE, _lcs("nav-time-mode#&Service data only mode\tF4"));
    menu.AppendMenu(MFT_SEPARATOR);
    menu.AppendMenu(MFT_STRING, 3, _lcs("LW-copyclip#&Export text to clipboard"));
    menu.AppendMenu(MFT_STRING, 6, _lcs("LW-copyclip-page#Export &page to clipboard"));
    menu.AppendMenu(MFT_STRING, 7, _lcs("LW-copyclip-image#Export &image to clipboard"));
    menu.AppendMenu(MFT_SEPARATOR);
    menu.AppendMenu(MFT_STRING, 5, _lcs("tileview-swap#Swap windows\tDblclk"));
    menu.AppendMenu(MFT_SEPARATOR);
    menu.AppendMenu(MFT_STRING, 1, _lcs("tileview-delete#&Delete window"));
    menu.AppendMenu(MFT_SEPARATOR);
    menu.AppendMenu(MFT_STRING, 2, _lcs("properties#&Properies"));

    int ret;
    if (pt.x==-1)
    {
        CRect rc; GetWindowRect(rc);
        pt=CPoint(rc.right, rc.top);
    }
    ret=menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
                            pt.x, pt.y, m_hWnd);

    switch (ret)
    {
    case 1:
    {
        DeleteFromParent();
    } break;
    case 2:
    {
        Document().Nag();

        ListPropertyDlg dialog(&model_, m_hWnd);
        dialog.DoModal();
    } break;
    case 5:
        UpdateWindow();
        StartDrag(pt);
        break;
    case 3:
        CopyClipboard();
        break;
    case 6:
        CopyPage();
        break;
    case 7:
        CopyImage();
        break;
    default:
        OnCommand(0, ret, 0);
        break;
    }

    return 0;
}


LRESULT CListWindow::OnMouseWheel(UINT a, short b, CPoint pt)
{
    //::ClientToScreen(m_hWnd, &pt);
    HWND other=WindowFromPoint(pt);
    if (other!=m_hWnd)
    {
        return ::SendMessage(other, WM_MOUSEWHEEL, MAKEWPARAM(a, b), MAKELPARAM(pt.x, pt.y));
    }

    move_page(-b*3/120);
    move_cursor(0);
    set_promptstring("");
    Invalidate();
    return 0;
}

LRESULT CListWindow::OnMouseMove(UINT, CPoint pt)
{
    return 0;
}

LRESULT CListWindow::OnKeyDown(TCHAR nChar, UINT, UINT)
{
    switch (nChar)
    {
    case VK_DOWN:
        move_cursor(1);
        set_promptstring("");
        break;
    case VK_UP:
        move_cursor(-1);
        set_promptstring("");
        break;
    case VK_PRIOR:
        move_cursor(-(int)lines());
        set_promptstring("");
        break;
    case VK_NEXT:
        move_cursor(lines());
        set_promptstring("");
        break;
    case VK_END:
        move_cursor( model_.size() );
        set_promptstring("");
        break;
    case VK_HOME:
        move_cursor(-get_pos());
        set_promptstring("");
        break;
    case VK_LEFT:
        move_hor(-10);
        break;
    case VK_RIGHT:
        move_hor(10);
        break;
    case VK_F4:
        OnCommand(0, IDC_NAVIGATION_MODE, 0);
        break;
    case VK_F3:
        OnCommand(0, IDC_RELATIVE_TIME_MODE, 0);
        break;
    }

    switch (nChar)
    {
    case VK_DOWN:
    case VK_UP:
    case VK_PRIOR:
    case VK_NEXT:
    case VK_HOME:
    case VK_END:
        break;
    }
    return 0;
}


LRESULT CListWindow::OnChar(TCHAR nChar, UINT, UINT)
{
    if (!model_.valid()) return 0;

    std::string prompt=get_promptstring();
    if (isdigit(nChar) || nChar=='.' || nChar==',')
    {
        if (strchr(",.", nChar)==0 || prompt.find_first_of(".,")==std::string::npos)
        {
            set_promptstring(prompt+=nChar);
            locate_time();
        }
    }
    else if (nChar==0x8)
    {
        if (!get_promptstring().empty())
        {
            if (prompt.length()>1)
            {
                set_promptstring(
                    prompt.substr(0, prompt.length()-1) );
                locate_time();
            }
            else set_promptstring("");
        }
    }
    else if (nChar==27)
    {
        set_promptstring("");
    }
    Invalidate();
    return 0;
}

LRESULT CListWindow::OnHScroll(int type, short pos, HWND)
{
    if (!model_.valid()) return 0;
    switch (type)
    {
    case SB_LEFT:
        move_hor(-get_max_wpos());
        break;
    case SB_RIGHT:
        move_hor(get_max_wpos());
        break;
    case SB_LINELEFT:
        move_hor(-10);
        break;
    case SB_LINERIGHT:
        move_hor(10);
        break;
    case SB_PAGELEFT:
        move_hor(-effwidth());
        break;
    case SB_PAGERIGHT:
        move_hor(effwidth());
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
    {
        move_hor(pos-get_wpos());
    }
    break;
    }
    Invalidate();
    return 0;
}

LRESULT CListWindow::OnVScroll(int type, short pos, HWND)
{
    if (!model_.valid()) return 0;
    switch (type)
    {
    case SB_TOP:
        move_cursor(-get_pos());
        break;
    case SB_BOTTOM:
        move_cursor( model_.size() );
        break;
    case SB_PAGEUP:
        move_cursor(-lines());
        break;
    case SB_PAGEDOWN:
        move_cursor(+lines());
        break;
    case SB_LINEUP:
        move_cursor(-1);
        break;
    case SB_LINEDOWN:
        move_cursor(+1);
        break;
    case SB_THUMBTRACK:
    case SB_THUMBPOSITION:
        move_cursor(pos-get_pos());
        break;
    }
    set_promptstring("");
    Invalidate();
    return 0;
}

LRESULT CListWindow::OnSetFocus(HWND)
{
    Invalidate();
    return 0;
}

LRESULT CListWindow::OnLostFocus(HWND)
{
    set_promptstring("");
    Invalidate();
    return 0;
}

LRESULT CListWindow::OnTimer(UINT)
{
    return 0;
}

LRESULT CListWindow::OnCommand(UINT, int id, HWND)
{
    switch (id)
    {
    case IDC_NAVIGATION_MODE:
    {
        set_promptstring("");
        preffered_time_=model_.get_time( get_pos() );

        model_.set_nav_mode( !model_.get_nav_mode() );
        Invalidate();
    }
    break;
    case IDC_RELATIVE_TIME_MODE:
    {
        model_.set_rel_mode( !model_.get_rel_mode() );
        on_pos_changed();
    }
    break;
    }
    return 0;
}

LRESULT CListWindow::OnCopyClip(UINT, WPARAM wParam, LPARAM lParam)
{
    CDCHandle dc((HDC)wParam);
    CRect   rect((LPRECT)lParam);

    OnPaint(dc, rect, true);

    return 0;
}

bool CListWindow::has_focus() const
{
    return m_hWnd==GetFocus();
}

LRESULT CListWindow::OnLButtonDbl(UINT, CPoint pt)
{
    ClientToScreen(&pt);
    StartDrag(pt);
    return 0;
}

void CListWindow::serialization(serl::archiver &ar)
{
    ar.serial(model_);
}

//////////////////////////////////////////////////////////////////////////


struct clipboard_result : process::rslt
{
    std::string text;
};

class ClipboardInserter
        : public process::host_t<clipboard_result>
{
public:
    ClipboardInserter(list_model *model)
    {
        set_parent(model);
    }

    ~ClipboardInserter()
    {
        detach_host();
    }

    void setup(process::hostsetup& setup)
    {
        setup.need_config = false;
        setup.weight = 10;
        setup.name = _ls("listwindow-clipboard#Collecting data");
    }

    struct processor : public process::processor_t<process::config, clipboard_result, list_result>
    {
        bool process()
        {
            result->text = source->get_header() + "\r\n";
            for(unsigned index=0; index < source->size(); ++index)
            {
                bool isEvent = false;
                result->text += source->line(index, isEvent)+"\r\n";

                if (!check_status(index, source->size()))
                    return false;
            }

            return true;
        }
    };

    process::processor* create_processor()
    {
        return new processor;
    }
};

//////////////////////////////////////////////////////////////////////////

class CProgressBar : public CDialogImpl<CProgressBar>
{
public:

    enum { IDD=IDD_PROGRESS_BAR };

    BEGIN_MSG_MAP_EX(CProgressBar)

    MSG_WM_INITDIALOG(OnInitDialog);
    MSG_WM_TIMER(OnTimer);
    COMMAND_ID_HANDLER_EX(IDCANCEL, OnCancel);

    END_MSG_MAP();

    CProgressBar(process::host *j)
        : host_(j)
    {}

    LRESULT OnInitDialog(HWND, LPARAM)
    {
        _lw(m_hWnd, IDD_PROGRESS_BAR);
        CenterWindow();

        SetTimer(1, 100);
        return 0;
    }

    LRESULT OnCancel(UINT, int, HWND)
    {
        EndDialog(IDCANCEL);
        return 0;
    }

    LRESULT OnTimer(UINT)
    {
        if (process::get_result<process::rslt>(host_, false))
            EndDialog(IDOK);
        return 0;
    }

private:

    process::host  *host_;
};

//////////////////////////////////////////////////////////////////////////

void CListWindow::CopyPage()
{
    ShowCopyClipDlg(GetParent());
}

void CListWindow::CopyImage()
{
    ShowCopyClipDlg(m_hWnd);
}

void CListWindow::CopyClipboard()
{        
    ClipboardInserter inserter( &model_ );

    CProgressBar bar(&inserter);
    if (bar.DoModal()==IDOK)
    {
        std::string text = inserter.get_result(true)->text;

        if (::OpenClipboard(m_hWnd))
        {
            ::EmptyClipboard();

            HGLOBAL clipbuffer= ::GlobalAlloc(GMEM_DDESHARE, text.size()+1);
            char *buffer=(char *)::GlobalLock(clipbuffer);
            std::copy(STL_II(text), buffer);
            ::GlobalUnlock(clipbuffer);

            ::SetClipboardData(CF_TEXT, clipbuffer);
            ::CloseClipboard();
        }
    }
}
