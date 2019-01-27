#include "stdafx.h"

#include "PlotMaker.h"

#include "plot/screen.h"
#include "plot/range.h"

#include "utilites/foreach.hpp"
#include "utilites/Localizator.h"

#include "data/nodeFactory.h"
#include "data/node.h"
#include "data/stage.h"

#include "data/grader.h"

//////////////////////////////////////////////////////////////////////////

class plot_maker;

#include "PlotPool.h"

#include "data/PlanarSetup.h"

using namespace aera::traits;

#include "PlotModelAddons.h"

//////////////////////////////////////////////////////////////////////////

// model

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

plot_maker::plot_maker(bool mode)
{
    config_.xaxe.auto_scale = true;
    config_.yaxe.auto_scale = true;
    config_.xaxe.lo = 0; config_.xaxe.hi = 1;
    config_.yaxe.lo = 0; config_.yaxe.hi = 1;
    config_.xaxe.log_scale = 0;
    config_.yaxe.log_scale = 0;
    config_.plottype_ = scatter;
    config_.colormode_ = bychannel;
    config_.tddata_ = false;
    config_.binnumber_ = 200;
    config_.no_clusters_ = false;
    config_.select_range_mode_ = mode;

    PlotManager::instance()->attach_maker(this);

    config_.xaxe.chartype=aera::C_Time;
    config_.yaxe.chartype=aera::C_Amplitude;

    axe_info x=PlotManager::instance()->get_current_axe_info();
    config_.xaxe.lo=x.lo;
    config_.xaxe.hi=x.hi;
    config_.xaxe.auto_scale=x.auto_scale;

    // рестартует автоматически
    parent_node_ = nodes::factory().get_active_node();
    host::set_parent( parent_node_.get() );
    restart();
}

plot_maker::~plot_maker()
{    
    detach_host();
    PlotManager::instance()->detach_maker(this);
}

void plot_maker::set_visibility(bool new_state)
{
    host::set_visibility(new_state);
}

plot_info plot_maker::get_plot_info() const
{
    plot_info res;
    get_plot_params(&res.bins, &res.colormode, &res.plottype);
    res.tddata=config_.tddata_;
    res.use_clusters=!config_.no_clusters_;
    return res;
}

void plot_maker::set_plot_info(const plot_info &pi)
{
    config_.tddata_=pi.tddata;
    config_.no_clusters_ = !pi.use_clusters;
    set_plot_params(pi.bins, pi.colormode, pi.plottype);
}

axe_info plot_maker::get_xaxe_info() const
{
    return config_.xaxe;
}

axe_info plot_maker::get_yaxe_info() const
{
    return config_.yaxe;
}

void plot_maker::set_xaxe_info(const axe_info &info)
{
    config_.xaxe = info;
    if (get_xaxe_info().chartype==aera::C_Time && !config_.select_range_mode_)
    {
        PlotManager::instance()->set_xminmax(info.auto_scale, info.lo, info.hi);
    }
    restart();
}

void plot_maker::set_yaxe_info(const axe_info &info)
{
    config_.yaxe = info;
    restart();
}

void plot_maker::get_xminmax(bool &a, double &z, double &x) const
{
    a=config_.xaxe.auto_scale;
    z=config_.xaxe.lo;
    x=config_.xaxe.hi;
}

void plot_maker::set_xminmax(bool xa, double z, double x)
{
    config_.xaxe.auto_scale=xa; config_.xaxe.lo=z, config_.xaxe.hi=x;
    if (get_xaxe_info().chartype==aera::C_Time && !config_.select_range_mode_)
    {
        PlotManager::instance()->set_xminmax(xa, z, x);
    }
    restart();
}

void plot_maker::get_yminmax(bool &a, double &z, double &x) const
{
    a=config_.yaxe.auto_scale;
    z=config_.yaxe.lo;
    x=config_.yaxe.hi;
}

object_id plot_maker::get_location() const
{
    return config_.location_tag_;
}

void plot_maker::set_location(object_id tag)
{
    // создавать новый нод нужно не под блокировкой
    // TODO загрузка локации
    config_.location_tag_=tag;

    // не ждем установки родителя
    parent_node_ = nodes::factory().get_active_node_by_location( tag );
    host::set_parent( parent_node_.get() );
}

bool plot_maker::get_location_setup(location::Setup &result) const
{
    object_id id=get_location();
    if (id!=object_id())
    {
        location::Setup sp=nodes::factory().get_location(id);
        if (!sp.empty())
        {
            result = sp;
            return true;
        }
    }

    return false;
}

std::string plot_maker::get_xaxe_unit_name() const
{
    // TODO не учтен параметрик
    return aera::traits::get_unit_name(
                static_cast<aera::chars>(config_.xaxe.chartype));
}

std::string plot_maker::get_yaxe_unit_name() const
{
    // TODO не учтен параметрик
    return aera::traits::get_unit_name(
                static_cast<aera::chars>(config_.yaxe.chartype));
}

void plot_maker::set_yminmax(bool a, double z, double x)
{
    config_.yaxe.auto_scale=a;
    config_.yaxe.lo=z;
    config_.yaxe.hi=x;
    restart();
}


int plot_maker::get_xlog() const
{
    return config_.xaxe.log_scale;
}

void plot_maker::set_xlog(int v)
{
    config_.xaxe.log_scale=v;
    restart();
}

int plot_maker::get_ylog() const
{
    return config_.yaxe.log_scale;
}

void plot_maker::set_ylog(int v)
{
    config_.yaxe.log_scale=v;
}

plottype_t plot_maker::get_plottype() const
{
    return config_.plottype_;
}

void plot_maker::set_plottype(plottype_t type)
{
    if (config_.plottype_!=type)
    {
        config_.plottype_=type;
        restart();
    }
}

plot::range plot_maker::get_xrange() const
{
    return plot::range(config_.xaxe.lo, config_.xaxe.hi, config_.xaxe.log_scale);
}

plot::range plot_maker::get_yrange() const
{
    return plot::range(config_.yaxe.lo, config_.yaxe.hi, config_.yaxe.log_scale);
}

void plot_maker::set_plot_params(int bins, bool clr, plottype_t line)
{
    config_.binnumber_=bins;
    config_.plottype_ =line;
    config_.colormode_=clr ? bychannel : single;

    restart();
}

void plot_maker::get_plot_params(int *bins, bool *clr, plottype_t *line) const
{
    if (bins) *bins=config_.binnumber_;
    if (line) *line=config_.plottype_;
    if (clr)  *clr= config_.colormode_!=single;
}

bool plot_maker::get_tddata() const
{
    return config_.tddata_;
}

// работа с диапазонами времен на графике

bool plot_maker::get_select_range_mode() const
{
    return config_.select_range_mode_;
}

void plot_maker::set_colormode(colormode_t cm)
{
    config_.colormode_=cm;
}

colormode_t plot_maker::get_colormode()
{
    return config_.colormode_;
}

colormode_t plot_maker::get_actualcolormode() const
{
    bool tdd_single=config_.tddata_ && is_tdd_common_data((aera::chars)config_.xaxe.chartype) && is_tdd_common_data((aera::chars)config_.yaxe.chartype);
    bool tdd_multi=config_.tddata_ && (!is_tdd_common_data((aera::chars)config_.xaxe.chartype) || !is_tdd_common_data((aera::chars)config_.yaxe.chartype));

    bool multi_color= tdd_multi || (config_.colormode_ && !tdd_single);
    return multi_color ? bychannel : single;
}


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////

struct plot_maker::processor
        : public process::processor_t<
                    plot_maker::config,
                    base_plot_result,
                    nodes::grader_result>
{
    virtual bool process()
    {
        result->screen.reset( new plot::screen());
        result->screen->set_buffer(buffer_width, buffer_height);

        form_screen();

        status_helper = strlib::strf("%d+%d",
                                     source->ae->size(),
                                     source->tdd->size());

        return true;
    }

    // внутренние переменные
    bool tdd_single;
    bool tdd_multi_channel;
    bool multi_color;
    int  channel;
    double abscissa;
    double ordinate;
    unsigned ch_index;
    unsigned index;
    std::vector<double> timemarks;

    plot::range xaxe;
    plot::range yaxe;
    std::map<int, shared_ptr<class histogramma> > hists;
    data::pslice slice;

public:
    bool form_screen();

    // scatter
    void fill_scatter_mode();

    // линейный график
    void fill_line_mode();
    void end_line_mode();

    void set_screen_ranges();
    void form_screen_timemarks();
};


void plot_maker::processor::set_screen_ranges()
{
    int k=0;
    try
    {
        data::pslice slice=
            config->tddata_
            ? source->tdd
            : source->ae;

        data::pslice other_slice=
            !config->tddata_
            ? source->tdd
            : source->ae;

        result->screen->clear();

        if (config->xaxe.auto_scale)
        {
            data::lohi_value mmx = slice->get_minmax_limits((aera::chars)config->xaxe.chartype);
            data::lohi_value other_mmx = other_slice->get_minmax_limits((aera::chars)config->xaxe.chartype);

            widen_range_begin(config->xaxe.lo, config->xaxe.hi);
            widen_range(config->xaxe.lo, config->xaxe.hi, mmx.lo);
            widen_range(config->xaxe.lo, config->xaxe.hi, mmx.hi);
            widen_range(config->xaxe.lo, config->xaxe.hi, other_mmx.lo);
            widen_range(config->xaxe.lo, config->xaxe.hi, other_mmx.hi);

            int coord_mode =
                config->xaxe.chartype == aera::C_CoordX ? 1 :
                config->xaxe.chartype == aera::C_CoordY ? 2 : 0;

            using namespace location;
            if (coord_mode )
            {
                // linear and planar as well
                if (ZonalSetup* setup = source->location.zonal())
                {
                    double a, b;
                    setup->get_coordinate_range(config->xaxe.chartype, a, b);
                    widen_range(config->xaxe.lo, config->xaxe.hi, a);
                    widen_range(config->xaxe.lo, config->xaxe.hi, b);
                }
            }
        }

        if (config->yaxe.auto_scale)
        {
            data::lohi_value mmy = slice->get_minmax_limits((aera::chars)config->yaxe.chartype);
            data::lohi_value other_mmy = other_slice->get_minmax_limits((aera::chars)config->yaxe.chartype);

            widen_range_begin(config->yaxe.lo, config->yaxe.hi);
            widen_range(config->yaxe.lo, config->yaxe.hi, mmy.lo);
            widen_range(config->yaxe.lo, config->yaxe.hi, mmy.hi);
            widen_range(config->yaxe.lo, config->yaxe.hi, other_mmy.lo);
            widen_range(config->yaxe.lo, config->yaxe.hi, other_mmy.hi);

            int coord_mode =
                config->yaxe.chartype == aera::C_CoordX ? 1 :
                config->yaxe.chartype == aera::C_CoordY ? 2 : 0;

            using namespace location;
            if (coord_mode)
            {
                // linear and planar as well
                if (ZonalSetup* setup = source->location.zonal())
                {
                    double a, b;
                    setup->get_coordinate_range(config->yaxe.chartype, a, b);
                    widen_range(config->yaxe.lo, config->yaxe.hi, a);
                    widen_range(config->yaxe.lo, config->yaxe.hi, b);
                }
            }
        }

        xaxe=plot::range(config->xaxe.lo, config->xaxe.hi, config->xaxe.log_scale);
        yaxe=plot::range(config->yaxe.lo, config->yaxe.hi, config->yaxe.log_scale);
        result->screen->set_xrange(xaxe);
        result->screen->set_yrange(yaxe);

    } LogException("set_screen_range");
}


bool plot_maker::processor::form_screen()
{
    try
    {
        set_screen_ranges();

        slice = config->tddata_
                ? source->tdd
                : source->ae;

        hists.clear();

        // tdd данные описывающие каналы
        tdd_single=config->tddata_
                && is_tdd_common_data((aera::chars)config->xaxe.chartype)
                && is_tdd_common_data((aera::chars)config->yaxe.chartype);

        // tdd данные не относящиеся к каналам
        tdd_multi_channel=config->tddata_
                && (!is_tdd_common_data((aera::chars)config->xaxe.chartype)
                 || !is_tdd_common_data((aera::chars)config->yaxe.chartype));
        multi_color= tdd_multi_channel || (config->colormode_ && !tdd_single);

        std::vector<aera::chars> chars=slice->get_chars();
        if (std::count(STL_II(chars), config->xaxe.chartype) &&
                std::count(STL_II(chars), config->yaxe.chartype))
        {
            unsigned size = slice->size();
            unsigned channel_count=tdd_multi_channel
                     ? slice->get_channel_count()
                     : 1;
            for (index=0; index<size; ++index)
            {
                for (ch_index=0; ch_index<channel_count; ++ch_index)
                {
                    channel=static_cast<int>(slice->get_value(index, aera::C_Channel, ch_index));
                    abscissa=slice->get_value(index, (aera::chars)config->xaxe.chartype, ch_index);
                    ordinate=slice->get_value(index, (aera::chars)config->yaxe.chartype, ch_index);

                    // канал выбран
                    bool use_record = channel==0 ? true :
                            source->working_channels->operator[](channel-1)
                            && source->selected_channels->operator[](channel-1);

                    // параметрик
                    if (tdd_single)
                        use_record = true;

                    // канал присутствует и задействован
                    if (use_record)
                    {
                        if (config->plottype_==scatter)
                           fill_scatter_mode();
                        else
                           fill_line_mode();
                    }

                    if (!check_status(index, size))
                        return false;
                }
            }

            if (config->plottype_!=scatter)
                end_line_mode();

            result->timemarks.clear();
            if (config->xaxe.chartype==aera::C_Time)
            {
                form_screen_timemarks();
            }
        }
    } LogException("plot_maker::form_screen");
    return true;
}

void plot_maker::restart()
{
    host::restart_using(config_);
}

void plot_maker::set_parent(nodes::pnode p)
{
    parent_node_ = p;
    host::set_parent( p.get() );
}

nodes::pnode plot_maker::get_parent() const
{
    return parent_node_;
}

void plot_maker::processor::fill_scatter_mode()
{
    if (abscissa==0 && config->xaxe.log_scale) abscissa=0.1;
    if (ordinate==0 && config->yaxe.log_scale) ordinate=0.1;


    bool select=
            source->selection
            && source->selection->contains( &slice->get_value(index) );

    if (xaxe.lo <= abscissa && abscissa <= xaxe.hi
            && yaxe.lo <= ordinate && ordinate <= yaxe.hi)
    {
        result->screen->set_dot(abscissa, ordinate,
                                select ?  colors::selected :
                                multi_color ? channel :
                                colors::normal);
    }
}


void plot_maker::processor::fill_line_mode()
{
    int local_channel=multi_color ? channel : 0;

    if (!hists[local_channel])
    {
        hists[local_channel].reset(
            new histogramma(config->plottype_,
                            config->binnumber_,
                            xaxe.get_left(),
                            xaxe.get_right(),
                            xaxe.log_scale));
    }

    hists[local_channel]->push(abscissa, ordinate);
}

void plot_maker::processor::end_line_mode()
{
    result->screen->reset();

    typedef std::map<int, shared_ptr<histogramma> >::value_type value_t;
    foreach(value_t hs, hists)
    {
        histogramma &hist=*hs.second;
        hist.process(config->plottype_);
    }

    if (config->yaxe.auto_scale)
    {
        bool atfirst=true;
        std::pair<double, double> p;
        foreach(value_t hs, hists)
        {
            histogramma &hist=*hs.second;

            switch (config->plottype_)
            {
            case distribution:
            case summation:
            case history:
            {
                p=hist.get_minmax_limits(config->plottype_==history);
                break;
            }
            case minmax:
                p=std::make_pair(hist[0].second, hist[hist.size()-1].second);
                break;
            case maxmin:
                p=std::make_pair(hist[hist.size()-1].second, hist[0].second);
                break;
            }

            if (atfirst)
            {
                config->yaxe.lo=p.first;
                config->yaxe.hi=p.second;
                atfirst=false;
            }
            else
            {
                config->yaxe.lo=std::min(config->yaxe.lo, p.first);
                config->yaxe.hi=std::max(config->yaxe.hi, p.second);
            }
        }
        if (config->yaxe.log_scale && 0==config->yaxe.hi)
            config->yaxe.hi=1.;

        if (config->yaxe.log_scale && 0==config->yaxe.lo)
            config->yaxe.lo=std::min(config->yaxe.hi, 1.);

        result->screen->set_yrange(plot::range(config->yaxe.lo, config->yaxe.hi, config->yaxe.log_scale));
    }

    // соединяем последовательно точки диаграммы
    foreach(value_t hs, hists)
    {
        int channel = hs.first;
        histogramma &hist=*hs.second;

        bool atfirst=true;
        for (unsigned index=0; index<hist.size(); ++index)
        {
            if (config->plottype_==history && hist.count(index)==0)
                continue;

            std::pair<double, double> v=hist[index];
            if (atfirst)
            {
                result->screen->move_to(v.first, v.second);
                atfirst=false;
            }

            result->screen->line_to(v.first, v.second,
                                 multi_color ? channel : colors::normal);
        }
    }
}

void plot_maker::processor::form_screen_timemarks()
{
    try
    {
        data::pslice slice=source->raw;
        for (unsigned index=0; index<slice->size(); ++index)
        {
            data::raw_record record=slice->get_raw_record(index);
            if (record.type_==data::CR_TIMEMARK)
            {
                result->timemarks.push_back( record.time_ );
            }
        }

    } LogException("form_screen_timemarks");
}


void plot_maker::setup(process::hostsetup & setup)
{
    setup.name = _ls("Plot_maker#Drawing plot");
    setup.weight = 0.1;
}

process::processor *plot_maker::create_processor()
{
    return new processor;
}

void plot_maker::serialization(serl::archiver &ar)
{
    ar.serial("xautorange", config_.xaxe.auto_scale);
    ar.serial("xlog", config_.xaxe.log_scale);
    ar.serial("xaxe", config_.xaxe.chartype);
    ar.serial("xmin", config_.xaxe.lo);
    ar.serial("xmax", config_.xaxe.hi);

    ar.serial("yautorange", config_.yaxe.auto_scale);
    ar.serial("ylog", config_.yaxe.log_scale);
    ar.serial("yaxe", config_.yaxe.chartype);
    ar.serial("ymin", config_.yaxe.lo);
    ar.serial("ymax", config_.yaxe.hi);

    ar.serial("plottype", serl::makeint(config_.plottype_));
    ar.serial("colormode", serl::makeint(config_.colormode_));
    ar.serial("tddata", serl::makeint(config_.tddata_));
    ar.serial("noclusters", serl::makeint(config_.no_clusters_));

    ar.serial("location_tag", config_.location_tag_);
    if (ar.is_loading())
    {
        set_location(config_.location_tag_);
        restart();
    }
}
