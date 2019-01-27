#pragma once

#include "data/aecollection.h"
#include "data/node.h"

#include "data/grader.h"

//////////////////////////////////////////////////////////////////////////

MAKE_SHARED_STRUCT(list_config);
struct list_config : process::config
{
    bool   navigate_mode_;
    bool   peak_mode_;
    size_t risetime_pos_;
    object_id location_tag_;

    std::vector<aera::chars> features_;
    std::vector<std::string> formats_;
    std::string              header_;
};

MAKE_SHARED_STRUCT(list_result);
struct list_result : nodes::result
{
    plist_config             config_;
    data::pslice             slice_;
    std::vector<unsigned>    indexes_;

    int                      relative_mode_;
    double                   base_time_;

    // свойства вытащенные из данных
    std::vector<aera::chars> avl_chars_; // доступные переменные
    bool                     event_mode_; // наличие событий и подхитов
    bool                     auto_peak_mode_; // авто режим пика (на основе событий)
    std::vector<bool>        present_; // наличие переменных

    unsigned size() const;
    std::string get_header() const;
    std::string line(unsigned index, bool &isSubHit) const;

    std::string make_line(unsigned index, bool& isSubHit) const;

    double get_time(unsigned index, bool strict_time = true) const;
    unsigned search_time(double time) const;

    static plist_result create_blank_model(list_config const& );
};

class list_maker
        : public process::host_t<list_result>
{
public:
    DEFINE_NERROR(access_error, "list_maker: access error");

    list_maker(nodes::node *item);
    ~list_maker();

    void set_features(aera::chars feats[], unsigned sz);
    std::vector<aera::chars> get_features() const;

    void set_nav_mode(bool);
    bool get_nav_mode() const;

    void set_peak_mode(bool x);
    bool get_peak_mode() const;

    object_id get_location() const;
    void set_location(object_id);

    list_config get_config() const;

    unsigned size() const;   // whole size
    //////////////////////////////////////////////////////////////////////////

    void serialization(serl::archiver &);

private:
    void setup(process::hostsetup &);
    process::processor* create_processor();

public:
    void restart();

private:

    std::string	make_line(data::pslice slice, unsigned index, bool& isSubHit) const;
    void		add_feature(aera::chars achar);

    void	set_cursor(unsigned) const;

public:
    std::vector< boost::function0<void> > sig_updated_;

private:
    list_config config_;
};
