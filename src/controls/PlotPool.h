#pragma once

#include "PlotMaker.h"

class CPlotView;

enum PlotStateMode { plotNormal, plotPan, plotZoom };

class plot_listener
{
public:
    virtual void select_channel(int ch) {}
};

class PlotManager : public utils::singleton<PlotManager>
{
public:
    PlotManager();

    void attach_maker(plot_maker *ptr);
    void detach_maker(plot_maker *ptr);

    void attach_view(CPlotView *view);
    void detach_view(CPlotView *view);

    // active channels

    void attach_listener(plot_listener* lst);
    void detach_listener(plot_listener* lst);

    void SetActiveChannel(int);
    int GetActiveChannel() const;

    bool CanZoomBack();
    void ZoomBack();


    struct setter_xminmax
    {
        bool a; double z; double x;
    public:
        setter_xminmax(bool a, double z, double x) : a(a) , z(z), x(x) {}
        void operator()(plot_maker *ptr);
    };

    void set_xminmax(bool a, double z, double x);
    axe_info get_current_axe_info();


    //
    struct data_t
    {
        CPlotView  *view;
        HWND        hwnd;
        axe_info    xaxe;
        axe_info    yaxe;

        data_t(CPlotView *view);

        HWND  get() const
        {
            return hwnd;
        }

        typedef HWND (data_t::*bool_t)() const;

        operator bool_t() const
        {
            return get() ? &data_t::get : NULL;
        }
        bool operator!() const
        {
            return !get();
        }

        bool operator==(CPlotView *v) const
        {
            return view==v;
        }
    };

    void Apply(data_t data);

    // вызывается из CPlotWindow

    void Zoom(CPlotView *self, double l, double r, double t, double b, bool store=true);

    void ZoomAll(CPlotView *view);

    bool CanZoomAll(CPlotView *view);

    void SetZoomMode(bool z=true);

    void SetPanMode(bool mode=true);

    void SetWideOperation(bool f=true);

    bool GetWideOperation() const;

    bool IsPanMode() const;

    // удаляем все записи по данному виду
    // (если, например вид был уничтожен)
    void ZoomClear(CPlotView *view);

    // сообщаем о начале смещения
    void OnZoom(CPlotView *view);

    bool IsZoomMode() const;

private:
    data_t back();

private:

    std::vector<plot_listener*> listeners_;

    // plot history
    std::vector< data_t > zoom_undo_stack_;
    std::vector< CPlotView* > views_;
    bool  zooming_;
    bool  panning_;
    bool  locked_;
    bool  wide_operation_;

    // showchannel
    int active_channel_;

    // plotpool
    bool cura_;
    double curleft_;
    double curright_;
    std::vector<plot_maker *> plots_;
    bool                      lock_changes_;
};

//////////////////////////////////////////////////////////////////////////
