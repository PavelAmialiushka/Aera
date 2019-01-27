#pragma once

#include "data/aecollection.h"
#include "data/node.h"

#include "data/grader.h"

#include "ListMaker.h"

//////////////////////////////////////////////////////////////////////////

struct list_info
{
    int  	based;
    bool	navigate;
    bool    peak;
    std::vector<aera::chars>	features;
    object_id location_id;

    void serialization(serl::archiver &arc);
};

//////////////////////////////////////////////////////////////////////////

class list_model
        : public list_maker
        , public process::host_listener
{
public:
    list_model(nodes::node* n = 0);
    ~list_model();

    std::string get_header();
    std::string line(unsigned index, bool& isSubhit);
    unsigned    size();

    double  get_time(unsigned index);
    int     search_time(double d);

    void set_rel_pos(double);
    void set_rel_mode(bool);
    bool get_rel_mode() const;

    std::vector<aera::chars> get_avl_chars() const;

    void update_config();

    void safe_on_finish(process::prslt);
    void safe_on_restart();

    void serialization(serl::archiver &arc);

    bool valid() const;

private:
    plist_result result_;
    bool         valid_;

    double       base_time_;
    int          relative_mode_;
};

