#ifndef PLOT_MODEL_H
#define PLOT_MODEL_H

#include "PlotMaker.h"
#include "BitmapMaker.h"

#include "data/locationClusters.h"

class plot_model
        : public plot_maker
        , public process::host_listener
{
public:
    plot_model(bool m);
    ~plot_model();

    bool valid() const;

    void set_dimensions(int cx, int cy);

    void attach_listener(process::host_listener *ls);
    void detach_listener(process::host_listener *ls);

    std::vector<double> get_timemarks() const;
    shared_ptr<CBitmap> get_bitmap() const;

    plot::range get_xrange() const;
    plot::range get_yrange() const;

    axe_info get_xaxe_info() const;
    axe_info get_yaxe_info() const;

    location::pclusters get_clusters() const;
    bool get_working_location_setup(location::Setup& result) const;

    void get_xminmax(bool &a, double &z, double &x) const;
    void get_yminmax(bool &a, double &z, double &x) const;

    void safe_on_restart();
    void safe_on_finish(process::prslt);

private:
    bool valid_;
    BitmapMaker  bitmap_maker_;
    pbitmap_result result_;
};

#endif // PLOT_MODEL_H
