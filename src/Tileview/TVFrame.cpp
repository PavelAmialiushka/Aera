#include "Stdafx.h"

#include "TVFrame.h"
#include "TVCreator.h"

#include "utilites/foreach.hpp"

#include "utilites/serl/Archive.h"
#include "utilites/serl/serl_registry.h"

#include "utilites/Localizator.h"

#include "arrangeWnd.h"

#include "utilites/document.h"

#include "others/cximage/ximage.h"

enum { m=point::max, };


CTileViewFrame::CTileViewFrame(bool autodelete)
    : settingselection_(0),
      do_create_window_(0),
      autodelete_(autodelete),
      hide_while_moving_(false),
      dragging_(false)
{
}



LRESULT CTileViewFrame::OnCreate(LPCREATESTRUCT)
{
    SetWindowLong(GWL_STYLE,  GetStyle() | WS_CLIPCHILDREN );
    return 0;
}

LRESULT CTileViewFrame::OnDestroy()
{
    focus_=pwindow_t();
    while (page_.size())
    {
        page_.erase(0);
    }
    return 0;
}

void  CTileViewFrame::AppendWindows(int n)
{
    for (int index=0; index<n; ++index)
    {
        page_.append_new_window(CreateNewWindow());
    }

    page_.rearrange_windows();
    UpdateLayout();

    TVFNMHDR hdr(m_hWnd, TVFN_WINDOWADDED);
    hdr.SendMessage();
}

LRESULT CTileViewFrame::OnSize(UINT, CSize cs)
{
    UpdateLayout();
    return 0;
}

BOOL CTileViewFrame::PreTranslateMessage(MSG *pMsg)
{
    pMsg;
    return FALSE;
}

template<typename T>
class vector2
{
    unsigned width_;
    unsigned size_;
    T *array_;
public:
    vector2(T *array, unsigned width, unsigned size=0)
        : width_(width), array_(array), size_(size/width)
    {}

    vector2(std::vector<double> &array, unsigned width)
        : width_(width), array_(&array[0]), size_(array.size()/width) {}

    T *operator[](unsigned index) const
    {
        return array_+index*width_;
    }
    unsigned size() const
    {
        return size_;
    }
    unsigned width() const
    {
        return width_;
    }
};

void ShadowArea(CDC &dc, CRect r)
{
    int *data;
    BITMAPINFO bi=
    {
        {
            sizeof(BITMAPINFOHEADER),
            r.Width(),
            r.Height(),
            1, 32, BI_RGB
        }
    };
    CBitmap work( ::CreateDIBSection(dc, &bi, DIB_RGB_COLORS, (void **)(&data), 0, 0) );
    vector2<int> vector(data, r.Width(), r.Height());

    CDC temp( CreateCompatibleDC(dc) );
    temp.SelectBitmap(work);

    temp.BitBlt(0, 0, r.Width(), r.Height(), dc, r.left, r.top, SRCCOPY);
    {
        for (int index=0; index < r.Width(); ++index)
        {
            for (int ord=0; ord<r.Height(); ++ord)
            {
                if ( (index+ord)%2 )
                {
                    vector[ord][index]=0x7F;
                }
            }
        }
    }
    dc.BitBlt(r.left, r.top, r.Width(), r.Height(), temp, 0, 0, SRCCOPY);
}


static CPoint make_line(CPoint zero, CPoint start, double angle, int len)
{
    start-=zero;
    double alpha=atan2((double)start.y, (double)start.x);
    alpha+=angle;

    CPoint other( static_cast<int>(len*cos(alpha)),
                  static_cast<int>(len*sin(alpha)) );
    other+=zero;
    return other;
}

LRESULT CTileViewFrame::OnPaint(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    CPaintDC pdc(m_hWnd);
    CRect rc; GetClientRect(rc);

    // memory dc
    CDC dc( ::CreateCompatibleDC(pdc) );
    CBitmap bitmap( ::CreateCompatibleBitmap(pdc, rc.Width(), rc.Height()));
    dc.SelectBitmap(bitmap);

    // background

    CBrush brush( ::CreateSolidBrush(GetSysColor(COLOR_3DFACE)) );
    CBrushHandle oldbrush=dc.SelectBrush(brush);

    std::vector<pwindow_t> vector;
    ReceiveSubWindows(vector);

    CPen pen( ::CreatePen(PS_SOLID, 1, ::GetSysColor(COLOR_3DFACE)) );
    CPen oldpen=dc.SelectPen(pen);

    //////////////////////////////////////////////////////////////////////////

    // drawing splitters and window images if needed
    foreach(pwindow_t win, vector)
    {
        CRect r=win->get_rect().transform(rc);
        dc.Rectangle(r);

        r.DeflateRect(1,1);
        dc.DrawEdge(r, EDGE_SUNKEN, BF_RECT);

        r.DeflateRect(2,2);

        if (dragging_)
        {
            int id=win->get_id();
            if (bitmaps_.find(id)!=bitmaps_.end())
            {
                CBitmapHandle bmp=bitmaps_[id];
                CSize bmpsize=bitmaps_[id].GetSize();

                CDC bdc( CreateCompatibleDC(dc) );
                bdc.SelectBitmap(bmp);

                dc.SetStretchBltMode(COLORONCOLOR);

                dc.StretchBlt(r.left, r.top, r.Width(), r.Height(),
                              bdc, 0, 0, bmpsize.cx, bmpsize.cy, SRCCOPY);
            }
        }
    }

    // drawing new empty window
    if (inserter_)
    {
        line ln=inserter_->get_selline();

        CPoint a=(ln.get_point1()).transform(rc);
        CPoint b=(ln.get_point2()).transform(rc);

        CPen pen( ::CreatePen(PS_SOLID, 2, RGB(255, 0, 0) ));
        dc.SelectPen(pen);

        dc.MoveTo(a);
        dc.LineTo(b);

        if (inserter_->get_newwindow())
        {
            dc.FillRect(
                GetChildRect( vector.back() ),
                CBrush(
                    ::CreateSolidBrush(RGB(0xf0,0xf0,0xc0))));
        }
    }

    // drawing a shadow over selected windows
    if (selection_)
    {
        LOGBRUSH lb= {BS_HOLLOW};
        CBrush brush( ::CreateBrushIndirect(&lb) );
        dc.SelectBrush(brush);

        if (settingselection_)
        {
            CPen pen( ::CreatePen(PS_SOLID, 1, RGB(0,255,0)) );
            dc.SelectPen(pen);
            dc.Rectangle( selection_->get_drag_rect().transform(rc) );
        }

        CPen pen( ::CreatePen(PS_SOLID, 1, RGB(0,0,255)) );
        dc.SelectPen(pen);
        dc.Rectangle( selection_->get_rect().transform(rc) );

        ShadowArea(dc, selection_->get_rect().transform(rc));
    }

    // рисуем изображание стрелки для замены окон

    if (dragging_)
    {
        CPen pen( CreatePen(PS_SOLID, 2, 0xFFFFFF) );
        dc.SelectPen( pen );

        CPoint cursor;
        GetCursorPos(&cursor);
        ScreenToClient(&cursor);

        dc.MoveTo(drag_start_point_);
        dc.LineTo(cursor);

        dc.MoveTo(cursor);
        dc.LineTo( make_line(cursor, drag_start_point_, 0.2, 20 ));
        dc.MoveTo(cursor);
        dc.LineTo( make_line(cursor, drag_start_point_, -0.2, 20 ));

        dc.MoveTo(drag_start_point_);
        dc.LineTo( make_line(drag_start_point_, cursor, 0.2, 20 ));
        dc.MoveTo(drag_start_point_);
        dc.LineTo( make_line(drag_start_point_, cursor, -0.2, 20 ));
    }

    dc.SelectPen(oldpen);
    dc.SelectBrush(oldbrush);
    pdc.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

    return 0;
}

LRESULT CTileViewFrame::OnFocus(HWND)
{
    UpdateLayout();
    Invalidate();
    return 0;
}

LRESULT CTileViewFrame::OnEraseBackground(HDC)
{
    return 0;
}

CRect CTileViewFrame::GetChildRect(pwindow_t win) const
{
    CRect rc; GetClientRect(rc);
    rc=win->get_rect().transform(rc);
    rc.DeflateRect(3, 3);
    return rc;
}

void CTileViewFrame::UpdateLayout()
{
    CRect rc; GetClientRect(rc);
    std::vector<CRect> vctr;
    std::vector<pwindow_t> wins;
    ReceiveSubWindows(wins);

    foreach(pwindow_t win, wins)
    {
        vctr.push_back( GetChildRect(win) );
    }
    page_.UpdateLayout(vctr);

    Invalidate();
}

void CTileViewFrame::ReceiveSubWindows(std::vector<pwindow_t> &vector)
{
    vector.clear();
    CRect rc; GetClientRect(rc);

    if (mobil_zone_)
    {
        foreach(pwindow_t wnd, *mobil_zone_->get_page())
        {
            vector.push_back( wnd );
        }
        foreach(pwindow_t win, page_)
        {
            if (!mobil_zone_->get_page()->find(win->get_id()))
            {
                vector.push_back( win );
            }
        }
    }
    else if (inserter_)
    {
        foreach(pwindow_t wnd, *inserter_->get_page())
        {
            vector.push_back( wnd );
        }
    }
    else
    {
        foreach(pwindow_t wnd, page_)
        {
            vector.push_back( wnd );
        }
    }
}

LRESULT CTileViewFrame::OnMouseMove(UINT, CPoint pt)
{
    CRect rc; GetClientRect(rc);

    if (dragging_)
    {

        //drag_list_.DragMove(pt.x, pt.y);
        InvalidateRect(0);

    }
    else if (mobil_zone_)
    {

        mobil_zone_->track(point(pt, rc));
        UpdateLayout();
        OnSetCursor();

    }
    else if (inserter_)
    {

        inserter_->track_point(point(pt, rc));
        UpdateLayout();
        Invalidate();

    }
    else if (settingselection_)
    {

        selection_=SelectionRect::ptr( new SelectionRect(&page_, selectionpoint_, point(pt,rc)));
        Invalidate();

    }
    else
    {

        Line snl=MakeSnapLine(&page_, point(pt, rc));
        if (snl.empty()) return 0;

        direction dir=snl.get_direction();
        vector tmp=snl-point(pt, rc);
        int dist=tmp.transform(rc);

        try
        {

            if (GetAsyncKeyState(VK_CONTROL)<0)
            {
                SingleSlideLine temp(&page_, snl, dir+cw);
            }
            else
            {
                DoubleSlideLine temp(&page_, point(pt, rc));
            }

        }
        catch (geometry_error)
        {
            ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
            return 0;
        }

        ::SetCursor(
            ::LoadCursor(NULL, snl.get_direction().is_horizontal()
                         ? IDC_SIZENS
                         : IDC_SIZEWE));

    }
    return 0;
}

LRESULT CTileViewFrame::OnLButtonDown(UINT, CPoint pt)
{
    CRect rc; GetClientRect(rc);

    if (!PtInRect(rc, pt))
        return 0;

    Line snl=MakeSnapLine(&page_, point(pt,rc));
    int dist=snl-point(pt,rc);

    if (::GetAsyncKeyState(VK_CONTROL)<0)
    {
        // inserting new
        inserter_=Inserter::ptr( new Inserter(&page_, point(pt, rc)) );
        selection_=SelectionRect::ptr();
        Invalidate();
        SetCapture();

    }
    else if (::GetAsyncKeyState(VK_MENU)<0 || dist > 1)
    {

//    settingselection_=true;
//    selectionpoint_=point(pt,rc);
//    selection_=SelectionRect::ptr( new SelectionRect(&page_, selectionpoint_, point(pt,rc)));
//    if (selection_->get_subpage(&page_)->size())
//    {
//      focus_=selection_->get_subpage(&page_)->at(0);
//    }
//
//    HideAll();
//    SetCapture();

    }
    else
    {
        // resizing
        try
        {
            mobil_zone_=shared_ptr<XSlideLine>(
                            GetAsyncKeyState(VK_SHIFT)<0
                            ? new XSlideLine(&page_, point(pt, rc),
                                             true, true,
                                             &MakeSnapLine)
                            : new XSlideLine(&page_, point(pt, rc),
                                             false, true,
                                             &MakeMinSnapLine)
                        );
            OnSetCursor();
            SetCapture();

            if (GetAsyncKeyState(VK_SHIFT)>=0)
            {
                CRect rc; GetWindowRect(rc);

                CMainDocument::instance()->GetDocument()->ShowHint(
                    _ls("tileview-hint1#SHIFT-drag to move entire border")+", "+
                    _ls("tileview-hint2#CTRL-drag to create new window")+", "+
                    _ls("tileview-hint3#DOUBLE-CLICK-drag to swap windows"));

            }

        }
        catch (geometry_error &) {}
    }

    return 0;
}


LRESULT CTileViewFrame::OnLButtonUp(UINT, CPoint pt)
{
    CRect rc; GetClientRect(rc);

    if (dragging_)
    {

        dragging_=false;
//    drag_list_.DragLeave(m_hWnd);
//    drag_list_.EndDrag();
//    drag_list_.Destroy();
        ReleaseCapture();

        ShowAll();

        Page::iterator begin=std::find(STL_II(page_), drag_window_);

        for (unsigned index=0; index<page_.size(); ++index)
        {
            pwindow_t wnd=page_.at(index);
            if (wnd->get_rect().transform(rc).PtInRect(pt))
            {
                rect tmp = wnd->get_rect();
                wnd->set_rect( drag_window_->get_rect() );
                drag_window_->set_rect( tmp );
                UpdateLayout();
                std::swap_ranges(begin, begin+1, page_.begin()+index);
                CMainDocument::instance()->GetDocument()->SetModified();
                break;
            }
        }

    }
    else if (mobil_zone_)
    {

        // изменение размеров
        mobil_zone_->commit();
        mobil_zone_=shared_ptr<XSlideLine>();
        ::ReleaseCapture();
        UpdateLayout();

        TVFNMHDR hdr(m_hWnd, TVFN_WINDOWRESIZED);
        hdr.SendMessage();
        CMainDocument::instance()->GetDocument()->SetModified();
        CMainDocument::instance()->GetDocument()->HideHint();

    }
    else if (inserter_)
    {

        inserter_->commit( &page_ );
        if (inserter_->get_newwindow())
        {
            page_.at(page_.size()-1)->set_hwnd(CreateNewWindow());
        }
        ShowAll();
        ReleaseCapture();
        UpdateLayout();
        inserter_=Inserter::ptr();

        TVFNMHDR hdr(m_hWnd, TVFN_WINDOWREMOVED);
        hdr.SendMessage();

        CMainDocument::instance()->GetDocument()->SetModified();

    }
    else if (settingselection_)
    {

        settingselection_=false;
        if (selection_->get_subpage(&page_)->size()<=1)
        {
            selection_=SelectionRect::ptr();
        }
        ReleaseCapture();
        Invalidate();
    }

    if (!selection_)
    {
        ShowAll();
    }

    return 0;
}

LRESULT CTileViewFrame::OnShowWindow(UINT, WPARAM, LPARAM)
{
    UpdateLayout();
    return 0;
}

LRESULT CTileViewFrame::OnSetFocus(HWND hwnd)
{
    if (page_.size()==0) return 0;
    focus_=pwindow_t();
    foreach(pwindow_t pw, page_)
    {
        if (pw->get_hwnd()==hwnd)
        {
            focus_=pw;
        }
    }
    if (!focus_)
    {
        focus_=page_.at(0);
    }
    if (CWindow win=focus_->get_hwnd())
    {
        if (win.GetStyle()&WS_VISIBLE)
        {
            win.SetFocus();
            return 0;
        }
    }
    return 0;
}

static bool by_position(pwindow_t a, pwindow_t b)
{
    if (a->get_rect().top_!=b->get_rect().top_)
        return a->get_rect().top_ < b->get_rect().top_;
    else
        return a->get_rect().left_ < b->get_rect().left_;
}

void CTileViewFrame::FocusNext()
{
    std::vector<pwindow_t> windows(page_.size());
    std::copy(STL_II(page_), windows.begin());

    std::sort(STL_II(windows), by_position);
    std::vector<pwindow_t>::iterator index=
        std::find(STL_II(windows), focus_);

    if (index==windows.end()) --index;
    if (++index==windows.end()) index=windows.begin();
    focus_=*index;
    if (CWindow win=focus_->get_hwnd())
    {
        win.SetFocus();
    }
}

void CTileViewFrame::HideAll()
{
//  if (!hide_while_moving_) return;
    foreach(pwindow_t pw, page_)
    {
        if (CWindow win=pw->get_hwnd())
        {
            if (win.m_hWnd==GetFocus())
            {
                focus_=pw;
            }

            if (win.GetStyle()&WS_VISIBLE)
            {
                CRect rc; win.GetClientRect(rc);

                CDC dc=CDC( ::CreateCompatibleDC(win.GetDC()) );
                CBitmapHandle bmp=CBitmapHandle(
                                      ::CreateCompatibleBitmap(win.GetDC(), rc.Width(), rc.Height()) );
                dc.SelectBitmap(bmp);

                dc.BitBlt(0, 0, rc.Width(), rc.Height(),
                          win.GetDC(), 0, 0, SRCCOPY);

                bitmaps_[pw->get_id()]=CBmp( bmp, rc.Size() );
            }

            win.ShowWindow(SW_HIDE);
        }
    }
}

void CTileViewFrame::ShowAll()
{
    foreach(pwindow_t pw, page_)
    {
        if (CWindow win=pw->get_hwnd())
        {
            win.ShowWindow(
                SW_SHOWNOACTIVATE);
            if (pw==focus_) win.SetFocus();
        }
    }
}

LRESULT CTileViewFrame::OnChildLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    PARENTMOUSEMESSAGE *pmm=(PARENTMOUSEMESSAGE *)lParam;
    CPoint pt=pmm->point;
    ::ClientToScreen((HWND)wParam, &pt);
    ::ScreenToClient(m_hWnd, &pt);

    UINT nFlags=pmm->nFlags;
    return OnLButtonDown(nFlags, pt);
}

LRESULT CTileViewFrame::OnDeleteChild(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
    HWND hwnd=(HWND)wParam;

    if (page_.size()>1)
    {
        foreach(pwindow_t pw, page_)
        {
            if (pw->get_hwnd()==hwnd)
            {
                CWindow win(pw->get_hwnd());
                win.ShowWindow(SW_HIDE);

                if (pw==focus_)
                {
                    FocusNext();
                }

                Deleter deleter(&page_, pw);

                UpdateLayout();

                TVFNMHDR hdr(m_hWnd, TVFN_WINDOWREMOVED);
                hdr.SendMessage();

                return 0;
            }
        }
    }
    return 0;
}

LRESULT CTileViewFrame::OnReplaceChild(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
    HWND prev=(HWND)wParam;
    HWND next=(HWND)lParam;

    foreach(pwindow_t pw, page_)
    {
        if (pw->get_hwnd()==prev)
        {
            CWindow win(prev);
            bool vis=win.IsWindowVisible();
            CRect rc; win.GetWindowRect(rc);
            ScreenToClient(rc);
            win.ShowWindow(SW_HIDE);
            win.DestroyWindow();

            pw->set_hwnd(next);
            win.Attach(next);
            win.MoveWindow(rc);
            win.ShowWindow(vis ? SW_SHOW : SW_HIDE);
            win.SetFocus();
        }
    }

    return 0;
}


void CTileViewFrame::SetCreateWindowCallback(boost::function1<HWND, HWND> func)
{
    do_create_window_=func;
}

HWND CTileViewFrame::CreateNewWindow()
{
    return do_create_window_
           ? do_create_window_(m_hWnd)
           : (HWND)NULL;
}

void CTileViewFrame::serialization(serl::archiver &ar)
{
    if (ar.is_loading())
    {
        focus_=page_.at(0);
        inserter_  =shared_ptr<Inserter>();
        selection_ =shared_ptr<SelectionRect>();
        mobil_zone_=shared_ptr<XSlideLine>();
        bitmaps_.clear();
        foreach(pwindow_t pw, page_)
        {
            ::DestroyWindow(pw->get_hwnd());
        }
    }

    ar.serial(page_);

    if (ar.is_loading())
    {
        foreach(pwindow_t pw, page_)
        {
            CTVCreator *ctr=pw->get_contents();
            pw->set_hwnd(ctr->activate(m_hWnd));
        }
        UpdateLayout();
        ShowAll();
    }
}

LRESULT CTileViewFrame::OnFileSave(UINT, int, HWND)
{
//  serl::save_archiver saver;
//  saver.serial("frame", *this);
//
//  serialization_buffer_=saver.string();
//
//  std::ofstream("d:\\temp.txt") << serialization_buffer_ << std::endl;
    return 0;
}

LRESULT CTileViewFrame::OnFileOpen(UINT, int, HWND)
{
//  if (!serialization_buffer_.empty())
//  {
//    serl::load_archiver loader;
//    loader.assign(serialization_buffer_);
//    loader.serial("frame", *this);
//  }
    return 0;
}

void CTileViewFrame::OnFinalMessage(HWND)
{
    if (autodelete_) delete this;
}

LRESULT CTileViewFrame::OnMButtonDown(UINT, CPoint)
{
    return 0;
}

LRESULT CTileViewFrame::OnArrangeWindows(UINT, WPARAM, LPARAM, BOOL &)
{
    if (page_.size()>1)
        ShowArrangeWindow();
    return 0;
}

static void arrange(CTileViewFrame *self, bool inc)
{
    CArrangeWnd *aw=new CArrangeWnd(self, inc);
    aw->Create(self->m_hWnd,
               CWindow::rcDefault, _lcs("ArrangeWindow#Select page layout"),
               WS_VISIBLE| DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU,
               WS_EX_DLGMODALFRAME);
}

void CTileViewFrame::ShowArrangeWindow()
{
    arrange(this, 0);
}

void CTileViewFrame::AddArrangeWindow()
{
    arrange(this, 1);
}

LRESULT CTileViewFrame::OnUpdateLayout(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL & /*bHandled*/)
{
    CMainDocument::instance()->GetDocument()->SetModified();
    UpdateLayout();
    return 0;
}

LRESULT CTileViewFrame::OnIsInDragMode(UINT, WPARAM, LPARAM, BOOL &)
{
    return dragging_;
}

LRESULT CTileViewFrame::OnStartDrag(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL & /*bHandled*/)
{
    HWND hview=(HWND)lParam;
    drag_window_=pwindow_t();
    foreach(pwindow_t wnd, page_)
    {
        if (hview==wnd->get_hwnd())
            drag_window_=wnd;
    }
    if (!drag_window_) return 0;

    CPoint mouse(wParam);
    ScreenToClient(&mouse);

    drag_start_point_=mouse;

    SetCapture();

    dragging_=true;

    OnSetCursor(0, 0, 0);

    HideAll();

//  CRect rc; ::GetClientRect(hview, rc);
//
//  drag_list_.Create(1000, 16, 1, 0xC0C0C0);
//  drag_list_.SetBkColor( 0x000000 );
//
//  CPoint pt; GetCursorPos(&pt);
//  ::ScreenToClient(hview, &pt);
//
//  drag_list_.BeginDrag(drag_list_.GetImageCount()-1, 8, 8);
//  drag_list_.DragEnter(m_hWnd, mouse.x, mouse.y);
    return 0;
}

LRESULT CTileViewFrame::OnCopyClip(UINT, WPARAM wParam, LPARAM lParam)
{
    CDCHandle dc=(HDC)wParam;
    CRect   rect=(LPRECT)lParam;

    CBrush brush( CreateSolidBrush(0xFFFFFF) );
    CBrushHandle oldb=dc.SelectBrush( brush );

    dc.FillRect(rect, brush);

    CPen pen( CreatePen(PS_SOLID, 1, 0x000000) );
    CPenHandle oldp=dc.SelectPen(pen);

    foreach(pwindow_t wnd, page_)
    {
        CRect wndrect=wnd->get_rect().transform(rect);
        dc.Rectangle(wndrect);

        wndrect.DeflateRect(1, 1);
        ::SendMessage(wnd->get_hwnd(), TVVM_COPYCLIP, wParam, (LPARAM)(LPRECT)wndrect);
    }

    dc.SelectPen( oldp );
    dc.SelectBrush( oldb );

    return 0;
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#define MAXWRITE 2048
// Bitmap for Graph and Rooler

PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
    BITMAP bmp;
    PBITMAPINFO pbmi;
    WORD    cClrBits;

    if (!GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp)) MessageBox(NULL,"","",NULL);

    cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

    if (cClrBits == 1)
        cClrBits = 1;
    else if (cClrBits <= 4)
        cClrBits = 4;
    else if (cClrBits <= 8)
        cClrBits = 8;
    else if (cClrBits <= 16)
        cClrBits = 16;
    else if (cClrBits <= 24)
        cClrBits = 24;
    else
        cClrBits = 32;


    if (cClrBits != 24)
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR,
                                        sizeof(BITMAPINFOHEADER) +
                                        sizeof(RGBQUAD) * (2^cClrBits));
    else
        pbmi = (PBITMAPINFO) LocalAlloc(LPTR,
                                        sizeof(BITMAPINFOHEADER));

    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = bmp.bmWidth;

    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (cClrBits < 24)

        pbmi->bmiHeader.biClrUsed = 2^cClrBits;

    pbmi->bmiHeader.biCompression = BI_RGB;
    pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8
                                  * pbmi->bmiHeader.biHeight
                                  * cClrBits;

    pbmi->bmiHeader.biClrImportant = 0;

    return pbmi;

}

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi,
                   HBITMAP hBMP, HDC hDC)
{

    HANDLE hf;                  /* file handle */
    BITMAPFILEHEADER hdr;       /* bitmap file-header */
    PBITMAPINFOHEADER pbih;     /* bitmap info-header */
    LPBYTE lpBits;              /* memory pointer */
//    DWORD dwTotal;              /* total count of bytes */
    DWORD cb;                   /* incremental count of bytes */
    BYTE *hp;                   /* byte pointer */

    DWORD dwTmp;

    hf = CreateFile(pszFile,
                    GENERIC_READ | GENERIC_WRITE,
                    0, 0, CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,0);

    debug::Assert<fault>(hf!=INVALID_HANDLE_VALUE, HERE);

    pbih = (PBITMAPINFOHEADER) pbi;
    lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);
    debug::Assert<fault>(lpBits, HERE);

    int res=GetDIBits(hDC, hBMP, 0, (WORD) pbih->biHeight,
                      lpBits, pbi, DIB_RGB_COLORS);
    debug::Assert<fault>(res, HERE);

    hdr.bfType = 0x4d42;        /* 0x42 = "B" 0x4d = "M" */
    hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +
                          pbih->biSize + pbih->biClrUsed
                          * sizeof(RGBQUAD) + pbih->biSizeImage);
    hdr.bfReserved1 = 0;
    hdr.bfReserved2 = 0;
    hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
                    pbih->biSize + pbih->biClrUsed
                    * sizeof (RGBQUAD);

    res=WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER),
                  (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL);
    debug::Assert<fault>(res, HERE);


    /* Copy the BITMAPINFOHEADER and RGBQUAD array into the file. */

    res=WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER)
                  + pbih->biClrUsed * sizeof (RGBQUAD),
                  (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL);
    debug::Assert<fault>(res, HERE);

    cb = pbih->biSizeImage;
    hp = lpBits;
    while (cb > MAXWRITE)
    {
        WriteFile(hf, (LPSTR) hp, (int) MAXWRITE,
                  (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL);
        cb-= MAXWRITE;
        hp += MAXWRITE;
    }

    res=WriteFile(hf, (LPSTR) hp, (int) cb,
                  (LPDWORD) &dwTmp, (LPOVERLAPPED) NULL);

    debug::Assert<fault>(res, HERE);

    ::CloseHandle(hf);
    ::GlobalFree((HGLOBAL)lpBits);
}

#include "controls\copyclipDlg.h"


void ShowCopyClipDlg(HWND window)
{
    std::vector<unsigned> array;

    serl::load_archiver(
        new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data"))
    .serial_container("clipboard", array);

    if (array.empty())
    {
        unsigned data[]= {640, 480};
        array.assign(data, data+2);
    }

    CCopyClipDlg dialog;
    dialog.pixelWidth = array[0];
    dialog.pixelHeight = array[1];

    int res=dialog.DoModal(::GetActiveWindow());

    if (res!=IDCANCEL)
    {
        CRect rc(0, 0, dialog.pixelWidth, dialog.pixelHeight);
        CDC windowDC( ::GetWindowDC( GetDesktopWindow() ) );

        CDC dc( CreateCompatibleDC( windowDC ) );
        CBitmap bitmap( ::CreateCompatibleBitmap(windowDC, rc.Width(), rc.Height()) );
        CBitmapHandle old=dc.SelectBitmap(bitmap);

        ::SendMessage(window, TVVM_COPYCLIP, (WPARAM)dc.m_hDC, (LPARAM)(LPRECT)rc);

        dc.SelectBitmap(old);

        if (res==IDOK)
        {
            if (::OpenClipboard(window))
            {
                ::EmptyClipboard();
                ::SetClipboardData(CF_BITMAP, bitmap.Detach());
                ::CloseClipboard();
            }
        }
        else
        {

            CFileDialog dlg(false, "gif", 0, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                            "GIF files (*.gif)\0*.gif\0");
            if (dlg.DoModal()==IDOK)
            {
                shared_ptr<CxImage> image(new CxImage);
                image->CreateFromHBITMAP(bitmap);

                image->DecreaseBpp(8, false);
                image->Save(dlg.m_szFileName, CXIMAGE_FORMAT_GIF);
            }
        }

        array[0] = dialog.pixelWidth;
        array[1] = dialog.pixelHeight;
        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, "Software\\AETestingTools\\Data"))
        .serial_container("clipboard", array);
    }
}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#include "..\aera\resource.h"

LRESULT CTileViewFrame::OnSetCursor(HWND, UINT, UINT)
{
//  if (dragging_) {
//
//    ::SetCursor(::LoadCursor(_Module.GetResourceInstance(),  MAKEINTRESOURCE(IDC_EXCHANGE)) );
//
//  } else
    if (mobil_zone_)
    {

        ::SetCursor(::LoadCursor(NULL,
                                 mobil_zone_->get_line().get_direction().is_horizontal()
                                 ? IDC_SIZENS
                                 : IDC_SIZEWE));

    }
    else
    {
        SetMsgHandled(false);
    }
    return 0;
}

