#include "stdafx.h"

#include "ListMaker.h"

#include "data/traits.h"
#include "data/nodefactory.h"
#include "data/blank_slice.h"

#include "utilites/tokenizer.h"
#include "utilites/strings.h"
#include "utilites/Localizator.h"

#include <cmath>
#include <boost/format.hpp>
#include "ListModel.h"

using namespace data;

list_maker::list_maker(nodes::node *node)    
{    
    config_.navigate_mode_ = false;
    config_.peak_mode_ = 0;

    aera::chars fts[]=
    {
        aera::C_Time,
        aera::C_Channel,
        aera::C_Amplitude,
        aera::C_Energy,
        aera::C_Duration
    };
    set_features(fts, SIZEOF(fts));
    set_parent(node);
}

list_maker::~list_maker()
{
    detach_host();
}

void list_maker::set_nav_mode(bool m)
{
    if (config_.navigate_mode_!=m)
    {
        config_.navigate_mode_=m;
    }

    restart();
}

bool list_maker::get_nav_mode() const
{    
    return config_.navigate_mode_;
}

void list_maker::set_peak_mode(bool x)
{
    config_.peak_mode_=x;
    restart();
}

bool list_maker::get_peak_mode() const
{
    return config_.peak_mode_;
}

void list_maker::add_feature(aera::chars achar)
{
    std::string temp;

    try
    {
        std::string format=aera::traits::get_format(achar)+' ';
        config_.formats_.push_back( format );

        temp=format.substr(0, format.find("."))+"s ";
        boost::format fmt(temp);
        std::string short_name = aera::traits::get_short_name(achar);

        config_.header_+=boost::io::str( fmt % short_name );

    } LogExceptionPath("list_maker::add_feature");
}

void list_maker::set_features(aera::chars feats[], unsigned sz)
{
    config_.header_="";
    config_.formats_.clear();

    config_.features_.assign(feats, feats+sz);
    std::for_each(feats, feats+sz, bind(&list_maker::add_feature, this, _1));

    restart();
}

object_id list_maker::get_location() const
{
    return config_.location_tag_;
}

void list_maker::set_location(object_id id)
{
    config_.location_tag_ = id;
    set_parent( nodes::factory().get_active_node_by_location(id).get());
}

list_config list_maker::get_config() const
{
    return config_;
}

std::string list_result::get_header() const
{
    std::string header;

    unsigned index=0;
    foreach(aera::chars ch, config_->features_)
    {
        if (true /*present_[index]*/)
        {
            std::string format=aera::traits::get_format(ch)+' ';
            format=format.substr(0, format.find("."))+"s ";
            boost::format fmt( format );

            std::string short_name = aera::traits::get_short_name(ch);

            if (ch == aera::C_Time && (config_->peak_mode_ && relative_mode_ || auto_peak_mode_))
                short_name = short_name + " (by peak)";

            header+=boost::io::str( fmt % short_name );
        }
        ++index;
    }

    return header;
}

std::string list_result::line(unsigned index, bool& isSubHit) const
{
    if (index < indexes_.size())
        return make_line(indexes_[index], isSubHit);
    return "";
}

unsigned list_result::size() const
{
    return indexes_.size();
}

//////////////////////////////////////////////////////////////////////////

namespace
{

/**
 * @brief вспомогательная структура для поиска определенного времени
 */
struct slice_time_iter : std::iterator<std::random_access_iterator_tag, double>
{
    data::pslice    slice;
    const unsigned *array;
    unsigned        index;
    bool            use_peak;

    double get_value() const
    {
        if (index < slice->size())
        {
            bool israw = slice->get_type(array[index]) == RAW;
            return slice->get_value(array[index],
                     use_peak && !israw
                                    ? aera::C_PeakTime
                                    : aera::C_Time);
        }
        return 0;
    }

    double operator*() const
    {
        return get_value();
    }

    bool operator<(const slice_time_iter &d) const
    {
        return index < d.index;
    }

    bool operator==(const slice_time_iter &d) const
    {
        return index == d.index;
    }

    bool operator!=(const slice_time_iter &d) const
    {
        return index != d.index;
    }

    slice_time_iter &operator++()
    {
        ++index;
        return *this;
    }
    slice_time_iter operator++(int)
    {
        slice_time_iter temp(*this);
        ++temp;
        return temp;
    }
    slice_time_iter &operator--()
    {
        --index;
        return *this;
    }
    slice_time_iter operator--(int)
    {
        slice_time_iter temp(*this);
        --temp;
        return temp;
    }
    slice_time_iter &operator+=(int z)
    {
        index+=z;
        return *this;
    }
    slice_time_iter operator+(int z) const
    {
        return slice_time_iter(slice, array, use_peak, index+z);
    }
    slice_time_iter operator-(int z) const
    {
        return slice_time_iter(slice, array, use_peak, index-z);
    }
    size_t operator-(const slice_time_iter &l) const
    {
        return index-l.index;
    }

    slice_time_iter() : slice(), array(0), index(0), use_peak(0)
    {}

    slice_time_iter(data::pslice cur, const unsigned *array, bool peak, unsigned i)
        : slice(cur)
        , array(array)
        , index(i)
        , use_peak(peak)
    {}
};
}

//////////////////////////////////////////////////////////////////////////

unsigned list_result::search_time(double time) const
{
    unsigned size = indexes_.size();
    if (slice_ && size)
    {
        slice_time_iter x=
            std::lower_bound(
                slice_time_iter(slice_, &indexes_[0], false, 0),
                slice_time_iter(slice_, &indexes_[0], false, size),
                time);

        slice_time_iter b = slice_time_iter(
                slice_, &indexes_[0], false, std::max(0, (int)x.index-10));
        slice_time_iter e = slice_time_iter(
                slice_, &indexes_[0], false, std::min(size, x.index+10));

        slice_time_iter y = std::find(b, e, time);
        if (y == e)
            return x.index;
        else
            return y.index;

    }
    return 0;
}

plist_result list_result::create_blank_model(list_config const& cfg)
{
    plist_result r(new list_result);
    r->config_.reset( new list_config(cfg) );
    r->slice_ = pslice( new data::blank_slice() );
    return r;
}

double list_result::get_time(unsigned index, bool strict_time) const
{
    if (index < indexes_.size() && slice_)
    {
        index=indexes_[index];

        if (auto_peak_mode_ && !strict_time)
            return slice_->get_value(index, aera::C_PeakTime);
        else
        {
            double rt=!strict_time
                    && config_->peak_mode_
                    && relative_mode_
                    && AE==slice_->get_type(index)
                    ? slice_->get_value(index, aera::C_Risetime) / 1e6
                    : 0;

            return slice_->get_value(index) + rt;
        }
    }

    return 0.0;
}

class list_processor
        : public process::processor_t
            <list_config
            , list_result
            , nodes::result>
{
    bool process()
    {
        result->config_ = config;
        data::pslice slice = config->navigate_mode_
            ? source->raw
            : source->raw->merge(
                  source->ae->merge(
                      source->ae_sub));

        result->slice_ = slice;
        result->indexes_.reserve( slice->size() );
        result->present_.resize(config->features_.size());

        result->avl_chars_= slice->get_chars();
        result->auto_peak_mode_ = std::count( STL_II(result->avl_chars_), aera::C_PeakTime ) >0;
        result->event_mode_ = std::count( STL_II(result->avl_chars_), aera::C_SubHitCount) > 0;

        unsigned index=0;
        foreach(aera::chars ch, config->features_)
        {
            result->present_[index]=std::count(STL_II(result->avl_chars_), ch);
            ++index;
        }

        // TODO присутствуют ли записи в выделении
        bool show_all=!source->selection || source->selection->size() == 0;

        // пропускаем записи до начала теста
        index=0;
        for (; index<slice->size(); ++index)
        {
            if (RAW==slice->get_type(index) &&
                    data::CR_RESUME==slice->get_raw_record(index).type_)
            {
                break;
            }
        }

        bool isSelectedEvent = true;
        for (; index<slice->size(); ++index)
        {
            int type = slice->get_type(index);
            switch(type)
            {
            case AE_SUBHITS:
            case AE:
            {
                if (show_all) break;

                bool isSubhit = type == AE_SUBHITS;
                ae_record rec = &slice->get_value(index);

                bool isSelected = source->selection->contains(rec);

                if (!isSubhit)
                {
                    isSelectedEvent = isSelected;
                    if (isSelected)
                        break;
                }
                else
                {
                    if (isSelectedEvent)
                        break;
                }
                // игнорировать
                continue;
            } break;
            default:
                if (!show_all)
                {
                    continue;
                }
            }
            result->indexes_.push_back(index);
        }

        return true;
    }
};


std::vector<aera::chars> list_maker::get_features() const
{
    return config_.features_;
}

void list_maker::setup(process::hostsetup &s)
{
    s.name = _ls("List-maker#list preparation");
    s.weight = 0.2;
}

process::processor *list_maker::create_processor()
{
    return new list_processor;
}

void list_maker::restart()
{
    restart_using( config_ );
}

std::string list_result::make_line(unsigned index, bool& isSubHit) const
{
    std::string line;
    try
    {
        int type = slice_->get_type(index);
        bool useRelative = relative_mode_ && !auto_peak_mode_;
        bool usePeaks = config_->peak_mode_ && useRelative;

        isSubHit = event_mode_ ? ( AE_SUBHITS==type ) : false;

        if (AE==type || AE_SUBHITS==type)
        {
            double time;

            if (type == AE)
            {
                double rt = 0;
                if (usePeaks)
                   rt = slice_->get_value(index, aera::C_Risetime) / 1e6;
                time = slice_->get_value(index) + rt;

                if (useRelative)
                    time -= base_time_;

            } else //(type == AE_SUBHITS)
            {
                time = config_->peak_mode_ || auto_peak_mode_
                        ? slice_->get_value(index, aera::C_RelPeakTime)
                        : slice_->get_value(index, aera::C_RelTime);
            }

            if (!config_->features_.empty())
            {
                std::string fmt = config_->formats_[0];
                if (time !=0 && (useRelative || type == AE_SUBHITS))
                    fmt = strlib::replace(fmt, "%", "%+");

                line += boost::io::str( boost::format(fmt) % time);
            }

            for (unsigned i=1; i<config_->features_.size(); ++i)
            {
                if (true /* present_[i]*/)
                {
                    double value=slice_->get_value(index, config_->features_[i]);
                    if (_isnan(value))
                    {
                        std::string format=aera::traits::get_format(config_->features_[i])+' ';
                        format=format.substr(0, format.find("."))+"s ";
                        boost::format fmt( format );
                        line+=boost::io::str( fmt % "-" );
                    }
                    else
                        line += boost::io::str( boost::format(config_->formats_[i]) % value );
                }
            }
        }
        else if (RAW==slice_->get_type(index))
        {
            data::raw_record record=slice_->get_raw_record(index);
            double value=slice_->get_value(index);
            if (usePeaks) value -= base_time_;
            line += boost::io::str( boost::format(config_->formats_[0])
                                    % value);
            if (record.string_)
            {
                std::string text= std::string(record.string_, record.string_size_);
                size_t pos=text.find_first_of(std::string(1, '\0'));
                if (pos!=std::string::npos)
                    text=text.substr(0, pos);
                line += text;
            }
        }
    }
    catch (std::exception &)
    {
        line = "Error";
    }
    return line;
}
