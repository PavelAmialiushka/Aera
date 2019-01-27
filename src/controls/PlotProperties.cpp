#include "stdafx.h"

#include "controls\PlotProperties.h"

#include "utilites/localizator.h"
#include "utilites/document.h"
#include "utilites/foreach.hpp"
#include "utilites/help.h"

#include "data/nodeFactory.h"
#include "data/collection.h"
#include "data/node.h"

#include "data/LocationSetup.h"

CPlotPropertyDlg::CPlotPropertyDlg(plot_model *model, HWND parent)
    : model_(model)
    , wndParent(parent)
    , ignore_(1)
    , invalid_(0)
{
    model_->attach_listener(this);
}

CPlotPropertyDlg::~CPlotPropertyDlg()
{
    model_->detach_listener(this);
}

LRESULT CPlotPropertyDlg::OnInitDialog(HWND hWnd, LPARAM ppage)
{
    _lw(m_hWnd, IDD_PLOTPPCOMMON);
    CenterWindow();

    xbox_.Attach( GetDlgItem(IDC_PLOT_XAXE) );
    ybox_.Attach( GetDlgItem(IDC_PLOT_YAXE) );
    xauto_.Attach( GetDlgItem(IDC_PLOT_XAXE_AUTO) );
    yauto_.Attach( GetDlgItem(IDC_PLOT_YAXE_AUTO) );
    xlog_.Attach( GetDlgItem(IDC_PLOT_XAXE_LOG) );
    ylog_.Attach( GetDlgItem(IDC_PLOT_YAXE_LOG) );
    xmin_.Attach( GetDlgItem(IDC_PLOT_XAXE_MIN) );
    xmax_.Attach( GetDlgItem(IDC_PLOT_XAXE_MAX) );
    ymin_.Attach( GetDlgItem(IDC_PLOT_YAXE_MIN) );
    ymax_.Attach( GetDlgItem(IDC_PLOT_YAXE_MAX) );
    use_location_.Attach( GetDlgItem(IDC_PP_USE_LOCATION) );
    use_clusters_.Attach( GetDlgItem(IDC_PP_USE_CLUSTERS) );
    location_type_.Attach( GetDlgItem(IDC_PP_LOCATION) );

    linestyle_.Attach( GetDlgItem(IDC_LINESTYLE) );
    binnumber_.Attach( GetDlgItem(IDC_LINEBINS) );
    tddata_.Attach( GetDlgItem( IDC_SHOW_TDD ) );
    bychannel_.Attach( GetDlgItem( IDC_COLORBYCHANNEL) );

    axe_info info = model_->get_xaxe_info();
    // X-axe
    model_->get_xminmax(info.auto_scale, info.lo, info.hi);
    xmin_.SetWindowText((strlib::strf("%g",info.lo)).c_str());
    xmax_.SetWindowText((strlib::strf("%g",info.hi)).c_str());

    xauto_.SetCheck(info.auto_scale);
    xlog_.SetCheck(info.log_scale);

    // Y-axe
    info = model_->get_yaxe_info();
    model_->get_yminmax(info.auto_scale, info.lo, info.hi);

    yauto_.SetCheck(info.auto_scale);
    ymin_.SetWindowText((strlib::strf("%g", info.lo)).c_str());
    ymax_.SetWindowText((strlib::strf("%g", info.hi)).c_str());
    ylog_.SetCheck(info.log_scale);
    OnYAutoClick(0, 0, 0);
    OnXAutoClick(0, 0, 0);

    plottype_t line;
    int bins;
    bool color;
    model_->get_plot_params(&bins, &color, &line);
    plot_info pinfo=model_->get_plot_info();

    linestyle_.SetItemData(linestyle_.AddString(_lcs("scatter")), scatter);
    linestyle_.SetItemData(linestyle_.AddString(_lcs("history")), history);
    linestyle_.SetItemData(linestyle_.AddString(_lcs("min-max")), minmax);
    linestyle_.SetItemData(linestyle_.AddString(_lcs("max-min")), maxmin);
    linestyle_.SetItemData(linestyle_.AddString(_lcs("distribution#distribution of max")), distribution);
    linestyle_.SetItemData(linestyle_.AddString(_lcs("summation#distribution of sum")), summation);
    linestyle_.SetCurSel(line);

    binnumber_.SetItemData(binnumber_.AddString("10"), 10);
    binnumber_.SetItemData(binnumber_.AddString("20"), 20);
    binnumber_.SetItemData(binnumber_.AddString("50"), 50);
    binnumber_.SetItemData(binnumber_.AddString("100"), 100);
    binnumber_.SetItemData(binnumber_.AddString("200"), 200);
    binnumber_.SetCurSel(binnumber_.FindStringExact(-1, (strlib::strf("%d", bins).c_str())));

    // set location types
    std::vector< location::Setup > tmp=
        nodes::factory().get_location_setup();

    int current_index=-1;
    object_id cur_location = model_->get_location();
    for (unsigned index=0; index<tmp.size(); ++index)
    {
        location::Setup loc=tmp[index];
        location_type_.AddString( loc.name().c_str() );
        if (loc.get_id()==model_->get_location())
            current_index=index;
    }

    if (current_index!=-1)
    {
        location_type_.SetCurSel( current_index );
    }
    else if (!cur_location.empty())
    {
        location_type_.AddString(_lcs("PP-noloc#location not found"));
        location_type_.SetCurSel( tmp.size() );
    }
    else if (tmp.size() == 0)
    {
        location_type_.AddString(_lcs("PP-noloc-empty#empty"));
        location_type_.SetCurSel( tmp.size() );
    } else
    {
        location_type_.SetCurSel( 0 );
    }

    use_location_.SetCheck( model_->get_location()!=object_id() );
    use_clusters_.SetCheck( pinfo.use_clusters );

    tddata_.SetCheck( pinfo.tddata );
    bychannel_.SetCheck( color );

    ignore_=false;

    MakeFeatureList();
    OnAnyChange();

    return TRUE;
}

LRESULT CPlotPropertyDlg::OnClose(UINT, int id, HWND)
{
    Apply();
    EndDialog(id);
    return 0;
}

object_id CPlotPropertyDlg::getLocationByIndex(int selectionIndex)
{
    std::vector< location::Setup > tmp=nodes::factory().get_location_setup();
    object_id selected=selectionIndex >=0 && (unsigned)selectionIndex < tmp.size()
            ? tmp[selectionIndex].get_id()
            : object_id();

    return selected;
}

void CPlotPropertyDlg::Apply()
{
    bool need_update=false;
    axe_info info;
    std::vector<char> buffer;

    nodes::presult source = model_->get_parent()->get_result(false);

    info.chartype=xbox_.GetItemData(xbox_.GetCurSel());
    info.log_scale=xlog_.GetCheck();
    info.auto_scale=xauto_.GetCheck();

    if (info.auto_scale && source)
    {
        data::pslice slice = source->ae;
        data::pslice other_slice = source->tdd;
        if (GetActualTddCheck()) std::swap(slice, other_slice);

        data::lohi_value mmx = slice->get_minmax_limits((aera::chars)info.chartype);
        data::lohi_value other_mmx = other_slice->get_minmax_limits((aera::chars)info.chartype);

        widen_range_begin(info.lo, info.hi);
        widen_range(info.lo, info.hi, mmx.lo);
        widen_range(info.lo, info.hi, mmx.hi);
        widen_range(info.lo, info.hi, other_mmx.lo);
        widen_range(info.lo, info.hi, other_mmx.hi);

        xmin_.SetWindowText(strlib::strff(info.lo).c_str());
        xmax_.SetWindowText(strlib::strff(info.hi).c_str());
    } else
    {
        buffer.resize(xmax_.GetWindowTextLength()+1);
        xmax_.GetWindowText(&buffer[0], buffer.size());
        info.hi=atof(&buffer[0]);

        buffer.resize(xmin_.GetWindowTextLength()+1);
        xmin_.GetWindowText(&buffer[0], buffer.size());
        info.lo=atof(&buffer[0]);
    }

    if (model_->get_xaxe_info()!=info)
    {
        model_->set_xaxe_info(info);
        need_update=true;
    }

    info.chartype=ybox_.GetItemData(ybox_.GetCurSel());
    info.log_scale=ylog_.GetCheck();
    info.auto_scale=yauto_.GetCheck();

    if (info.auto_scale && source)
    {
        data::pslice slice = source->ae;
        data::pslice other_slice = source->tdd;
        if (GetActualTddCheck()) std::swap(slice, other_slice);

        data::lohi_value mmx
                = slice->get_minmax_limits((aera::chars)info.chartype);
        data::lohi_value other_mmx
                = other_slice->get_minmax_limits((aera::chars)info.chartype);

        widen_range_begin(info.lo, info.hi);
        widen_range(info.lo, info.hi, mmx.lo);
        widen_range(info.lo, info.hi, mmx.hi);
        widen_range(info.lo, info.hi, other_mmx.lo);
        widen_range(info.lo, info.hi, other_mmx.hi);

        ymin_.SetWindowText(strlib::strff(info.lo).c_str());
        ymax_.SetWindowText(strlib::strff(info.hi).c_str());
    } else
    {
        buffer.resize(ymin_.GetWindowTextLength()+1);
        ymin_.GetWindowText(&buffer[0], buffer.size());
        info.lo=atof(&buffer[0]);
        buffer.resize(ymax_.GetWindowTextLength()+1);
        ymax_.GetWindowText(&buffer[0], buffer.size());
        info.hi=atof(&buffer[0]);
    }

    if (model_->get_yaxe_info()!=info)
    {
        model_->set_yaxe_info(info);
        need_update=true;
    }

    int selectionIndex=use_location_.GetCheck() ? location_type_.GetCurSel() : -1;
    object_id selected = getLocationByIndex(selectionIndex);
    bool is_planar_or_vessel =
            nodes::factory().get_location(selected).planar()
            || nodes::factory().get_location(selected).vessel();


    if (model_->get_location()!=selected)
    {
        model_->set_location(selected);
        need_update=true;
    }

    plot_info pinfo=
    {
        static_cast<plottype_t>(linestyle_.GetItemData(linestyle_.GetCurSel())),
        // если локация, то всегда один цвет
        !use_location_.GetCheck() ? bychannel_.GetCheck() : false,

        // если локация, то всегда без ТДД
        !use_location_.GetCheck() ? tddata_.GetCheck() : false,

        binnumber_.GetItemData(binnumber_.GetCurSel()),

        is_planar_or_vessel ? use_clusters_.GetCheck() : false,
    };

    if (model_->get_plot_info()!=pinfo)
    {
        model_->set_plot_info(pinfo);
        need_update=true;
    }

    if (need_update)
    {
        model_->restart();
        wndParent.Invalidate();
        Document().SetModified();
    }
}

void CPlotPropertyDlg::OnXAutoClick(UINT, int, HWND)
{
    int style=xmin_.GetStyle();
    xmin_.SetReadOnly(xauto_.GetCheck());
    xmax_.SetReadOnly(xauto_.GetCheck());
}

void CPlotPropertyDlg::OnYAutoClick(UINT, int, HWND)
{
    int style=ymin_.GetStyle();
    ymin_.SetReadOnly(yauto_.GetCheck());
    ymax_.SetReadOnly(yauto_.GetCheck());
}

LRESULT CPlotPropertyDlg::OnHelp(LPHELPINFO)
{
    //help(m_hWnd, "plot");
    return 0;
}

void CPlotPropertyDlg::safe_on_finish(process::prslt)
{
    Apply();
}


void CPlotPropertyDlg::DelayedApply()
{
    if (!ignore_)
    {
        SetTimer(1, 100);
    }
}


void CPlotPropertyDlg::OnAnyChange(...)
{
    if (linestyle_ && binnumber_)
    {
        binnumber_.EnableWindow(linestyle_.GetCurSel()!=0);
    }

    location_type_.EnableWindow( use_location_.GetCheck() );

    int selectionIndex=use_location_.GetCheck() ? location_type_.GetCurSel() : -1;
    object_id selected = getLocationByIndex(selectionIndex);
    bool is_planar_or_vessel =
               nodes::factory().get_location(selected).planar()
               || nodes::factory().get_location(selected).vessel();

    use_clusters_.EnableWindow( use_location_.GetCheck() && is_planar_or_vessel );
    bychannel_.EnableWindow( !use_location_.GetCheck() && tddata_.GetCheck()==0 );
    tddata_.EnableWindow( !use_location_.GetCheck() );

    if (!ignore_)
    {
        DelayedApply();
    }
}

void CPlotPropertyDlg::OnLocationChanged(...)
{
    Apply();
    MakeFeatureList();

    OnAnyChange();
}

LRESULT CPlotPropertyDlg::OnTimer(UINT timer)
{
    switch (timer)
    {
    case 1:
        Apply();
        KillTimer(1);
        break;

    case 2:
        MakeFeatureList();
        break;
    }

    return 0;
}

bool CPlotPropertyDlg::GetActualTddCheck()
{
    return tddata_.GetCheck() && use_location_.GetCheck()==0;
}

void CPlotPropertyDlg::MakeFeatureList()
{
    nodes::presult source = model_->get_parent()->get_result(false);
    if (!source && !invalid_)
    {
        if (!invalid_) SetTimer(2, 100);
        invalid_=true;
    } else
    {
        invalid_=false;
        KillTimer(2);
    }

    features_.clear();

    if ( GetActualTddCheck()==0 )
    {
        if  (source)
            features_=source->ae->get_chars();

        if (!features_.size())
        {
            features_.push_back(aera::C_Time);
            features_.push_back(aera::C_Channel);
            features_.push_back(aera::C_Amplitude);
            features_.push_back(aera::C_Energy);
            features_.push_back(aera::C_Duration);
            features_.push_back(aera::C_Risetime);
            features_.push_back(aera::C_ASL);
            features_.push_back(aera::C_Hits);
            features_.push_back(aera::C_CoordX);
            features_.push_back(aera::C_CoordY);
        }
    }
    else
    {
        if  (source)
            features_=source->tdd->get_chars();

        if (!features_.size())
        {
            features_.push_back(aera::C_Time);
            features_.push_back(aera::C_Parametric1);
        }
    }

    strings_.clear();
    for(unsigned index = 0; index < features_.size(); ++index)
    {
        std::string s = aera::traits::get_wide_name(features_[index]);
        if (!s.empty())
            strings_.push_back( s );
    }

    // загружаем

    xbox_.ResetContent();
    ybox_.ResetContent();
    unsigned index=0;
    for (std::vector<std::string>::iterator iter=strings_.begin(); iter!=strings_.end(); ++iter, ++index)
    {
        int pos;
        pos=xbox_.AddString(iter->c_str());
        xbox_.SetItemData(pos, features_[index]);

        pos=ybox_.AddString(iter->c_str());
        ybox_.SetItemData(pos, features_[index]);
    }

    // ищем и выделяем текущее значение
    axe_info info=model_->get_xaxe_info();
    if (std::count(STL_II(features_), info.chartype)==0)
    {
        std::string text=aera::traits::get_wide_name( static_cast<aera::chars>(info.chartype) );
        text += " ("+_ls("PP-notpresent#not present")+")";
        int pos=xbox_.AddString( text.c_str() );
        xbox_.SetItemData( pos, info.chartype );
    }
    int fx=xbox_.FindString(0, aera::traits::get_wide_name(static_cast<aera::chars>(info.chartype)).c_str());
    xbox_.SetCurSel(fx==-1 ? 0 : fx);

    info=model_->get_yaxe_info();
    if (std::count(STL_II(features_), info.chartype)==0)
    {
        std::string text=aera::traits::get_wide_name( static_cast<aera::chars>(info.chartype) );
        text += " ("+_ls("PP-notpresent#not present")+")";
        int pos=ybox_.AddString( text.c_str() );
        ybox_.SetItemData( pos, info.chartype );
    }
    int fy=ybox_.FindString(0, aera::traits::get_wide_name(static_cast<aera::chars>(info.chartype)).c_str());
    ybox_.SetCurSel(fy==-1 ? 0 : fy);
}

void CPlotPropertyDlg::OnTddChanged(...)
{
    MakeFeatureList();
    OnAnyChange();
}
