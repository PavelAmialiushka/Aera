#pragma once

#include "data_fwrd.h"
#include "object_id.h"
#include "criterion.h"

#include "select_action.h"
#include "LocationSetup.h"

#include "utilites/serl/Archive.h"

#include <boost/thread/xtime.hpp>

// TODO: убрать ссылка
// разделить файл process на два внешний и внутренний
#include "processes.h"
#include "hitset.h"
//////////////////////////////////////////////////////////////////////////

#include "channel_sheme.h"

template<typename T> struct type_of
{
    type_of(int) {}
};

namespace nodes
{

class loader;
class sorter;
class inspector;
class modifier;
class filter;
class selector;
class locator;
class grader;

using monpac::criterion;

class slot
{
public:
    void connect(void *ptr, boost::function0<void> f)
    {
        map_.insert( std::make_pair(ptr, f) );
    }

    void disconnect(void *ptr)
    {
        map_.erase( ptr );
    }

    void	raise_signal()
    {
        std::multimap<void *, boost::function0<void> >::iterator index=map_.begin();
        for (; index!=map_.end(); ++index)
        {
            index->second();
        }
    }
private:
    std::multimap<void *, boost::function0<void> > map_;
};


enum
{
    ordinary_mode,
    layout_mode
};

class factory_impl
    : public utils::singleton<factory_impl>
    , public serl::serializable
{
    friend class utils::singleton<factory_impl>;
    factory_impl();
    ~factory_impl();

public:

    double get_progress() const;
    double get_progress(int &loc, std::string& stat) const;

public:

    shared_ptr< process::suspend_lock > block_execution();

    void reload();
    void open(std::string filename);

    void set_parametric_data(parametric *params);
    void get_parametric_data(parametric *params);

    void reset_filter();
    void set_filter(std::string fstr);
    std::string get_filter() const;

    //void set_sel_channel_list(bool* pb, unsigned size);
    void set_channel_sheme(channel_sheme const &sheme);
    void get_channel_sheme(channel_sheme &sheme) const;
    void get_sel_channel_list(data::channel_map& result) const;

    void set_sel_stage(stage stg);
    stage get_sel_stage() const;

    criterion get_monpac_criterion(const object_id& =object_id()) const;
    void set_monpac_criterion(criterion const &, object_id const& =object_id());

    // selector
    void modify_selection(select_action, node* = 0);
    void modify_selection(int mode, std::string fs = std::string(), node* = 0);
    void set_selection(hits::phitset);

    // location
    std::vector< location::Setup >
                    get_location_setup() const;
    void			set_location_setup(std::vector<location::Setup> &);
    location::Setup get_location(object_id const &id);

    unsigned        get_sel_location() const;
    void			set_sel_location(unsigned);
    slot			&location_slot();


public:

    void set_serial_mode(int);
    void serialization(serl::archiver &ar);

public:

    pnode get_root_node() const;
    pnode get_active_node() const;

    int get_process_time() const;

    shared_ptr<grader> get_active_node_by_location(object_id id) const;
    pnode get_node_by_name(std::string) const;

    bool get_active_channels(data::channel_map& result) const;

    void apply_delete_selection();
    std::string make_channel_filter();

    shared_ptr<grader> create_temp_grader(pnode=pnode());

    void stop_all();

    void create_new();

private:

    friend class _thread_proc;

private:

    // monpak
    std::map<object_id, criterion> criterions_;
    shared_ptr<criterion> criterion_;


    std::vector< location::Setup > location_setups_;
    struct location_pack
    {
        shared_ptr<locator> locator;
        shared_ptr<grader> grader;
    };
    std::map<object_id, location_pack> location_nodes_;

    shared_ptr<loader> loader_;
    shared_ptr<sorter> sorter_;
    shared_ptr<inspector> inspector_;
    shared_ptr<modifier> modifier_;
    shared_ptr<filter> filter_;
    shared_ptr<selector> selector_;

    shared_ptr<grader> grader_;

    std::vector<shared_ptr<node> > nodes_;

    mutable boost::xtime start_time, end_time;

    int       serial_mode_;
    int				sel_location_;

    slot			location_slot_;
};


typedef shared_ptr<process::suspend_lock> factory_lock;

inline factory_impl &factory()
{
    return *factory_impl::instance();
}

}

