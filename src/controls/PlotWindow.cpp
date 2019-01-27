#include "stdafx.h"

#include "PlotWindow.h"

#include "PlotProperties.h"

#include "PlotPool.h"
#include "plot/range.h"
#include "plot/screen.h"

#include "utilites/localizator.h"

#include "utilites/document.h"
#include "aera/wtladdons.h"

#include "utilites/foreach.hpp"
#include <boost/tuple/tuple.hpp>

#include "data/stage.h"
#include "data/nodeFactory.h"

#include "data/LinearSetup.h"
#include "data/PlanarSetup.h"

#include "utilites/strings.h"

//////////////////////////////////////////////////////////////////////////

#include "TileView/TVFrame.h"
#include "data/VesselSetup.h"
#include "data/VesselStructure.h"
#include "data/Collector.h"
#include "data/locator.h"

//////////////////////////////////////////////////////////////////////////

serl::mark_as_rtti< CTVCreatorImpl<CPlotView> > INSTANCE;

//////////////////////////////////////////////////////////////////////////

static bool is_addition_mode()
{
    return
        GetAsyncKeyState(VK_SHIFT)<0;
}

static bool is_substraction_mode()
{
    return
        !is_addition_mode() &&
        GetAsyncKeyState(VK_MENU)<0;
}

static bool is_selection_mode()
{
    return
        is_addition_mode() ||
        is_substraction_mode();
}

static bool is_zoom_mode()
{
    return
        !is_selection_mode() &&
        GetAsyncKeyState(VK_CONTROL)<0;
}

CPlotView::CPlotView(bool mode, bool ro)
    : model_(mode),
      select_rect_mode_(false),
      bitmap_(new plot::darray(1, 1)),
      sel_rect_(0,0,0,0),
      panmode_(false),
      selecting_range_(false),
      read_only_(ro),
      range_start_(0), range_end_(0),
      current_stage_(UINT_MAX)
{
    model_.attach_listener(this);
    PlotManager::instance()->attach_view(this);
}

CPlotView::~CPlotView()
{
    model_.detach_listener(this);
    PlotManager::instance()->detach_view(this);
}

LRESULT CPlotView::OnCreate(LPCREATESTRUCT)
{    
    OnWindowPosChanged(0, 0, 0);    
    return 0;
}

LRESULT CPlotView::OnDestroy()
{
    return 0;
}


LRESULT CPlotView::OnFocus(HWND next)
{
    Invalidate();
    return 0;
}

void CPlotView::OnSizeChanged()
{
    CSize sz = frame_.Size();
    model_.set_dimensions(sz.cx, sz.cy);
}

LRESULT CPlotView::OnRButtonDown(UINT, CPoint pt)
{
    SetFocus();
    if (!read_only_ && !select_rect_mode_ && !model_.get_select_range_mode())
    {
        CMenu menu( ::CreatePopupMenu() );
        menu.AppendMenu(MFT_STRING, IDC_PLOT_ZOOMALL, _lcs("Zoom all#Zoom &best"));
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MFT_STRING, IDC_PLOT_COPYCLIP, _lcs("copyclip#&Export graph &image.."));
        menu.AppendMenu(MFT_STRING, IDC_TILE_COPYCLIP, _lcs("copyclip_page#Export &page image..."));
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MFT_STRING, IDC_TILE_SWAP,    _lcs("tileview-swap#&Swap windows\tDblclk"));
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MFT_STRING, 1,                _lcs("tileview-delete#&Delete window"));
        menu.AppendMenu(MFT_SEPARATOR);
        menu.AppendMenu(MFT_STRING, 2,                _lcs("properties#&Properies"));

        ::SetCursor( ::LoadCursor(NULL, IDC_ARROW) );

        ClientToScreen(&pt);
        int ret=menu.TrackPopupMenu(
                    TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD,
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

            CPlotPropertyDlg dialog(&model_, m_hWnd);
            dialog.DoModal();
        } break;
        case IDC_TILE_SWAP:
            UpdateWindow();
            StartDrag(pt);
            break;
        default:
            OnPlotCommand(0, ret, 0);
            break;
        }
    }
    return 0;
}

void CPlotView::OnFinalMessage(HWND)
{
    delete this;
}

void CPlotView::DoInvalidate()
{
    if (m_hWnd) Invalidate();
}

LRESULT CPlotView::OnLButtonDbl(UINT, CPoint pt)
{
    if (read_only_)
    {
    }
    else if (model_.get_select_range_mode())
    {
        ::PostMessage(GetParent(), UM_INVALIDATEFRAME, 0, 0);
    }
    else
    {

        ClientToScreen(&pt);
        StartDrag(pt);
    }
    return 0;
}

LRESULT CPlotView::OnLButtonDown(UINT keys, CPoint pt)
{
    if (!read_only_)
    {
        SetFocus();

        if (model_.get_select_range_mode())
        {
            StartSelectRange(pt);
        }
        else if (PlotManager::instance()->IsPanMode())
        {
            StartPan(pt);
        }
        else if (is_zoom_mode() || PlotManager::instance()->IsZoomMode() )
        {
            StartZoom(pt);
        }
        else
        {
            StartSelectRect(pt);
        }
        Invalidate();
    }
    return 0;
}

void CPlotView::StartSelectRect(CPoint pt)
{
    if (model_.get_plottype()==scatter &&
            model_.get_tddata()==0)
    {
        select_rect_mode_=true;
        startdrag_=pt;
        SetCapture();
    }
}

void CPlotView::StartZoom(CPoint pt)
{
    PlotManager::instance()->SetZoomMode(true);
    select_rect_mode_=true;
    startdrag_=pt;
    SetCapture();
}

void CPlotView::DoSelectRect(CPoint pt)
{
    sel_rect_=CRect(startdrag_, pt);
    Invalidate();
}

void CPlotView::EndZoom(CPoint pt)
{
    sel_rect_.NormalizeRect();
    sel_rect_.IntersectRect(sel_rect_, frame_);

    ZoomAxesFrame(
        sel_rect_.left, sel_rect_.right,
        sel_rect_.top, sel_rect_.bottom);

    sel_rect_=CRect(0,0,0,0);
    select_rect_mode_=false;
}

void CPlotView::EndSelectRect(CPoint pt)
{
    CRect temp_selection=sel_rect_;

    DoSelectRect(pt);
    SelectRect(temp_selection);

    sel_rect_=CRect(0,0,0,0);
    select_rect_mode_=false;
}

// опрелеляем, на какой стадии находится курсор
unsigned CPlotView::CursorAtRangeSelector(CPoint pt)
{
    int start=0, end=0;
    for (unsigned index=0; index<stages_.size(); ++index)
    {
        start=map_to(0, stages_[index].self->start);
        end  =map_to(0, stages_[index].self->end);

        if (start > end) std::swap(start, end);

        if (abs(pt.x-start)<3 || abs(pt.x-end)<3)
            return index;

        if ( start <= pt.x && pt.x <= end)
            return index;
    }
    return UINT_MAX;
}

bool CPlotView::CursorAtRangeBorder(CPoint pt)
{
    for (unsigned index=0; index<stages_.size(); ++index)
    {
        int start=map_to(0, stages_[index].self->start);
        int end  =map_to(0, stages_[index].self->end);
        if (abs(pt.x-start)<3 || abs(pt.x-end) <3)
            return true;
    }
    return false;
}

void CPlotView::StartSelectRange(CPoint pt)
{
    current_stage_=CursorAtRangeSelector(pt);

    if (current_stage_==UINT_MAX)
    {
        for (unsigned index=0; index<stages_.size(); ++index)
        {
            stage &some_stage=*stages_[index].self;
            if (some_stage.start==some_stage.end)
            {
                current_stage_=index;
                range_start_=pt.x;
                range_end_  =pt.x;
                break;
            }
        }

        if (current_stage_==UINT_MAX)
        {
            plot_stage ps;
            ps.num = 0;
            ps.self = shared_ptr<stage>(new stage(_lcs("stages-new#new stage"), 0, 0));
            stages_.push_back(ps);

            current_stage_=stages_.size()-1;
            range_start_=pt.x;
            range_end_  =pt.x;
        }
    }
    else
    {
        range_start_=map_to(0, stages_[current_stage_].self->start);
        range_end_  =map_to(0, stages_[current_stage_].self->end);
        if (!CursorAtRangeBorder(pt))
        {
            Invalidate();
            return;
        }
    }

    int point=std::min(std::max(pt.x, frame_.left), frame_.right);
    if (range_end_==range_start_)
    {
        range_end_=range_start_=point;
    }
    if (abs(point-range_end_) > abs(point-range_start_))
    {
        std::swap(range_start_, range_end_);
    }

    range_end_=point;
    selecting_range_=true;
    Invalidate();
    SetCapture();
}

void CPlotView::DoSelectRange(CPoint pt)
{
    range_end_=std::min(std::max(pt.x, frame_.left), frame_.right);

    if (current_stage_<stages_.size())
    {
        stages_[current_stage_].self->start=GetRangeValue(range_start_).second;
        stages_[current_stage_].self->end  =GetRangeValue(range_end_).second;
    }

    ::PostMessage(GetParent(), UM_INVALIDATEFRAME, 0, 0);
    Invalidate();
}

static bool srt(plot_stage const &a, plot_stage const &b)
{
    return a.self->start < b.self->start;
}

template<class T> inline void sort(T &a, T &b)
{
    if (a>b) std::swap(a, b);
}

void CPlotView::EndSelectRange(CPoint pt)
{
    selecting_range_=false;
    if (!(current_stage_<stages_.size())) return;

    shared_ptr<stage> tmp=stages_[current_stage_].self;
    sort(tmp->start, tmp->end);
    std::sort(STL_II(stages_), srt);

    current_stage_ = 0;
    foreach(plot_stage const& s, stages_)
    {
        if (s.self==tmp) break;
        current_stage_++;
    }
    //current_stage_=std::find(STL_II(stages_), tmp)-stages_.begin();

    if (range_start_==range_end_ && current_stage_<stages_.size())
    {
        stages_.erase(stages_.begin()+current_stage_);
    }

    ::PostMessage(GetParent(), UM_INVALIDATEFRAME, 0, 0);
    ReleaseCapture();
}

LRESULT CPlotView::OnMouseLeave()
{
    PlotManager::instance()->SetActiveChannel(0);
    return 0;
}

LRESULT CPlotView::OnMouseMove(UINT keys, CPoint pt)
{
    TRACKMOUSEEVENT me= {sizeof(TRACKMOUSEEVENT), TME_LEAVE, m_hWnd, HOVER_DEFAULT};
    ::TrackMouseEvent(&me);

    if (!read_only_)
    {
        if (selecting_range_)
        {
            DoSelectRange(pt);
        }
        else if (panmode_)
        {
            PanPlot(pan_point_, pt);
        }
        else if (select_rect_mode_)
        {
            DoSelectRect(pt);
        }
    }

    if (frame_.PtInRect(pt) && model_.get_actualcolormode()==bychannel)
    {
        CDC dc=GetDC();

        CPoint points[]=
        {   pt,
            pt+CSize(0, 1),
            pt+CSize(0, -1),
            pt+CSize(1, 0),
            pt+CSize(-1, 0),
            pt+CSize(1, 1),
            pt+CSize(1, -1),
            pt+CSize(-1, 1),
            pt+CSize(-1, -1),
        };

        int channel;
        for (unsigned i=0; i<SIZEOF(points); ++i)
        {
            COLORREF color=dc.GetPixel(points[i]);
            channel=colors::channel_by_color(color);
            if (channel) break;
        }

        PlotManager::instance()->SetActiveChannel(channel);
    }
    else
    {
        PlotManager::instance()->SetActiveChannel(0);
    }

    Invalidate();
    return 0;
}

void CPlotView::PanPlotXAxe(CPoint start, CPoint end)
{
    double start_value=
        xrange_.unmap_double(
            start.x-frame_.left, frame_.Width());
    double end_value=
        xrange_.unmap_double(
            end.x-frame_.left, frame_.Width());

    double delta=start_value-end_value;
    axe_info inf=boost::get<0>(pan_buffer_);

    inf.lo+=delta;
    inf.hi+=delta;
    model_.set_xminmax(false, inf.lo, inf.hi);
}

void CPlotView::PanPlotYAxe(CPoint start, CPoint end)
{
    double start_value=
        yrange_.unmap_double(
            frame_.bottom-start.y, frame_.Height());

    double end_value=
        yrange_.unmap_double(
            frame_.bottom-end.y, frame_.Height());

    axe_info inf=boost::get<1>(pan_buffer_);

    if (inf.log_scale)
    {
        double delta=log10(start_value)-log10(end_value);
        inf.lo=pow(10., log10(inf.lo) + delta);
        inf.hi=pow(10., log10(inf.hi) + delta);
    }
    else
    {
        double delta=start_value-end_value;
        inf.lo+=delta;
        inf.hi+=delta;
    }
    model_.set_yminmax(false, inf.lo, inf.hi);
}

void CPlotView::StartPan(CPoint start)
{
    panmode_=true;
    track_values_=false;

    pan_prev_point_=pan_point_=start;
    pan_buffer_=
        boost::tuples::make_tuple(
            model_.get_xaxe_info(),
            model_.get_yaxe_info());

    PlotManager::instance()->OnZoom(this);
    PlotManager::instance()->SetWideOperation(true);
    OnSetCursor();
    SetCapture();
    Invalidate();
}

void CPlotView::PanPlot(CPoint start, CPoint end)
{
    CSize d = pan_prev_point_ - end;
    if (abs(d.cx)+abs(d.cy) < 2)
        return;

    pan_prev_point_ = end;

    if (model_.get_xaxe_info().chartype==aera::C_Time && model_.get_yaxe_info().auto_scale)
    {
        PanPlotXAxe(pan_point_, end);
    }
    else
    {
        PanPlotXAxe(pan_point_, end);
        PanPlotYAxe(pan_point_, end);
    }

    model_.restart();
}

void CPlotView::EndPan(CPoint end)
{
    PanPlot(CPoint(), end);

    panmode_=false;
    track_values_=true;
    PlotManager::instance()->SetWideOperation(false);

    ReleaseCapture();
    OnSetCursor();
}

void CPlotView::ZoomXAxeFrame(int f, int s)
{
    ZoomAxesFrame(f, s, 0, 0);
}

void CPlotView::ZoomYAxeFrame(int f, int s)
{
    ZoomAxesFrame(0, 0, f, s);
}

void CPlotView::ZoomAxesFrame(int l, int r, int t, int b)
{
    CRect frame=get_frame();
    double dl=xrange_.unmap_double( l -frame.left, frame.Width() );
    double dr=xrange_.unmap_double( r -frame.left, frame.Width() );
    double dt=yrange_.unmap_double( frame.bottom-t, frame.Height() );
    double db=yrange_.unmap_double( frame.bottom-b, frame.Height() );

    PlotManager::instance()->Zoom(this, dl, dr, dt, db);
}

static std::string pack_parentethis(std::string val)
{
    return tokenizer::join("(", val, ")");
}

void CPlotView::SelectRect(CRect sel)
{
    sel.NormalizeRect();
    std::string xa=xaxe_mark(sel.left);
    std::string xb=xaxe_mark(sel.right);
    std::string ya=yaxe_mark(sel.bottom);
    std::string yb=yaxe_mark(sel.top);

    if (model_.get_xaxe_info().auto_scale)
    {
        if (sel.left <= frame_.left) xa="";
        if (sel.right>= frame_.right) xb="";
    }

    if (model_.get_yaxe_info().auto_scale)
    {
        if (sel.bottom>=frame_.bottom) ya="";
        if (sel.top   <= frame_.top) yb="";
    }

    std::string x = aera::traits::get_short_name((aera::chars)model_.get_xaxe_info().chartype) ;
    std::string y = aera::traits::get_short_name((aera::chars)model_.get_yaxe_info().chartype) ;

    std::string fs=tokenizer::join(
                tokenizer::join(
                           (xa.size() ? tokenizer::join(xa, "<=", x) : ""),
                           "&&",
                           (xb.size() ? tokenizer::join(x, "<=", xb) : "")),
                       "&&",
                       tokenizer::join(
                           (ya.size() ? tokenizer::join(ya, "<=", y) : ""),
                           "&&",
                           (yb.size() ? tokenizer::join(y, "<=", yb) : "")) );
    if (fs.empty()) fs="1";


    if (sel.top==sel.bottom || sel.right==sel.left)
    {
        fs="0";
    }

    int mode = selection_set;
    if (is_addition_mode())          mode = selection_add;
    else if (is_substraction_mode()) mode = selection_sub;

    nodes::factory().modify_selection(mode, fs, model_.get_parent().get());

    sel_rect_=CRect(0,0,0,0);
}

LRESULT CPlotView::OnLButtonUp(UINT keys, CPoint pt)
{
    if (axedragvalues_.first>axedragvalues_.second)
    {
        std::swap(axedragvalues_.first, axedragvalues_.second);
    }

    if (read_only_)
    {
        return 0;
    }

    if (model_.get_select_range_mode())
    {
        EndSelectRange(pt);
    }

    if (panmode_)
    {
        EndPan(pt);
    }

    if (PlotManager::instance()->IsZoomMode())
    {
        EndZoom(pt);
    }

    if (select_rect_mode_)
    {
        EndSelectRect(pt);
    }

    ReleaseCapture();
    Invalidate();
    return 0;
}



bool CPlotView::get_axedrag() const
{
    return select_rect_mode_;
}

//std::pair<int, int>  CPlotView::get_axedragvalues() const
//{
//    return axedragvalues_;
//}

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

void CPlotView::SetCaption()
{
    std::string location_title;
    location::Setup sp;

    bool fixed = model_.get_location_setup(sp);
    if (fixed || model_.get_working_location_setup(sp))
    {
        location_title = sp.name();
        location_title = strlib::head(location_title, " ");
        if (fixed) location_title = "# " + location_title;
        location_title = "[" + location_title + "] ";
    }

    std::string y = aera::traits::get_wide_name(static_cast<aera::chars>(model_.get_yaxe_info().chartype));
    std::string x = aera::traits::get_wide_name(static_cast<aera::chars>(model_.get_xaxe_info().chartype));

    std::string uy = model_.get_yaxe_unit_name();
    std::string ux = model_.get_xaxe_unit_name();

    uy = tokenizer::join(" [", uy, "]");
    ux = tokenizer::join(" [", ux, "]");

    plottype_t pty=model_.get_plottype();

    std::string type=
        pty==scatter ?      _ls("Correlation") :
        pty==history ?      _ls("History") :
        pty==distribution ? _ls("Distribution") :
        pty==summation ?    _ls("Summary") :
        _ls("Accumulation") ;

    if (model_.get_tddata())
        type=_ls("plot-TDD#TDD")+": "+type;

    xcaption_=x + ux;
    ycaption_=y + uy;
    caption_=location_title + type+": " + ycaption_ + _ls(" vs ") + xcaption_;
}


void CPlotView::DrawTimemark(CDCHandle dc, CRect frame, double dbl) const
{
    CRect outframe=MakeOutframe(frame);

    plot::range xaxe=xrange_;
    int x= scale(dbl,
                 xaxe.get_left(), xaxe.get_right(),
                 frame.left, frame.right);

    if (outframe.left < x && x < outframe.right)
    {
        dc.SetBkMode(TRANSPARENT);

        CPen pen( ::CreatePen(PS_DASH, 1, clrTimemark) );
        CPenHandle oldp=dc.SelectPen(pen);

        dc.MoveTo(x, frame.top);
        dc.LineTo(x, frame.bottom);

        dc.SelectPen(oldp);
    }
}

void CPlotView::DrawTimemarks(CDCHandle dc, CRect frame)
{
    if (model_.get_xaxe_info().chartype==aera::C_Time)
    {
        plot::range xaxe=xrange_;

        std::vector<double> timemarks = model_.get_timemarks();
        std::for_each(STL_II(timemarks), bind(&CPlotView::DrawTimemark, this, dc, frame, _1));
    }
}

void CPlotView::DrawSensors(CDCHandle dc, CRect frame, location::LinearSetup &linear)
{
    bool linear_graph =
            model_.get_xaxe_info().chartype==aera::C_CoordX
            || model_.get_yaxe_info().chartype==aera::C_CoordX;

    if (!linear_graph) return;

    CFont font; font.CreatePointFont(80, "Tahoma", NULL, true);
    CFontHandle oldf=dc.SelectFont(font);

    CPen pen( CreatePen(PS_SOLID, 1, copyclip_ ? 0x7F7F7F : 0x7F7F7F) );
    CPenHandle open=dc.SelectPen( pen );

    dc.SetBkColor(0);
    dc.SetTextColor(copyclip_ ? 0x7F7F7F : 0x7F7F7F);
    dc.SetBkMode(TRANSPARENT);

    std::map<int, double> channels=linear.get_sensors();

    typedef std::map<int, double> tmap;
    foreach(tmap::value_type iter, channels)
    {
        int channel=iter.first;
        double coord=iter.second;

        std::string text=strlib::strf("%d", channel);
        CSize sz; dc.GetTextExtent(text.c_str(), -1, &sz);

        int x, y;
        if (model_.get_xaxe_info().chartype==aera::C_CoordX)
        {
            x=xrange_.map_to( coord, frame.Width());
            y=sz.cy/2;
        }
        else
        {
            x=frame.Width()-sz.cx/2;
            y=yrange_.map_to( coord, frame.Height());
        }

        CRect rc(CPoint(x+frame_.left-sz.cx/2, y+frame_.top-sz.cy/2), sz);
        dc.DrawText(text.c_str(), -1, rc, DT_CENTER|DT_VCENTER);

        rc.InflateRect(2, 2);
        dc.MoveTo(rc.TopLeft());
        dc.LineTo(rc.right, rc.top);
        dc.LineTo(rc.BottomRight());
        dc.LineTo(rc.left, rc.bottom);
        dc.LineTo(rc.TopLeft());

        std::string f = _ls("Sensor:")  + " %d";
        AddToolTipRegion(rc, strlib::strf(f.c_str(), text));
    }

    dc.SelectPen(open);
    dc.SelectFont(oldf);
}

void CPlotView::DrawSensors(CDCHandle dc, CRect frame, location::PlanarSetup &vessel)
{
    bool planar_graph =
        (model_.get_xaxe_info().chartype == aera::C_CoordX || model_.get_xaxe_info().chartype == aera::C_CoordY)
            &&
        (model_.get_yaxe_info().chartype == aera::C_CoordX || model_.get_yaxe_info().chartype == aera::C_CoordY)
            &&
        (model_.get_xaxe_info().chartype != model_.get_yaxe_info().chartype)
            &&
        (model_.get_plottype() == scatter);

    if (!planar_graph) return;

    CFont font; font.CreatePointFont(80, "Tahoma", NULL, true);
    CFontHandle oldf=dc.SelectFont(font);

    CPen pen( CreatePen(PS_SOLID, 2, copyclip_ ? 0x7F7F7F : 0x7F7F7F) );
    CPenHandle open=dc.SelectPen( pen );

    dc.SetBkColor(0);
    dc.SetTextColor(copyclip_ ? 0x7F7F7F : 0x7F7F7F);
    dc.SetBkMode(TRANSPARENT);

    // рисуем границы сосуда
    double vel  = vessel.get_velocity().get(location::m_s);
    double vert = vessel.get_vertical_diam().get_distance(vel);
    double hor = vessel.get_horizontal_diam().get_distance(vel);
    double wrap = vert ? vert * M_PI : hor * M_PI;

    bool coordinate_x_is_absciss = model_.get_xaxe_info().chartype==aera::C_CoordX;

    if (wrap)
    {
        if (coordinate_x_is_absciss == (bool)vert)
        {
            int x1 = xrange_.map_to(0, frame_.Width()) + frame_.left;
            int x2 = xrange_.map_to(wrap, frame_.Width()) + frame_.left;

            dc.MoveTo(x1, frame.top); dc.LineTo(x1, frame.bottom);
            dc.MoveTo(x2, frame.top); dc.LineTo(x2, frame.bottom);
        }
        else
        {
            int y1 = yrange_.map_to(0, -frame_.Height()) + frame_.bottom;
            int y2 = yrange_.map_to(wrap, -frame_.Height()) + frame_.bottom;

            dc.MoveTo(frame.left, y1); dc.LineTo(frame.right, y1);
            dc.MoveTo(frame.left, y2); dc.LineTo(frame.right, y2);
        }
    }

    // рисуем каналы
    using namespace location;
    std::map<int, planar_coords> sensors=vessel.get_sensors();

    typedef std::map<int, planar_coords> tmap;
    foreach(tmap::value_type iter, sensors)
    {
        int channel=iter.first;
        double cx=iter.second.x;
        double cy=iter.second.y;

        std::string text=strlib::strf("%d", channel);
        CSize sz; dc.GetTextExtent(text.c_str(), -1, &sz);

        int x, y;
        if (coordinate_x_is_absciss)
        {
            x=xrange_.map_to( cx, frame.Width()) + frame_.left;
            y=yrange_.map_to( cy, -frame.Height()) + frame_.bottom;
        }
        else
        {
            x=xrange_.map_to( cy, frame.Width() ) + frame_.left;
            y=yrange_.map_to( cx, -frame.Height() ) + frame_.bottom;
        }

        CRect rc(CPoint(x-sz.cx/2, y-sz.cy/2), sz);
        dc.DrawText(text.c_str(), -1, rc, DT_CENTER|DT_VCENTER);

        rc.InflateRect(2, 2);
        dc.MoveTo(rc.TopLeft());
        dc.LineTo(rc.right, rc.top);
        dc.LineTo(rc.BottomRight());
        dc.LineTo(rc.left, rc.bottom);
        dc.LineTo(rc.TopLeft());

        std::string f = _ls("Sensor:")  + " %d";
        AddToolTipRegion(rc, strlib::strf(f.c_str(), text));
    }

    dc.SelectPen(open);
    dc.SelectFont(oldf);
}

void CPlotView::DrawSensors(CDCHandle dc, CRect frame, location::VesselSetup &setup)
{
    using namespace location;

    bool planar_graph =
        (model_.get_xaxe_info().chartype == aera::C_CoordX || model_.get_xaxe_info().chartype == aera::C_CoordY)
            &&
        (model_.get_yaxe_info().chartype == aera::C_CoordX || model_.get_yaxe_info().chartype == aera::C_CoordY)
            &&
        (model_.get_xaxe_info().chartype != model_.get_yaxe_info().chartype)
            &&
        (model_.get_plottype() == scatter);

    if (!planar_graph) return;

    CFont font; font.CreatePointFont(80, "Tahoma", 0, true);
    CFontHandle oldf=dc.SelectFont(font);

    CPen pen( CreatePen(PS_SOLID, 1, copyclip_ ? 0x7F7F7F : 0x7F7F7F) );
    CPenHandle open=dc.SelectPen( pen );

    dc.SetBkColor(0);
    dc.SetTextColor(copyclip_ ? 0x7F7F7F : 0x7F7F7F);
    dc.SetBkMode(TRANSPARENT);

    bool coordinate_x_is_absciss = model_.get_xaxe_info().chartype==aera::C_CoordX;

    // рисуем границы сосуда

    VesselStructure const& vessel = setup.get_vessel();
    for(unsigned index=0; index < vessel.size(); ++index)
    {
        PVElement elem = vessel.element(index);

        double y1 = elem->bottom_y;
        double y2 = y1 + elem->height_y;

        double x1 = 0;
        double x2 = elem->get_width();

        if (!coordinate_x_is_absciss)
        {
            std::swap(x1,y1);
            std::swap(x2,y2);
        }

        int xx1 = xrange_.map_to(x1, frame.Width()) + frame.left;
        int xx2 = xrange_.map_to(x2, frame.Width()) + frame.left;
        int yy1 = yrange_.map_to(y1, -frame.Height()) + frame.bottom;
        int yy2 = yrange_.map_to(y2, -frame.Height()) + frame.bottom;

        if (vessel[index].type == VesselPart::Cylinder)
        {
            dc.MoveTo(xx1, yy1);
            dc.LineTo(xx1, yy2);
            dc.LineTo(xx2, yy2);
            dc.LineTo(xx2, yy1);
            dc.LineTo(xx1, yy1);
        } else
        {
            std::vector<lxy_coords> coords;
            create_net_map(elem, coords);

            bool firstly = 1;
            foreach(lxy_coords const& lxy, coords)
            {
                double kx = x1 + lxy.x;
                double ky = y1 + lxy.y;
                if (!coordinate_x_is_absciss)
                {
                    ky = y1 + lxy.x;
                    kx = x1 + lxy.y;
                }

                int xx = xrange_.map_to(kx, frame.Width()) + frame.left;
                int yy = yrange_.map_to(ky, -frame.Height()) + frame.bottom;

                if (firstly) dc.MoveTo(xx,yy), firstly = false;
                else dc.LineTo(xx,yy);
            }
        }
    }

    // рисуем каналы
    VesselSensors const& sensors = setup.get_sensors();
    foreach(VesselCoords const& coords, sensors)
    {
        int channel=coords.number;
        double cx = vessel.to_nx(coords.global);
        double cy = coords.global.y;

        std::string text=strlib::strf("%d", channel);
        CSize sz; dc.GetTextExtent(text.c_str(), -1, &sz);

        if (!coordinate_x_is_absciss)
        {
            std::swap(cx, cy);
        }
        int x=xrange_.map_to( cx, frame.Width() ) + frame_.left;
        int y=yrange_.map_to( cy, -frame.Height() ) + frame_.bottom;

        CRect rc(CPoint(x-sz.cx/2, y-sz.cy/2), sz);
        dc.DrawText(text.c_str(), -1, rc, DT_CENTER|DT_VCENTER);

        rc.InflateRect(2, 2);
        dc.MoveTo(rc.TopLeft());
        dc.LineTo(rc.right, rc.top);
        dc.LineTo(rc.BottomRight());
        dc.LineTo(rc.left, rc.bottom);
        dc.LineTo(rc.TopLeft());

        std::string f = _ls("Sensor:")  + " %d";
        AddToolTipRegion(rc, strlib::strf(f.c_str(), text));
    }

    dc.SelectPen(open);
    dc.SelectFont(oldf);
}

void CPlotView::DrawClusters(CDCHandle dc, CRect frame)
{
    if (!get_plot_params().use_clusters)
        return;

    using namespace location;
    pclusters cls = model_.get_clusters();
    if (!cls) return;

    COLORREF cluster_classes_normal[] =
    {
        0x009966, // зеленый
        0xcc9900, // голубой
        0x00cccc, // желтый
        0x0099ff, // оранжевый
        0x0000ff  // красный
    };

    COLORREF cluster_classes_copyclip[] =
    {
        0x99CC99, // зеленый    // 2
        0xffcc99, // голубой    // 3
        0x99ffff, // желтый     // 4
        0x0099ff, // оранжевый  // 5
        0x6666ff  // красный    // 6
    };

    foreach(cluster const& it, cls->items)
    {
        std::vector<double> xs, ys;
        foreach(cluster::point const &p, it.points)
        {
            xs.push_back(xrange_.map_to( p.x, frame.Width()) + frame_.left);
            ys.push_back(yrange_.map_to( p.y, -frame.Height()) + frame_.bottom);
        }

        // подбираем цвет по энергии
        COLORREF *cluster_classes =
                copyclip_ ? cluster_classes_copyclip
                          : cluster_classes_normal;
        unsigned cluster_class_count = SIZEOF(cluster_classes_copyclip);
        int energy_log = int(it.energy ? log10(it.energy) : 0);
        unsigned color_index = scale(energy_log, 2, 6,
                                     0u, cluster_class_count-1 );
        color_index = std::min<int>(cluster_class_count-1, color_index);
        int color = cluster_classes[color_index];

        CPen pen( CreatePen(PS_SOLID, 4, color) );
        CPenHandle open=dc.SelectPen( pen );

        dc.MoveTo(xs[xs.size()-1], ys[ys.size()-1]);
        for(int index=0; index < xs.size(); ++index)
            dc.LineTo(xs[index], ys[index]);

        dc.SelectPen(open);

        // определяем размеры для tooltip
        double l, r, b, t; it.get_rect(l, t, r, b);
        CRect rect(
                    xrange_.map_to(l, frame_.Width()) + frame_.left,
                    yrange_.map_to(t, -frame_.Height()) + frame_.bottom,
                    xrange_.map_to(r, frame_.Width()) + frame_.left,
                    yrange_.map_to(b, -frame_.Height()) + frame_.bottom
               );

        int energy = (int)it.energy;
        int cclass = (int)log10(energy) - 1;
        int count = it.count;

        std::string txtclass =
                cclass == 1 ? _ls("green") :
                cclass == 2 ? _ls("blue") :
                cclass == 3 ? _ls("yellow") :
                cclass == 4 ? _ls("orange") :
                cclass == 5 ? _ls("red") :
                _ls("extra");

        std::string text = strlib::strf(
                    (_ls("Cluster class:") + " %d (%s)\n" +
                     _ls("Energy:") + " %d\n" +
                     _ls("Count:") + " %d").c_str(),
                cclass, txtclass,
                energy, count);

        AddToolTipRegion(rect, text);
    }
}

static void MyFillRect(CDCHandle dc, CRect rc)
{
    rc.NormalizeRect();
    for (int x=rc.left; x<rc.right; ++x)
        for (int y=rc.top; y<rc.bottom; ++y)
            if ( (x+y) % 8 ==0)
                dc.SetPixel(x, y, clrSRHatch);
}

void CPlotView::DrawContents(CDCHandle dc, CRect frame)
{
    // prepare tool tips
    RemoveAllToolTipRegions();

    if (!copyclip_)
    {
        model_.set_visibility(true);
    }

    //////////////////////////////////////////////////////////////////////////
    // draw data

    shared_ptr<BitmapMaker> clipcopy_maker;
    if (copyclip_)
    {
        clipcopy_maker.reset(new BitmapMaker(&model_, true));
        clipcopy_maker->set_visibility(true);
        clipcopy_maker->set_dimensions(frame.Size());
    }

    shared_ptr<CBitmap> hb = model_.get_bitmap();
    if (clipcopy_maker)
    {
        hb = clipcopy_maker->get_result()->bitmap_;
    }

    //////////////////////////////////////////////////////////////////////////
    // вывод рамки выделения
    if (!copyclip_ && sel_rect_.Height()*sel_rect_.Width())
    {
        CRect sel_edge=sel_rect_;
        sel_edge.NormalizeRect();
        sel_edge.IntersectRect(sel_edge, frame);
        dc.DrawEdge(sel_edge, EDGE_ETCHED, BF_RECT);
    }

    // задаем прямоугольник для рисования
    int state = dc.SaveDC();
    dc.IntersectClipRect(frame);

    //////////////////////////////////////////////////////////////////////////
    // вывод вспомогательных вещей:
    // - датчики
    // - тайммаки
    // - кластеры
    // - стадии

    location::Setup sp;
    if (model_.get_location_setup(sp))
    {
        if (location::LinearSetup *setup=sp.linear())
            DrawSensors(dc, frame, *setup);
        if (location::PlanarSetup *setup=sp.planar())
        {
            DrawSensors(dc, frame, *setup);
            DrawClusters(dc, frame);
        }
        if (location::VesselSetup* setup=sp.vessel())
        {
            DrawSensors(dc, frame, *setup);
            DrawClusters(dc, frame);
        }
    }

    DrawTimemarks(dc, frame);
    DrawStages(dc, frame);

    //////////////////////////////////////////////////////////////////////////
    // вывод графика

    // источник изображения
    CDC dc_bitmap( ::CreateCompatibleDC(dc) );
    dc_bitmap.SelectBitmap( *hb );

    // устанавливаем цвета
    COLORREF transparent;
    if (copyclip_)
    {
        transparent = 0xFFFFFF;
        dc.SetBkColor(0xFFFFFF);
        dc.SetTextColor(0x0);
    }
    else
    {
        dc.SetBkColor(0x0);
        dc.SetTextColor(0xFFFFFF);
        transparent = 0;
    }


    // копируем битмапы
    dc.TransparentBlt(frame.left, frame.top,
                      frame.Width(), frame.Height(),
                      dc_bitmap, 0, 0, frame.Width(), frame.Height(),
                      transparent);

    dc.RestoreDC(state);

}

void CPlotView::DrawStages(CDCHandle dc, CRect frame)
{
    CRect outframe=MakeOutframe(frame);

    // draw plot::range selectors
    if (model_.get_select_range_mode())
    {
        // невыделенная стадия
        CPen penh( ::CreatePen(PS_SOLID, 2,  clrSRBorderIH) );
        CPen penl( ::CreatePen(PS_SOLID, 1,  clrSRBorderIL) );

        // активная стадия
        CPen penah( ::CreatePen(PS_SOLID, 2, clrSRBorderAH) );
        CPen pena( ::CreatePen(PS_SOLID, 1,  clrSRBorderAL) );

        CPenHandle oldp=dc.SelectPen(penh);

        for (unsigned index=0; index<stages_.size(); ++index)
        {
            long range_start=map_to(0, stages_[index].self->start);
            long range_end=map_to(0, stages_[index].self->end);

            MyFillRect(dc, CRect(
                           std::min(std::max(range_start, outframe.left), outframe.right),
                           outframe.top+1,
                           std::min(std::max(range_end, outframe.left), outframe.right),
                           outframe.bottom));

            dc.SelectPen(index==current_stage_
                         ? GetRangeValue(range_start).first ? penah: pena
             : GetRangeValue(range_start).first ? penh : penl );

            dc.MoveTo(range_start, outframe.top+1);
            dc.LineTo(range_start, outframe.bottom-1);

            dc.SelectPen( index==current_stage_
                          ? GetRangeValue(range_end).first ? penah: pena
              : GetRangeValue(range_end).first ? penh : penl);

            dc.MoveTo(range_end, outframe.top+1);
            dc.LineTo(range_end, outframe.bottom-1);
        }

        dc.SelectPen( oldp );
    }
}

CRect CPlotView::get_sel_rect() const
{
    return sel_rect_;
}

void CPlotView::set_sel_rect(CRect r)
{
    r.NormalizeRect();
    r.left=std::min(std::max(r.left, frame_.left), frame_.right);
    r.right=std::min(std::max(r.right, frame_.left), frame_.right);
    r.top=std::min(std::max(r.top, frame_.top), frame_.bottom);
    r.bottom=std::min(std::max(r.bottom, frame_.top), frame_.bottom);
    sel_rect_=r;
}

std::string CPlotView::yaxe_mark(int y)
{
    CRect frame=get_frame();
    return model_.get_yrange().unmap( frame.bottom-y, frame.Height() );
}

std::string CPlotView::xaxe_mark(int x)
{
    CRect frame=get_frame();
    return xrange_.unmap( x-frame.left, frame.Width() );
}

LRESULT CPlotView::OnPlotCommand(UINT, int id, HWND)
{
    switch (id)
    {
    case IDC_PLOT_ZOOMIN:
    {
        PlotManager::instance()->SetZoomMode(
            !PlotManager::instance()->IsZoomMode());
    }
    break;
    case IDC_PLOT_PAN:
    {
        PlotManager::instance()->SetPanMode(
            !PlotManager::instance()->IsPanMode());
    }
    break;
    case IDC_PLOT_ZOOMOUT:
    {
        PlotManager::instance()->ZoomBack();
    }
    break;
    case IDC_PLOT_ZOOMALL:
    {
        PlotManager::instance()->ZoomAll(this);
    }
    break;
    case IDC_PLOT_COPYCLIP:
        ShowCopyClipDlg(m_hWnd);
        break;
    case IDC_TILE_COPYCLIP:
        ShowCopyClipDlg(GetParent());
        break;
    }

    return 0;
}

LRESULT CPlotView::OnSetCursor(HWND, UINT ht, UINT mm)
{
    if (mm==0)
        return 0;

    CRect rc; GetClientRect(rc);
    CPoint cursor; GetCursorPos(&cursor);
    ScreenToClient(&cursor);

    if (!rc.PtInRect(cursor))
    {
        GetCursorPos(&cursor);
        HWND hwnd=WindowFromPoint(cursor);
        SetMsgHandled(false);
        return ::SendMessage(hwnd, WM_SETCURSOR, (WPARAM)hwnd, 0);
    }

    if (model_.get_select_range_mode())
    {
        if (CursorAtRangeBorder(cursor))
        {
            ::SetCursor(::LoadCursor(NULL, IDC_SIZEWE));
        }
        else
        {
            ::SetCursor(::LoadCursor(NULL, IDC_ARROW));
            SetMsgHandled(false);
        }
    }
    else if (PlotManager::instance()->IsPanMode() || panmode_ )
    {

        ::SetCursor(::LoadCursor( _Module.GetResourceInstance(),
                                  panmode_
                                  ? MAKEINTRESOURCE(IDC_HAND2)
                                  : MAKEINTRESOURCE(IDC_HAND1))
                   );

    }
    else if (PlotManager::instance()->IsZoomMode() || is_zoom_mode())
    {
        ::SetCursor(::LoadCursor( _Module.GetResourceInstance(),
                                  MAKEINTRESOURCE(IDC_ZOOMIN)));
    }
    else if (is_addition_mode())
    {
        ::SetCursor(::LoadCursor( _Module.GetResourceInstance(),
                                  MAKEINTRESOURCE(IDC_SELECT_1)));
    }
    else if (is_substraction_mode())
    {
        ::SetCursor(::LoadCursor( _Module.GetResourceInstance(),
                                  MAKEINTRESOURCE(IDC_SELECT_2)));
    }
    else
    {
        ::SetCursor(::LoadCursor( _Module.GetResourceInstance(),
                                  MAKEINTRESOURCE(IDC_SELECT_0)));
    }
    return 0;
}

LRESULT CPlotView::OnKeyDown(TCHAR w, UINT l, UINT h)
{
    if (model_.get_select_range_mode() && w==VK_DELETE)
    {
        if (current_stage_<stages_.size())
        {
            stages_.erase(stages_.begin()+current_stage_);
            current_stage_=std::min<size_t>(current_stage_, stages_.size()-1);
            ::PostMessage(GetParent(), UM_INVALIDATEFRAME, 0, 0);
        }
    }

    if (IsInDragMode() && w==VK_ESCAPE)
    {

    }

    OnSetCursor();
    SetMsgHandled(false);
    return TRUE;
}

LRESULT CPlotView::OnKeyUp(TCHAR w, UINT, UINT)
{
    static std::string buffer;
    buffer+=std::string(1, w);
    if (buffer.size() > 10)
    {
        buffer.erase(0, 1);
    }

    if (buffer.find("KILLME")==0)
    {
        /*
            testing crash_report facility
        */

        int *p=0;
        *p=0;
    } else if (buffer.find("IDDQD")==0)
    {
        /*
            enabling debug console
         */

        debug::log_enable_console();
    }

    //////////////////////////////////////////////////////////////////////////

    OnSetCursor();
    SetMsgHandled(false);
    return TRUE;
}

LRESULT CPlotView::OnChar(TCHAR w, UINT l, UINT h)
{
    if (w=VK_ESCAPE)
    {
        return ::SendMessage(GetParent(), WM_CHAR, w, MAKELPARAM(l, h));
    }

    if (model_.get_select_range_mode())
    {
        ::SendMessage(GetParent(), WM_NEXTDLGCTL, 0, 0);
        ::SendMessage(GetFocus(),  WM_CHAR, w, MAKELPARAM(l,h));
    }
    return 0;
}

LRESULT CPlotView::OnMButtonDown(UINT, CPoint start)
{
    if (!read_only_ && !select_rect_mode_ && !model_.get_select_range_mode())
    {
        StartPan(start);
    }
    return 0;
}

LRESULT CPlotView::OnMButtonUp(UINT, CPoint start)
{
    EndPan(start);
    return 0;
}

LRESULT CPlotView::OnMouseWheel(UINT a, short z, CPoint pt)
{
    HWND other=WindowFromPoint(pt);
    if (other!=m_hWnd)
    {
        return ::SendMessage(other, WM_MOUSEWHEEL, MAKEWPARAM(a, z), MAKELPARAM(pt.x, pt.y));
    }

    if (!read_only_ && !select_rect_mode_)
    {
        if (!model_.get_select_range_mode())
        {
            axe_info inf=model_.get_xaxe_info();
            double delta=(inf.hi-inf.lo);
            delta *= (- z / WHEEL_DELTA) * 0.25;

            inf.lo-=delta;
            inf.hi+=delta;
            model_.set_xminmax(false, inf.lo, inf.hi);

            bool yScaling=                    
                    !(model_.get_xaxe_info().chartype == aera::C_Time
                      && model_.get_yaxe_info().auto_scale);

            if (yScaling)
            {
                axe_info inf=model_.get_yaxe_info();
                double delta=(inf.hi - inf.lo);
                delta *= (- z / WHEEL_DELTA) * 0.25;

                inf.lo-=delta;
                inf.hi+=delta;
                model_.set_yminmax(false, inf.lo, inf.hi);
            }

            model_.restart();
        }
    }
    return 0;
}

void CPlotView::on_change_axes()
{
    PlotManager::instance()->OnZoom(this);
}

unsigned CPlotView::GetSelectionRange(std::vector<shared_ptr<stage> > &tmp) const
{
    tmp.clear();
    foreach(plot_stage const& s, stages_) tmp.push_back( s.self );

    return current_stage_;
}

void CPlotView::SetSelectionRange(const std::vector<shared_ptr<stage> > &tmp, unsigned i)
{
    stages_.clear();
    foreach(shared_ptr<stage> const& s, tmp)
    {
        plot_stage ps;
        ps.num = 0;
        ps.self = s;
        stages_.push_back(ps);
    }

    current_stage_=i;
    Invalidate();
}

std::pair<bool, double> CPlotView::GetRangeValue(int value) const
{
    value=std::min(std::max((long)value, frame_.left), frame_.right);
    double result=unmap_double(0, value);
    std::vector<double> tmks = model_.get_timemarks();
    tmks.push_back(0.0);
    for (unsigned index=0; index<tmks.size(); ++index)
    {
        int x=map_to(0, tmks[index]);
        if (abs(x-value)<=1)
        {
            return std::make_pair(true, tmks[index]);
        }
    }
    return std::make_pair(false, result);
}

void CPlotView::serialization(serl::archiver &arc)
{
    arc.serial(model_);
    if (arc.is_loading())
    {
        xrange_=model_.get_xrange();
        yrange_=model_.get_yrange();
    }
}

axe_info CPlotView::get_xaxe_info() const
{
    return xaxe_;
}

void CPlotView::set_xaxe_info(const axe_info &info)
{
    model_.set_xaxe_info(info);
    xaxe_ = info;
}

axe_info CPlotView::get_yaxe_info() const
{
    return yaxe_;
}

void CPlotView::set_yaxe_info(const axe_info &info)
{
    model_.set_yaxe_info(info);
    yaxe_ = info;
}

plot_info CPlotView::get_plot_params() const
{
    return model_.get_plot_info();
}

void CPlotView::set_plot_params(const plot_info &info)
{
    model_.set_plot_info(info);
}

CTVCreator *CPlotView::MakeClassCreator()
{
    return new CTVCreatorImpl<CPlotView>(this);
}

LRESULT CPlotView::OnWindowPosChanged(UINT, LPARAM, WPARAM)
{
    SetMsgHandled(false);
    bool vsbl2=::IsWindowVisible(m_hWnd);
    model_.set_visibility( vsbl2 );
    return 0;
}

void CPlotView::set_visibility(bool b)
{
    model_.set_visibility(b);
}

void CPlotView::set_parent(nodes::pnode p)
{
    model_.set_parent(p);
}

void CPlotView::safe_on_restart()
{
    Invalidate();
}

void CPlotView::safe_on_finish(process::prslt)
{
    xrange_=model_.get_xrange();
    yrange_=model_.get_yrange();

    xaxe_ = model_.get_xaxe_info();
    yaxe_ = model_.get_yaxe_info();

    Invalidate();
}


LRESULT CPlotView::OnGetDlgCode(LPMSG)
{
    return DLGC_STATIC;
}


