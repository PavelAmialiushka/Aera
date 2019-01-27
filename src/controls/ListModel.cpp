#include "stdafx.h"

#include "ListModel.h"
#include "data/nodeFactory.h"

void list_info::serialization(serl::archiver &ar)
{
    ar.serial("based", based);
    ar.serial("navigate", navigate);
    ar.serial("peak", peak);

    ar.serial("location_tag", location_id);

    LOG( "location_id = " + location_id.string() );

    std::string str;
    foreach(aera::chars chrs, features)
    {
        str+=aera::traits::get_short_name(chrs) + " ";
    }
    ar.serial("parameters", str);


    if (ar.is_loading())
    {
        std::map<std::string, aera::chars> map;
        for (int index=aera::C_Begin; index<=aera::C_End; ++index)
        {
            map[aera::traits::get_short_name(static_cast<aera::chars>(index))]=
                static_cast<aera::chars>(index);
        }

        features.clear();
        for (std::string token; token=tokenizer::head(str), token.size();)
        {
            features.push_back( map[token] );
        }
    }
}

list_model::list_model(nodes::node* n)
    : list_maker( n ? n : nodes::factory().get_active_node().get())
    , valid_(false)
{
    base_time_ = 0;
    relative_mode_ = 0;

    result_ = list_result::create_blank_model(get_config());

    attach_listener(this);
}

list_model::~list_model()
{
    detach_listener(this);
}

void list_model::serialization(serl::archiver &arc)
{
    list_info info;
    info.based=get_rel_mode();
    info.navigate=get_nav_mode();
    info.features=get_features();
    info.peak=get_peak_mode();
    info.location_id=get_location();

    arc.serial(info);

    if (arc.is_loading())
    {
        process::suspend_lock lock(this);
        set_rel_mode(info.based);
        set_nav_mode(info.navigate);
        set_peak_mode(info.peak);
        set_features(&info.features[0], info.features.size());
        set_location(info.location_id);

        update_config();
    }
}

bool list_model::valid() const
{
    return valid_;
}

std::string list_model::get_header()
{
    return result_->get_header();
}

std::string list_model::line(unsigned index, bool &isSubHit)
{
    return result_->line(index, isSubHit);
}

unsigned list_model::size()
{
    return result_->size();
}

double list_model::get_time(unsigned index)
{
    return result_->get_time(index);
}

int list_model::search_time(double d)
{
    return result_->search_time(d);
}

void list_model::set_rel_pos(double time)
{
    if (relative_mode_)
    {
        base_time_= time;

        if (valid())
        {
            result_->base_time_ = base_time_;
        }
    }
}

void list_model::set_rel_mode(bool rm)
{
    relative_mode_=rm;
    base_time_=0;

    if (valid())
    {
        result_->relative_mode_ = relative_mode_;
        result_->base_time_ = base_time_;
    }
}

bool list_model::get_rel_mode() const
{
    return relative_mode_;
}

std::vector<aera::chars> list_model::get_avl_chars() const
{
    if (valid())
        return result_->avl_chars_;

    return std::vector<aera::chars>();
}

void list_model::update_config()
{
    result_ = list_result::create_blank_model( get_config() );
    valid_ = false;
}

void list_model::safe_on_finish(process::prslt r)
{
    result_ = boost::dynamic_pointer_cast<list_result>(r);

    result_->relative_mode_ = relative_mode_;
    result_->base_time_ = base_time_;
    valid_ = true;
}

void list_model::safe_on_restart()
{
    update_config();
}



