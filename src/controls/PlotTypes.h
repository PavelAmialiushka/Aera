#pragma once

//////////////////////////////////////////////////////////////////////////

// forward declaration
namespace plot
{
class range;
class screen;
}

//////////////////////////////////////////////////////////////////////////

struct axe_info
{
    int  chartype;
    bool auto_scale;
    double  lo;
    double  hi;
    int  log_scale;

    bool operator==(const axe_info &other) const
    {
        return chartype==other.chartype
               && auto_scale==other.auto_scale
               && (auto_scale || fabs(lo-other.lo)<1e-6)
               && (auto_scale || fabs(hi-other.hi)<1e-6)
               && log_scale==other.log_scale;
    }

    bool operator!=(const axe_info &other) const
    {
        return !operator==(other);
    }
};

//////////////////////////////////////////////////////////////////////////

enum plottype_t
{
    scatter=0,
    history,
    minmax,
    maxmin,
    distribution,
    summation,
};

enum colormode_t
{
    single,
    bychannel,
};

struct plot_info
{
    plottype_t  plottype;
    bool	colormode;
    bool	tddata;
    int    	bins;
    bool        use_clusters;

    bool operator==(const plot_info &other) const
    {
        return plottype==other.plottype
               && colormode==other.colormode
               && tddata==other.tddata
               && bins==other.bins
               && use_clusters == other.use_clusters;
    }


    bool operator!=(const plot_info &other) const
    {
        return !operator==(other);
    }
};
