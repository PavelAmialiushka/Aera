#include "stdafx.h"

#include "CChannelWnd.h"

#include "controls/plotcolors.h"
#include "controls/PlotPool.h"

#include "data/nodeFactory.h"
#include "data/node.h"

#include "wtladdons.h"

#include "utilites/foreach.hpp"

#include "utilites/Localizator.h"

#include "toolbarbtns.h"

#include "resource.h"

enum { ID_UP=1001, ID_DOWN=1002, };

CChannelWnd::Button::Button(int i, int g)
{
    id = i;
    group = g;
}

CChannelWnd::CChannelWnd()
    : hiddenrows_(0),
      row_(0)
{
    std::fill(STL_II(checks_), true);

    PlotManager::inst().attach_listener(this);
}

CChannelWnd::~CChannelWnd()
{
    PlotManager::inst().detach_listener(this);
}

HWND CChannelWnd::Create(HWND hparent)
{
    CWindow parent(hparent);
    CRect prc; parent.GetWindowRect(prc);
    parent.ClientToScreen(prc);

    return baseClass::Create(
                parent, CRect(prc.TopLeft(), CSize(75, 200)), "",
               WS_VISIBLE|WS_CAPTION|WS_POPUP|WS_OVERLAPPED|WS_CLIPSIBLINGS|WS_CLIPCHILDREN|WS_SYSMENU
               |WS_SYSMENU|WS_THICKFRAME);
}

// создать одну кнопку с номером i

void CChannelWnd::CreateButton(int i, int g)
{
    Button button(i, g);

    std::string name =
        i == B_NAME
        ? sheme_.objects[g].name
        : boost::lexical_cast<std::string>(i).c_str();

    button.wnd.Create(m_hWnd, CRect(CPoint(0,0), GetButtonSize()),
                      name.c_str(),
                      WS_CHILD|WS_VISIBLE|BS_TEXT
                      |BS_PUSHLIKE|BS_AUTOCHECKBOX
                      |BS_NOTIFY
                      |BS_OWNERDRAW,
                      0,
                      MAKELONG(i, g)
                     );

    button.wnd.SetFont(spin_buttons_font_);

    buttons_.push_back(button);
}

channel_sheme filter(channel_sheme const &other, boost::array<bool, 256> &active)
{
    if (std::accumulate(STL_II(active), 0)==0)
        return other;

    channel_sheme self = other;

    for (unsigned index=0; index < self.objects.size(); ++index)
    {
        channel_object &object = self.objects[index];
        for (unsigned ki=0; ki<object.channels.size();)
        {
            if (active[ object.channels[ki]-1 ])
                ++ki;
            else
                object.channels.erase( object.channels.begin() + ki );
        }
    }

    return self;
}

// создать все кнопки
void CChannelWnd::InitiateButtons()
{
    nodes::factory().get_channel_sheme(sheme_);

    nodes::factory().get_active_channels(activec_);

    channel_sheme sheme = filter(sheme_, activec_);
    for (unsigned index=0; index < sheme.objects.size(); ++index)
    {
        CreateButton(B_NAME, index);

        foreach(int channel, sheme.objects[index].channels)
        {
            CreateButton(channel, index);
        }
    }

    CRect rc; GetWindowRect(rc);

    if (!IsDocking())
    {
        MoveWindow(CRect(rc.TopLeft(), AdjustSize(rc.Size())));
    }
    AdjustButtonLayout();
    AdjustScrollBar();
}

static void button_destroyer(CWindow &wnd)
{
    if (wnd) wnd.DestroyWindow();
}

void CChannelWnd::DestroyButtons()
{
    foreach(Button& button, buttons_)
    {
        button.wnd.DestroyWindow();
    }
    buttons_.clear();
}

void CChannelWnd::InvalidateButtons()
{
    foreach(Button& btn, buttons_)
    {
        btn.wnd.Invalidate(FALSE);
    }
}

LRESULT CChannelWnd::OnCreate(LPCREATESTRUCT)
{
    if (!spin_buttons_font_)
    {
        spin_buttons_font_.CreatePointFont(80, "Tahoma");
        LOGFONT lf = {0};
        ::GetObject(spin_buttons_font_, sizeof(lf), (LPVOID)&lf);
        lf.lfWeight = FW_BOLD;

        spin_buttons_font_.DeleteObject();
        spin_buttons_font_.CreateFontIndirect(&lf);
    }

    toolbar_.Create(m_hWnd, rcDefault, "",
                    WS_CHILD|WS_VISIBLE|TBSTYLE_FLAT|TBSTYLE_LIST|CCS_TOP);
    toolbar_.SetButtonStructSize();

    CToolbarButtons main=IDR_STAGE_WND;
    main.Append(IDC_EDIT_ALL, 0, "A", TBSTYLE_AUTOSIZE);
    main.Append(IDC_EDIT_INVERT, 1, "B", TBSTYLE_AUTOSIZE);
    main.Apply(toolbar_);
    OnChangeLanguage();

    scroll_.Create(m_hWnd, rcDefault, "",
                   WS_CHILD|WS_VISIBLE|SBS_VERT|SBS_RIGHTALIGN);

    nodes::factory().get_root_node()->attach_listener(this);

    InitiateButtons();
    return 0;
}

LRESULT CChannelWnd::OnDestroy()
{
    nodes::factory().get_root_node()->detach_listener(this);

    DestroyButtons();
    return 0;
}

void CChannelWnd::OnChangeLanguage()
{
    SetWindowText(_lcs("Channels"));

    toolbar_.SetButtonInfo(0, 0x80000000|TBIF_TEXT, 0, 0, _lcs("All"), 0, 0, 0, 0);
    toolbar_.SetButtonInfo(1, 0x80000000|TBIF_TEXT, 0, 0, _lcs("Invert"), 0, 0, 0, 0);
}

void CChannelWnd::safe_on_restart()
{
    for(unsigned index=0; index < 64; ++index)
        activec_[index] = true;

    DestroyButtons();
    InitiateButtons();
    InvalidateButtons();

}

void CChannelWnd::select_channel(int ch)
{
    InvalidateButtons();
}




void CChannelWnd::safe_on_finish(process::prslt pr)
{
    nodes::presult r = boost::dynamic_pointer_cast<nodes::result>(pr);
    if (!r) return;

    activec_ = *r->working_channels;

    DestroyButtons();
    InitiateButtons();

    foreach(Button& btn, buttons_)
    {
        btn.wnd.Invalidate(FALSE);
    }
}

CRect CChannelWnd::AdjustRect(CRect rc) const
{
    ::AdjustWindowRectEx(rc, GetWindowLong(GWL_STYLE), false, GetWindowLong(GWL_EXSTYLE));
    return rc;
}

CSize CChannelWnd::GetButtonSize() const
{
    return CSize(25, 20);
}

LRESULT CChannelWnd::OnGetMinMaxInfo(LPMINMAXINFO lpm)
{
    CSize sz=AdjustRect(CRect(CPoint(), GetButtonSize())).Size();
    lpm->ptMinTrackSize.x=sz.cx*2;
    lpm->ptMinTrackSize.y=sz.cy*1;
    return 0;
}

static CSize set_layout(CSize sz, int count, bool noxchg=false)
{
    if (!noxchg) std::swap(sz.cx, sz.cy);

    sz.cy = std::max(sz.cy, (long)1);
    unsigned mx=(count + sz.cy - 1) / sz.cy ;
    sz.cx = std::max(1u, mx);

    if (!noxchg) std::swap(sz.cx, sz.cy);
    return sz;
}

CSize CChannelWnd::AdjustSize(CSize sz, UINT type)
{
    CSize but=GetButtonSize();
    sz.cy/=but.cy; sz.cx/=but.cx;

    sz=set_layout(sz, buttons_.size(),
                  type==WMSZ_BOTTOM||type==WMSZ_BOTTOMLEFT||type==WMSZ_BOTTOMRIGHT||
                  type==WMSZ_TOP||type==WMSZ_TOPLEFT||type==WMSZ_TOPRIGHT);

    sz.cy*=but.cy; sz.cx*=but.cx;
    CRect szr=CRect(CPoint(0,0), sz);
    sz=AdjustRect(szr).Size();
    return sz;
}

LRESULT CChannelWnd::OnMoving(UINT, LPRECT rect)
{
    static CRect prev;
    CRect maximum;
    ::SystemParametersInfo(SPI_GETWORKAREA, 0, &maximum, 0);

    if (rect->left < maximum.left+20 && rect->left < prev.left)
    {
        OffsetRect(rect, maximum.left-rect->left, 0);
    }
    if (rect->right> maximum.right-20 && rect->right > prev.right)
    {
        OffsetRect(rect, maximum.right-rect->right, 0);
    }
    if (rect->top< maximum.top+20 && rect->top < prev.top)
    {
        OffsetRect(rect, 0, maximum.top-rect->top);
    }
    if (rect->bottom> maximum.bottom-20 && rect->bottom > prev.bottom)
    {
        OffsetRect(rect, 0, maximum.bottom-rect->bottom);
    }

    prev=rect;
    return 0;
}

LRESULT CChannelWnd::OnSizing(UINT type, LPRECT rect)
{
    CSize but=GetButtonSize();
    CRect test=AdjustRect( CRect(0,0,0,0) );

    CSize sz=AdjustSize(
                 CSize(
                     rect->right-rect->left-test.Width(),
                     rect->bottom-rect->top-test.Height()), type);

    switch (type)
    {
    case WMSZ_BOTTOM: case WMSZ_BOTTOMLEFT: case WMSZ_BOTTOMRIGHT:
        rect->bottom=rect->top+sz.cy; break;
    case WMSZ_TOP: case WMSZ_TOPLEFT: case WMSZ_TOPRIGHT:
        rect->top=rect->bottom-sz.cy; break;
    }

    switch (type)
    {
    case WMSZ_LEFT: case WMSZ_BOTTOMLEFT: case WMSZ_TOPLEFT:
        rect->left=rect->right-sz.cx; break;
    case WMSZ_RIGHT: case WMSZ_BOTTOMRIGHT: case WMSZ_TOPRIGHT:
        rect->right=rect->left+sz.cx; break;
    }

    SetMsgHandled(false);
    return 0;
}

LRESULT CChannelWnd::OnSize(UINT, CSize sz)
{
    if (toolbar_)
    {
        toolbar_.AutoSize();
    }
    AdjustButtonLayout();
    AdjustScrollBar();
    return 0;
}


void CChannelWnd::serialization(serl::archiver &arc)
{
    arc.serial("docking", serial_dock_pos_.bDocking);
    arc.serial("side",  serial_dock_pos_.dockPos.dwDockSide);

    arc.serial("left",  serial_dock_pos_.rect.left);
    arc.serial("right", serial_dock_pos_.rect.right);
    arc.serial("top",   serial_dock_pos_.rect.top);
    arc.serial("bottom",serial_dock_pos_.rect.bottom);

    arc.serial("nBar", serial_dock_pos_.dockPos.nBar);
    arc.serial("fPctPos", serial_dock_pos_.dockPos.fPctPos);
    arc.serial("nWidth", serial_dock_pos_.dockPos.nWidth);
    arc.serial("nHeight", serial_dock_pos_.dockPos.nHeight);
}

void CChannelWnd::AdjustScrollBar()
{
    unsigned count=buttons_.size();
    unsigned cx=GetButtonSize().cx;
    unsigned cy=GetButtonSize().cy;

    CRect client; GetClientRect(client);

    unsigned dy=client.Height()/cy;
    unsigned dx=client.Width()/cx;

    SCROLLINFO si=
    {
        sizeof(SCROLLINFO),
        SIF_ALL,
        0, dy+hiddenrows_, dy,
        row_, row_
    };

    scroll_.SetScrollInfo(&si, true);
}

// расставить кнопки по местам
void CChannelWnd::AdjustButtonLayout()
{
    unsigned count=buttons_.size();

    // размеры кнопок
    unsigned cx=GetButtonSize().cx;
    unsigned cy=GetButtonSize().cy;

    CRect client; GetClientRect(client);

    if (toolbar_)
    {
        CRect tb;
        toolbar_.GetWindowRect(tb);
        ScreenToClient(tb);
        client.top=tb.bottom;
    }

    // максимальное количество по высоте
    dy_=client.Height()/cy;

    // максимальное количество кнопок по ширине
    unsigned dx=client.Width()/cx;

    // если места для кнопок нет, то все остальное не имеет смысла
    if (dx < 1 || dy_ < 1) return;

    // определяем необходимое количество рядов
    unsigned rowcount = 0;
    // при наличии скроллбара
    int rowcount_sb = 0;

    channel_sheme sheme = filter(sheme_, activec_);
    for (unsigned index=0; index < sheme.objects.size(); ++index)
    {
        rowcount += 1 + (sheme.objects[index].channels.size() + dx - 1) / dx;
        rowcount_sb += 1 + (sheme.objects[index].channels.size() + (dx-1) - 1) / (dx - 1);
    }

    // для скроллбара. Количество рядов, которые не поместились на экране
    hiddenrows_=0;

    // если кнопки не помещаются на экране
    if (dy_ < rowcount)
    {
        // используем скролл

        //рядов стало меньше...
        --dx;

        // столько рядов не помещается
        hiddenrows_=rowcount_sb - dy_;

        scroll_.MoveWindow( CRect(
                                CPoint(client.right - GetSystemMetrics(SM_CXVSCROLL), client.top),
                                CPoint(client.right, client.bottom) ));
        client.right-=GetSystemMetrics(SM_CXVSCROLL);

        // в дальнейшем используем это значение рядов
        rowcount = rowcount_sb;

        scroll_.ShowWindow(true);
    }
    else
    {
        // скролл не нужен
        scroll_.ShowWindow(false);
    }

    // номер ряда, с которого начинаем расставлять кнопки
    row_=std::min(row_, hiddenrows_+1);

    // отступ от края окна
    int deltax = (client.Width() - cx*dx)/2;
    int deltay = row_ * cy;
    unsigned index=0;

    // прячем кнопки, чтобы они не мелькали
    for (unsigned index=0; index<count; ++index)
        buttons_[index].wnd.ShowWindow(SW_HIDE);

    int x=0, y=0;

    for (unsigned obji=0; obji < sheme.objects.size() && index < count; ++obji)
    {
        // выводим заголовок
        if (y >= (int)row_)
        {
            std::string name = sheme.objects[obji].name;
            if (name == "") name = _ls("CHW-AllChannel#All channels");

            buttons_[index].wnd.SetWindowText(name.c_str());

            buttons_[index].wnd.MoveWindow(
                CRect(
                    CPoint(deltax, client.top+deltay+y*cy),
                    CSize(cx*dx, cy)));
            buttons_[index].wnd.ShowWindow(SW_SHOWNOACTIVATE);
        }
        ++index;
        ++y; // следующий ряд

        x=0;
        for (unsigned chn=0; chn < sheme.objects[obji].channels.size() && index < count; ++chn)
        {
            if (y >= (int)row_)
            {
                buttons_[index].wnd.MoveWindow(
                    CRect(
                        CPoint(deltax+x*cx, client.top+deltay+y*cy),
                        GetButtonSize()));
                buttons_[index].wnd.ShowWindow(SW_SHOWNOACTIVATE);
            }

            ++x;

            // если текущий ряд закончился начинаем новый ряд
            if (x>=(int)dx)
            {
                x=0;
                ++y;
            }
            ++index;
        }
        ++y; // следующий ряд
    }
}

static std::pair<int, int> get_number_by_hwnd(HWND hwnd)
{
    int param = GetWindowLong(hwnd, GWL_ID);
    int i=LOWORD(param), g=HIWORD(param);

    return std::make_pair(i, g);
}

LRESULT CChannelWnd::DrawItem(int, LPDRAWITEMSTRUCT dis)
{
    CWindow wnd(dis->hwndItem);
    CDCHandle prevdc(dis->hDC);
    CRect rc(dis->rcItem);

    CDC dc( ::CreateCompatibleDC(prevdc) );
    CBitmap bmp( ::CreateCompatibleBitmap(prevdc, rc.Width(), rc.Height()));
    dc.SelectBitmap(bmp);

    int id=0, group=0;
    tie(id, group)=get_number_by_hwnd(wnd);

    channel_sheme sheme = filter(sheme_, activec_);
    channel_object &object = sheme.objects[group];

    std::string string=CWindowText((HWND)wnd);

    bool istitle = (id == B_NAME);
    bool active=
        istitle
        ? object.hasactive()
        : sheme.isactive(id);
    bool halfactive = istitle && active && object.hasinactive();

    COLORREF back =
        istitle
        ? ::GetSysColor(COLOR_3DFACE)
        : active ? colors::by_chanel(id) : ::GetSysColor(COLOR_3DFACE);

    dc.FillRect(rc, CBrush().CreateSolidBrush(back));

    // Text out

    dc.SetTextColor(
        // если неполностью погашен
        halfactive ? 0x7f7f7f :

        // в противном случае текст выбирается согласно фона
        GetBValue(back) <= 0xFF &&
        GetRValue(back) <= 0xa0 &&
        GetGValue(back) <= 0x80
        ? 0xFFFFFF : 0x000000);

    dc.SetBkMode(TRANSPARENT);

    CFontHandle oldf=dc.SelectFont( spin_buttons_font_ );

    dc.DrawText(string.c_str(), string.size(), rc,
                DT_CENTER|DT_VCENTER|DT_SINGLELINE);

    dc.SelectFont(oldf);

    dc.DrawEdge(rc,
                active
                ? EDGE_SUNKEN
                : EDGE_RAISED,
                BF_RECT);

    if (PlotManager::inst().GetActiveChannel()==id)
    {
        CBrush brush( (HBRUSH)GetStockObject(NULL_BRUSH) );
        CBrushHandle oldb=dc.SelectBrush(brush);

        CPen pen( CreatePen(PS_SOLID, 3, 0x0000FF) );
        CPenHandle oldp=dc.SelectPen(pen);

        dc.Ellipse(rc);

        CPen pen2 ( CreatePen(PS_SOLID, 1, 0xFFFFFF) );
        dc.SelectPen(pen2);

        dc.Ellipse(rc);

        dc.SelectBrush(oldb);
        dc.SelectPen(oldp);
    }

    prevdc.BitBlt(0, 0, rc.Width(), rc.Height(), dc, 0, 0, SRCCOPY);

    return 0;
}

void    CChannelWnd::update_nodes()
{
    nodes::factory().set_channel_sheme(sheme_);
}

LRESULT CChannelWnd::OnRButton(UINT, int, HWND)
{
    return 0;
}

LRESULT CChannelWnd::OnClicked(UINT, int, HWND hwnd)
{
    unsigned prevrow=row_;

    int id=0, group=0;
    tie(id, group)=get_number_by_hwnd(hwnd);

    if (id == B_NAME)
    {
        sheme_.objects[group].setactive(
            sheme_.objects[group].hasinactive() );
    }
    else
    {
        checks_.at(id-1)=!checks_.at(id-1);
        sheme_.setactive( id, !sheme_.isactive(id) );
    }

    update_nodes();
    InvalidateButtons();

    ::InvalidateRect(hwnd, 0, true);

    ::SendMessage(GetActiveWindow(), WM_FOCUSONACTIVEVIEW, 0, 0);
    return 0;
}

void CChannelWnd::ClickAll()
{
    bool all=true;
    for (unsigned index=0; index<buttons_.size(); ++index)
    {
        int id=0, group=0;
        tie(id, group)=get_number_by_hwnd(buttons_[index].wnd);
        if (id != B_NAME)
        {
            all=all && checks_[ id-1 ];
        }
    }

    for (unsigned index=0; index<buttons_.size(); ++index)
    {
        int id=0, group=0;
        tie(id, group)=get_number_by_hwnd(buttons_[index].wnd);
        if (id!=B_NAME)
        {
            sheme_.setactive( id, !all);

            checks_[id-1]=!all;
            buttons_[index].wnd.Invalidate();
        }
    }

    update_nodes();
    ::SendMessage(GetActiveWindow(), WM_FOCUSONACTIVEVIEW, 0, 0);
}

void CChannelWnd::ClickInvert()
{
    for (unsigned index=0; index<buttons_.size(); ++index)
    {
        int id=0, group=0;
        tie(id, group)=get_number_by_hwnd(buttons_[index].wnd);
        if (id!=B_NAME)
        {
            sheme_.setactive( id, !sheme_.isactive(id) );

            checks_[id-1]=!checks_[id-1];
            buttons_[index].wnd.Invalidate();
        }
    }
    update_nodes();
    ::SendMessage(GetActiveWindow(), WM_FOCUSONACTIVEVIEW, 0, 0);
}


LRESULT CChannelWnd::OnEraseBackground(HDC hdc)
{
    CBrush brush( GetSysColorBrush(COLOR_3DFACE));
    CRect rc; GetClientRect(rc);
    FillRect(hdc, rc, brush);
    return 0;
}

LRESULT CChannelWnd::OnMouseWheel(UINT, short z, CPoint)
{
    if (hiddenrows_)
    {
        unsigned prevr=row_;
        row_=std::max(0, std::min((int)hiddenrows_, (int)row_-z/WHEEL_DELTA));
        if (prevr!=row_) AdjustButtonLayout();
    }
    return 0;
}

LRESULT CChannelWnd::OnSetFocus(HWND)
{
    return 0;
}

LRESULT CChannelWnd::OnLostFocus(HWND)
{
    return 0;
}

LRESULT CChannelWnd::PostToParent(UINT a, int b, HWND w)
{
    if (b==IDC_EDIT_ALL) ClickAll();
    if (b==IDC_EDIT_INVERT) ClickInvert();
    return 0;
}

LRESULT CChannelWnd::OnVScroll(int tp, short pos, HWND)
{
    switch (tp)
    {
    case SB_PAGEDOWN:
        row_+=dy_;
        row_=std::min(row_, hiddenrows_+1);
        break;
    case SB_PAGEUP:
        row_-=dy_;
        row_=std::max((int)row_, 0);
        break;
    case SB_BOTTOM:
        row_=hiddenrows_+1; break;
    case SB_TOP:
        row_=0; break;
    case SB_LINEDOWN:
        if (row_<=hiddenrows_) row_+=1; break;
    case SB_LINEUP:
        if (row_) row_-=1; break;
        break;
    case SB_THUMBPOSITION:
    case SB_THUMBTRACK:
        row_=pos;
        break;
    }
    AdjustButtonLayout();
    AdjustScrollBar();
    return 0;
}
