#pragma once

#include "data/processes.h"

#include "data/slice.h"
#include "data/data_fwrd.h"
#include "data/object_id.h"
#include "data/node.h"

#include "data/LocationSetup.h"

#include "plot/range.h"
#include "plot/screen.h"
#include "plottypes.h"

namespace nodes
{
    class grader;
}

enum { buffer_width=1024, buffer_height=768, };

MAKE_SHARED_STRUCT(base_plot_result);
struct base_plot_result : public process::rslt
{        
    shared_ptr<plot::screen> screen;
    std::vector<double> timemarks;
};

struct plot_config
{
    // axes info
    axe_info    xaxe;
    axe_info    yaxe;

    // graphic info
    plottype_t  plottype_;
    colormode_t colormode_;

    object_id  location_tag_;

    bool       tddata_;
    unsigned   binnumber_;
    bool       no_clusters_;

    // работа с диапазонами времен на графике
    bool       select_range_mode_;
};

class plot_maker
    : public process::host
{
public:

    //////////////////////////////////////////////////////////////////////////

    plot_maker(bool);
    ~plot_maker();

    //////////////////////////////////////////////////////////////////////////

    plot_info get_plot_info() const;
    void set_plot_info(const plot_info &);

    axe_info get_xaxe_info() const;
    void set_xaxe_info(const axe_info &);

    axe_info get_yaxe_info() const;
    void set_yaxe_info(const axe_info &);

    object_id get_location() const;
    void set_location(object_id);
    bool get_location_setup(location::Setup&) const;

    //////////////////////////////////////////////////////////////////////////

    void get_xminmax(bool &, double &, double &) const;
    void set_xminmax(bool, double, double);

    std::string get_xaxe_unit_name() const;
    std::string get_yaxe_unit_name() const;

    void get_yminmax(bool &, double &, double &) const;
    void set_yminmax(bool, double, double);

    int get_xlog() const;
    void set_xlog(int);

    int get_ylog() const;
    void set_ylog(int);

    plottype_t get_plottype() const;
    void set_plottype(plottype_t);

    plot::range get_xrange() const;
    plot::range get_yrange() const;

    void set_plot_params(int bins, bool clr, plottype_t line);
    void get_plot_params(int *bins, bool *clr, plottype_t *line) const;

    // работа с диапазонами времен на графике
    bool get_select_range_mode() const;
    bool get_tddata() const;

    void set_colormode(colormode_t cm);
    colormode_t get_actualcolormode() const;
    colormode_t get_colormode();

    void set_visibility(bool);

    //////////////////////////////////////////////////////////////////////////

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void serialization(serl::archiver &ar);

    void restart();

    void set_parent(nodes::pnode p);
    nodes::pnode get_parent() const;

protected:
    nodes::pnode     parent_node_;

    struct config
            : process::config
            , plot_config
    {
    } config_;

    struct processor;
};
