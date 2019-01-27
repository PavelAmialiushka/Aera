#pragma once

#include <boost/tuple/tuple.hpp>
#include <atlctrls.h>

#include "data/data_fwrd.h"

#include "data/processes.h"

#include "BitmapMaker.h"
#include "PlotBaseWnd.h"
#include "PlotTypes.h"

//////////////////////////////////////////////////////////////////////////

#include "PlotModel.h"

#include "ToolTipWindow.h"

using namespace WTL;

class stage;
class plot_maker;
class BitmapMaker;

namespace plot
{
class darray;
}

namespace location
{
class LinearSetup;
class PlanarSetup;
}

struct plot_stage
{
    int num;
    shared_ptr<stage> self;
};

//////////////////////////////////////////////////////////////////////////

class CPlotView
    : public CPlotWndBase
    , public CToolTipWindow<CPlotView>
    , process::host_listener
{
    typedef CPlotWndBase baseClass;

    enum
    {
        UM_INVALIDATEFRAME =WM_USER+1,
    };

    BEGIN_MSG_MAP_EX(CPlotView)
        CHAIN_MSG_MAP(CToolTipWindow<CPlotView>);

        MSG_WM_CREATE(OnCreate);
        MSG_WM_DESTROY(OnDestroy);

        MESSAGE_HANDLER_EX(WM_SHOWWINDOW, OnWindowPosChanged);
        MESSAGE_HANDLER_EX(WM_WINDOWPOSCHANGED, OnWindowPosChanged);

        MSG_WM_SETFOCUS(OnFocus);
        MSG_WM_KILLFOCUS(OnFocus);
        MSG_WM_SETCURSOR(OnSetCursor);

        MSG_WM_GETDLGCODE(OnGetDlgCode);

        MSG_WM_LBUTTONDOWN(OnLButtonDown);
        MSG_WM_LBUTTONUP(OnLButtonUp);
        MSG_WM_LBUTTONDBLCLK(OnLButtonDbl);

        MSG_WM_RBUTTONDOWN(OnRButtonDown);

        MSG_WM_MBUTTONDOWN(OnMButtonDown);
        MSG_WM_MBUTTONUP(OnMButtonUp);
        MSG_WM_MOUSEWHEEL(OnMouseWheel);

        MSG_WM_KEYDOWN(OnKeyDown);
        MSG_WM_KEYUP(OnKeyUp);

        MSG_WM_CHAR(OnChar);

        MSG_WM_MOUSEMOVE(OnMouseMove);
        MSG_WM_MOUSELEAVE(OnMouseLeave);

        MSG_WM_COMMAND(OnPlotCommand);

        CHAIN_MSG_MAP(baseClass);
    END_MSG_MAP()

public:

    CPlotView(bool=false, bool ro=false);
    ~CPlotView();

    axe_info get_xaxe_info() const;
    void set_xaxe_info(const axe_info &);

    axe_info get_yaxe_info() const;
    void set_yaxe_info(const axe_info &);

    plot_info get_plot_params() const;
    void set_plot_params(const plot_info &);

    void set_visibility(bool);

    void set_parent(nodes::pnode p);

    void safe_on_restart();
    void safe_on_finish(process::prslt);

private:
    // controller

    LRESULT OnCreate(LPCREATESTRUCT);
    LRESULT OnDestroy();

    LRESULT OnFocus(HWND);
    LRESULT OnSetCursor(HWND=0, UINT=0, UINT=0);

    LRESULT OnWindowPosChanged(UINT, LPARAM, WPARAM);

    LRESULT OnLButtonDown(UINT, CPoint);
    LRESULT OnLButtonUp(UINT, CPoint);
    LRESULT OnLButtonDbl(UINT, CPoint);

    LRESULT OnRButtonDown(UINT, CPoint);

    LRESULT OnMButtonDown(UINT, CPoint);
    LRESULT OnMButtonUp(UINT, CPoint);

    LRESULT OnMouseWheel(UINT, short, CPoint);

    LRESULT OnMouseMove(UINT, CPoint);
    LRESULT OnMouseLeave();

    LRESULT OnKeyDown(TCHAR, UINT, UINT);
    LRESULT OnKeyUp(TCHAR, UINT, UINT);
    LRESULT OnChar(TCHAR, UINT, UINT);

    LRESULT OnGetDlgCode(LPMSG);

    LRESULT OnPlotCommand(UINT, int, HWND);

    void    OnFinalMessage(HWND);

public:

    void    DoInvalidate();

    virtual void on_change_axes();

    bool get_axedrag() const;
//    std::pair<int, int>  get_axedragvalues() const;

    // zooming
    void  ZoomXAxeFrame(int a, int b);
    void  ZoomYAxeFrame(int a, int b);
    void  ZoomAxesFrame(int l, int r, int t, int b);

    void  SelectRect(CRect sel);

    // view

    CRect get_sel_rect() const;
    void set_sel_rect(CRect r);

    std::string yaxe_mark(int y);
    std::string xaxe_mark(int x);

    unsigned GetSelectionRange(std::vector<shared_ptr<stage> > &tmp) const;
    void SetSelectionRange(const std::vector<shared_ptr<stage> > &tmp, unsigned);

    void serialization(serl::archiver &);

private:

    // CPlotWndBase
    virtual void OnSizeChanged();
    virtual void DrawContents(CDCHandle dc, CRect frame);
    CTVCreator  *MakeClassCreator();

private:
    void StartPan(CPoint start);
    void PanPlot(CPoint start, CPoint end);
    void PanPlotXAxe(CPoint start, CPoint end);
    void PanPlotYAxe(CPoint start, CPoint end);
    void EndPan(CPoint start);

    void StartSelectRect(CPoint pt);
    void DoSelectRect(CPoint pt);
    void EndSelectRect(CPoint pt);

    //  select range
    unsigned CursorAtRangeSelector(CPoint pt);
    bool     CursorAtRangeBorder(CPoint pt);
    std::pair<bool, double> GetRangeValue(int) const;

    void StartSelectRange(CPoint pt);
    void DoSelectRange(CPoint pt);
    void EndSelectRange(CPoint pt);

    void StartZoom(CPoint);
    void EndZoom(CPoint);

    void DrawTimemark(CDCHandle dc, CRect frame, double) const;
    void DrawTimemarks(CDCHandle dc, CRect frame);

    void DrawSensors(CDCHandle dc, CRect frame, location::LinearSetup &linear);
    void DrawSensors(CDCHandle dc, CRect frame, location::PlanarSetup &planar);
    void DrawSensors(CDCHandle dc, CRect frame, location::VesselSetup &vessel);

    void DrawClusters(CDCHandle dc, CRect frame);

    void DrawStages(CDCHandle dc, CRect frame);
    void SetCaption();

private:

    friend class PlotPool;

    plot_model   model_;

    // controller
    bool                select_rect_mode_;
    std::pair<int, int> axedragvalues_;
    CPoint              startdrag_;

    // panning
    bool        panmode_;
    CPoint      pan_point_;
    CPoint      pan_prev_point_;
    boost::tuples::tuple<axe_info, axe_info> pan_buffer_;

    //
    bool        read_only_;

    // range selector
    bool		selecting_range_;
    int			range_start_;
    int			range_end_;
    unsigned	current_stage_;

    std::vector<plot_stage> stages_;

    // view
    shared_ptr<plot::darray> bitmap_;


    CBitmap     hbitmap_;
    CRect       sel_rect_;
};


