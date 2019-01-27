#include "stdafx.h"

#include "PlotPool.h"

#include "PlotWindow.h"

#include "utilites/document.h"
#include "utilites/foreach.hpp"

PlotManager::PlotManager()
    : lock_changes_(false)
    , cura_(true)
    , curleft_(0)
    , curright_(1)
    , zooming_(false)
    , panning_(false)
    , locked_(false)
    , wide_operation_(false)
{}

void PlotManager::attach_maker(plot_maker *ptr)
{
    plots_.push_back(ptr);
}


void PlotManager::detach_maker(plot_maker *ptr)
{
    plots_.erase(std::remove(STL_II(plots_), ptr), plots_.end());
}

void PlotManager::SetActiveChannel(int ch)
{
    active_channel_=ch;

    foreach(plot_listener* lst, listeners_)
    {
        lst->select_channel(ch);
    }
}

int PlotManager::GetActiveChannel() const
{
    return active_channel_;
}

void PlotManager::set_xminmax(bool a, double z, double x)
{
    if (!lock_changes_)
    {
        lock_changes_=true;

        cura_=a;
        curleft_=z;
        curright_=x;

        foreach(plot_maker* maker, plots_)
        {
            bool loc_a; double loc_x, loc_z;
            maker->get_xminmax(loc_a, loc_z, loc_x);

            if (maker->get_xaxe_info().chartype==aera::C_Time && (loc_a!=a || loc_z!=z || loc_x!=x))
            {
                maker->set_xminmax(a, z, x);
                maker->restart();
            }

        }
        lock_changes_=false;
    }
}


axe_info PlotManager::get_current_axe_info()
{
    axe_info tmp= {aera::C_Time, cura_, curleft_, curright_};
    return tmp;
}

void PlotManager::Apply(PlotManager::data_t data)
{
    locked_=true;

    data.view->set_xaxe_info(data.xaxe);
    data.view->set_yaxe_info(data.yaxe);

    Document().SetModified(true);

    locked_=false;
}

void PlotManager::ZoomBack()
{
    if (data_t command=back())
    {
        Apply(command);
        if (zoom_undo_stack_.size())
            zoom_undo_stack_.pop_back();

        SetPanMode(false);
        SetZoomMode(false);
    }
}

void PlotManager::SetZoomMode(bool z)
{
    if (IsPanMode())
        SetPanMode(false);

    zooming_=z;
}

void PlotManager::SetPanMode(bool mode)
{
    if (IsZoomMode()) SetZoomMode(false);

    panning_=mode;
}

void PlotManager::SetWideOperation(bool f)
{
    wide_operation_=f;
}

bool PlotManager::GetWideOperation() const
{
    return wide_operation_;
}

bool PlotManager::IsPanMode() const
{
    return panning_;
}

bool PlotManager::IsZoomMode() const
{
    return zooming_;
}

bool PlotManager::CanZoomBack()
{
    return back();
}

PlotManager::data_t PlotManager::back()
{
    if (zoom_undo_stack_.size())
        return zoom_undo_stack_.back();
    else
        return data_t(0);
}

void PlotManager::attach_view(CPlotView *view)
{
    views_.push_back( view );
}

void PlotManager::detach_view(CPlotView *view)
{
    views_.erase(
                std::remove(STL_II(views_), view),
                views_.end());
}

void PlotManager::attach_listener(plot_listener *lst)
{
    listeners_.push_back( lst );
}

void PlotManager::detach_listener(plot_listener *lst)
{
    listeners_.erase(
                std::remove(STL_II(listeners_), lst),
                listeners_.end());
}


void PlotManager::OnZoom(CPlotView *view)
{
    data_t curr=view;
    if (data_t old=back())
    {
        if (old.xaxe.chartype!=curr.xaxe.chartype ||
                old.yaxe.chartype!=curr.yaxe.chartype)
        {
            ZoomClear(view);
            return;
        }
    }
    if (!locked_)
    {
        if (!wide_operation_ || !back())
        {
            zoom_undo_stack_.push_back( data_t(view) );
        }
    }
}

void PlotManager::ZoomClear(CPlotView *view)
{
    zoom_undo_stack_.erase(
                std::remove(STL_II(zoom_undo_stack_), view),
                zoom_undo_stack_.end()
                );
}

bool PlotManager::CanZoomAll(CPlotView *view)
{
    assert(view);
    return !view->get_xaxe_info().auto_scale || !view->get_yaxe_info().auto_scale;
}

void PlotManager::ZoomAll(CPlotView *view)
{
    assert(view);
    if (CanZoomAll(view))
    {
        ZoomClear(view);
        locked_=true;
        axe_info info=view->get_xaxe_info();
        info.auto_scale=true; view->set_xaxe_info(info);

        info=view->get_yaxe_info();
        info.auto_scale=true; view->set_yaxe_info(info);

        Document().SetModified(true);
        locked_=false;
    }
    SetPanMode(false);
    SetZoomMode(false);
}

void PlotManager::Zoom(CPlotView *self, double l, double r, double t, double b, bool store)
{
    locked_=true;
    data_t curr=self;

    if (store)
    {
        zoom_undo_stack_.push_back(curr);
    }

    // дурацкая система выдает логарифмическую энергию равную 0.
    // мы же будем считать, что в этом случае минимальная энергия равна
    // 1,0
    l=curr.xaxe.log_scale && curr.xaxe.lo==0
            ? std::max(1.0, l)
            : std::max(curr.xaxe.lo, l);
    r=std::min(curr.xaxe.hi, r);
    if (l!=r && !(curr.xaxe.auto_scale && l == curr.xaxe.lo && curr.xaxe.hi == r))
    {
        axe_info info=self->get_xaxe_info();
        info.auto_scale=false;
        info.lo=l;
        info.hi=r;
        self->set_xaxe_info(info);
    }

    b=curr.yaxe.log_scale && curr.yaxe.lo==0
            ? std::max(1.0, b)
            : std::max(curr.yaxe.lo, b);
    t=std::min(curr.yaxe.hi, t);
    if (t!=b && !(curr.yaxe.auto_scale && b == curr.yaxe.lo && curr.yaxe.hi == t))
    {
        axe_info info=self->get_yaxe_info();
        info.auto_scale=false;
        info.lo=b;
        info.hi=t;
        self->set_yaxe_info(info);
    }

    Document().SetModified(true);

    SetZoomMode(false);
    locked_=false;
}


PlotManager::data_t::data_t(CPlotView *view)
    : hwnd(0), view(view)
{
    if (view)
    {
        hwnd=view->m_hWnd;
        xaxe=view->get_xaxe_info();
        yaxe=view->get_yaxe_info();
    }
}
