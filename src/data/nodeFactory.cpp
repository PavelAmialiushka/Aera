#include "StdAfx.h"

#include "nodefactory.h"

#include "loader.h"
#include "sorter.h"
#include "inspector.h"
#include "modifier.h"
#include "selector.h"
#include "filter.h"
#include "locator.h"
#include "grader.h"

#include "stage.h"
#include "criterion.h"

#include "LocationSetup.h"
#include "LocationUnit.h"
#include "ZonalSetup.h"

#include "parametric.h"

#include "selection_maker.h"

#include "utilites/document.h"

#include <boost/format.hpp>

namespace nodes
{

//////////////////////////////////////////////////////////////////////////
class join
{
public:
    join(std::string sep)
        : separator_(sep)
    {}

    join(std::string str, std::string sep)
        : separator_(sep), self_(str)
    {}

    join operator,(std::string other) const
    {
        return join(
                   self_.empty()
                   ? other
                   : other.empty()
                   ? self_
                   : separator_.empty()
                   ? separator_
                   : self_+separator_+other,
                   separator_);
    }

    operator std::string() const
    {
        return self_;
    }

private:
    std::string self_;
    std::string separator_;
};


pnode factory_impl::get_node_by_name(std::string node) const
{
    if (node=="loader")	return loader_;
    if (node=="inspector") return inspector_;
    if (node=="sorter")	return sorter_;
    if (node=="modifier") return modifier_;
    if (node=="filter")	return filter_;
    if (node=="selector") return selector_;
    if (node=="grader")	return grader_;
    if (node=="locator")
    {
        if (!location_setups_.size())
        {
            factory_impl* self = const_cast<factory_impl*>(this);
            self->location_setups_.push_back( location::Setup::create(location::locZonal) );
            self->set_location_setup(self->location_setups_);
        }
        object_id id = location_setups_.front().get_id();

        std::map<object_id, location_pack>::const_iterator iter
                = location_nodes_.find(id);
        if (iter != location_nodes_.end())
        {
            return iter->second.grader;
        }
        assert(!"location not found");
        return grader_;
    }

    assert(!"name not found");
    return pnode();
}

double factory_impl::get_progress(int &loc, std::string& stat) const
{
    return process::server::instance()->get_progress(loc, stat);
}

//////////////////////////////////////////////////////////////////////////

factory_impl::factory_impl()
    : criterion_(new criterion),
      serial_mode_(ordinary_mode),
      sel_location_((unsigned)-1)
{
    // сначала выполняется загрузка данных
    loader_.reset( new loader );
    nodes_.push_back( loader_ );

    // определение границ
    //
    inspector_.reset( new inspector );
    inspector_->set_parent( nodes_.back().get() );
    // добавим его чуть позже

    // сортировка по времени
    //
    sorter_.reset( new sorter );
    sorter_->set_parent( nodes_.back().get() );
    nodes_.push_back( sorter_ );

    // модификация записек (параметрика)
    //
    modifier_.reset( new modifier );
    modifier_->set_parent( nodes_.back().get() );
    nodes_.push_back( modifier_ );

    // отбрасываем удаленные хиты
    //
    filter_.reset( new filter );
    filter_->set_parent( nodes_.back().get() );
    nodes_.push_back( filter_ );

    // добавляем выделение
    selector_.reset( new selector);
    selector_->set_parent( nodes_.back().get() );
    selector_->add_parent( inspector_.get() );
    nodes_.push_back( inspector_ );
    nodes_.push_back( selector_ );

    // выбираем режим просмотра
    //
    grader_.reset( new grader );
    grader_->set_parent( filter_.get() );
    grader_->add_parent( selector_.get() );
    nodes_.push_back( grader_ );
}

factory_impl::~factory_impl()
{
}

double factory_impl::get_progress() const
{
    int x; std::string s;
    return get_progress(x, s);
}

pnode factory_impl::get_root_node() const
{
    return nodes_.front();
}

pnode factory_impl::get_active_node() const
{
    return nodes_.back();
}

shared_ptr<grader> factory_impl::get_active_node_by_location(object_id id) const
{
    if (id.empty())
        return grader_;

    std::map<object_id, location_pack>::const_iterator iter = location_nodes_.find(id);
    if (iter != location_nodes_.end())
    {
        location_pack pack = iter->second;
        return pack.grader;
    }

    //assert(!"WTF!: no such location");
    return grader_;
}

void factory_impl::reload()
{
    boost::xtime_get(&start_time, 1);
    end_time = start_time;
    loader_->restart();
}

void factory_impl::open(std::string filename)
{
    boost::xtime_get(&start_time, 1);
    end_time = start_time;
    loader_->open(filename);
}

int factory_impl::get_process_time() const
{
    if (get_progress()<1) return 0;

    if (boost::xtime_cmp(start_time, end_time) == 0)
    {
        boost::xtime_get(&end_time, 1);
    }

    return (end_time.sec - start_time.sec) * 1000
         + (end_time.nsec - start_time.nsec) / 1000000;
}

void factory_impl::set_filter(std::string fstr)
{
    filter_->add_filter( fstr );
}

std::string factory_impl::get_filter() const
{
    assert(!"not implemented");
    return "";
}

void factory_impl::set_channel_sheme(channel_sheme const &sheme)
{
    grader_->set_channel_sheme(sheme);
}

void factory_impl::get_channel_sheme(channel_sheme &sheme) const
{
    grader_->get_channel_sheme(sheme);
}

void factory_impl::get_sel_channel_list(data::channel_map& result) const
{
    grader_->get_channel_list(result);
}

bool factory_impl::get_active_channels(data::channel_map& result) const
{
    nodes::presult r = loader_->get_result(false);
    if (!r) return false;

    result = *r->working_channels;
    return true;
}

void factory_impl::set_parametric_data(parametric *params)
{
    modifier_->set_parametric_data(params);
}

void factory_impl::get_parametric_data(parametric *params)
{
    modifier_->get_parametric_data(params);
}

void factory_impl::reset_filter()
{
    filter_->clear();
}

void factory_impl::set_monpac_criterion(const criterion &crit, object_id const &id)
{
    if (id==object_id()) criterion_=shared_ptr<criterion>(new criterion(crit));
    else
    {
        criterions_[id]=crit;//.insert( std::make_pair(id, crit) );
        criterion test=criterions_[id];
    }

    Document().SetModified();
}


void factory_impl::modify_selection(select_action a, node *nd)
{
    selection_maker *sel = new selection_maker(a);

    if (!nd) nd = get_active_node().get();
    assert(dynamic_cast<grader*>(nd));

    sel->set_parent(nd);
}

void factory_impl::modify_selection(int mode, std::string fs, node *nd)
{
    modify_selection(select_action(mode, fs), nd);
}

void factory_impl::set_selection(hits::phitset selection)
{
    selector_->set_selection( selection );
}

criterion factory_impl::get_monpac_criterion(object_id const &id) const
{
    std::map<object_id, criterion>::const_iterator iter
        =criterions_.find(id);

    if (criterions_.end()!=iter)
    {
        criterion temp=iter->second;
        return temp;
    }
    else if (criterion_)
    {
        return *criterion_;
    }
    else
    {
        return criterion();
    }
}

std::string factory_impl::make_channel_filter()
{
    LOG("make channel filter");

    std::string chlst;

    boost::array<bool, 256> active;
    get_active_channels(active);

    boost::array<bool, 256> selected;
    get_sel_channel_list(selected);

    std::vector<int> channels;
    for (unsigned index=0; index<active.size(); ++index)
        if (active[index])
            channels.push_back(index);

    for (unsigned index=0; index<channels.size(); ++index)
    {
        if (selected[channels[index]])
        {
            unsigned yes=index;
            for (; yes<channels.size() && selected[channels[yes]]; ++yes);
            if (!chlst.empty()) chlst+=" || ";
            if (yes-index==1)
            {
                chlst += (boost::format("(Ch==%s)") % (channels[index]+1)).str();
            }
            else
            {
                bool a= index!=0;
                bool b= yes!=channels.size();
                if (a && b) chlst += "(";
                if (a) chlst += (boost::format("Ch>=%s") % (channels[index]+1)).str();
                if (a && b) chlst += " && ";
                if (b) chlst += (boost::format("Ch<=%s") % (channels[yes-1]+1)).str();
                if (a && b) chlst += ")";
            }
            index=yes-1;
        }
    }
    if (!chlst.empty())
    {
        chlst.insert(0, "(");
        chlst+=")";
    }
    return chlst;
}

shared_ptr<grader> factory_impl::create_temp_grader(pnode n)
{
    if (!n) n = get_node_by_name("locator");

    shared_ptr<nodes::grader> grader( new nodes::grader() );
    grader->set_parent( n.get() );
    grader->add_parent( inspector_.get() );

    return grader;
}


void factory_impl::stop_all()
{
    loader_->open("");
}

void factory_impl::apply_delete_selection()
{
    LOG("apply delete selection");

    nodes::presult result = selector_->get_result(true);
    filter_->append_set( result->selection );
    Document().SetModified();
}

void factory_impl::set_serial_mode(int mode)
{
    serial_mode_=mode;
}

struct old_zonal_location
{
    old_zonal_location()
        : def(3000, location::usec)
        , loc(0, location::usec)
        , vel(3, location::mm_us)
        , use_peak_time(0)
    {}

    location::unit_value def;
    location::unit_value loc;
    location::unit_value vel;
    bool                 use_peak_time;

    void serialization(serl::archiver &ar)
    {
        assert(ar.is_loading());

        double a;
        ar.serial("lockout",  a);
        def=location::unit_value(a, location::sec);

        ar.serial("2edt",  def);
        ar.serial("2loc",  loc);
        ar.serial("2vel",  vel);

        ar.serial("peak_time", use_peak_time);
    }
};

static void old_fashion_serial(serl::archiver &ar, std::vector<location::Setup> &vector)
{
    using namespace location;

    std::vector<shared_ptr<old_zonal_location> > buffer;
    ar.serial_static_ptr_container("locations", buffer);
    foreach(shared_ptr<old_zonal_location> old, buffer)
    {
        Setup setup=Setup::create(locZonal);
        if (ZonalSetup *zonal=setup.zonal())
        {
            zonal->set_definition_time(old->def);
            zonal->set_lockout(old->loc);
            zonal->set_use_peak_time(old->use_peak_time);
            zonal->set_velocity(old->vel);
        }

        vector.push_back(setup);
    }
}

shared_ptr< process::suspend_lock > factory_impl::block_execution()
{
    shared_ptr< process::suspend_lock > lock;
    lock.reset( new process::suspend_lock(loader_.get()));

    return lock;
}

void factory_impl::serialization(serl::archiver &ar)
{
    LOG("factory_impl::serialization");
    try
    {
        if (serial_mode_==ordinary_mode)
        {
            if (ar.is_loading())
            {
                std::string tmpf;
                ar.serial("filter", tmpf);
                set_filter(tmpf);

                filter_->clear();
            }
            ar.serial("filter-more", *filter_);

            ar.serial("stages", *stages::instance());

            stage stg;
            if (ar.is_saving()) stg=get_sel_stage();
            ar.serial("current_stage", stg);
            if (ar.is_loading()) set_sel_stage(stg);

            ar.serial("monpac-ex", serl::make_container(criterions_));
        }

        // настройки локализации
        // TODO
        std::vector< location::Setup > tmp;
        if (ar.is_saving()) tmp=get_location_setup();
        ar.serial_container("location_setup", tmp);
        if (ar.is_loading())
        {
            if (tmp.empty()) old_fashion_serial(ar, tmp);
            set_location_setup( tmp );
        }

        ar.serial("locator",
                  serl::indirect(this, unsigned(),
                                 &factory_impl::set_sel_location,
                                 &factory_impl::get_sel_location));

        // настройка по-умолчанию
        ar.serial_static_ptr("monpac", criterion_);

        // создание нового файла
        std::vector<parametric> values(4);
        get_parametric_data(&values[0]);
        ar.serial_container("parametric", values);
        if (ar.is_loading())
        {
            set_parametric_data(&values[0]);
        }
    } LogExceptionPath("factory_impl::serialization");

    serial_mode_=ordinary_mode;
    CMainDocument::instance()->GetDocument()->SetModified(false);
}

void factory_impl::set_sel_stage(stage stg)
{
    LOG("set sel stage");

    grader_->set_stage(stg);
    CMainDocument::instance()->GetDocument()->SetModified();
}

stage factory_impl::get_sel_stage() const
{
    return grader_->get_stage();
}

void factory_impl::create_new()
{
    LOG("create new file");

    set_sel_stage( stage() );
    reset_filter();

    criterion empty;
    empty.set_test_threshold(false);
    empty.set_vessel(0);
    empty.set_stage( stage() );

    set_monpac_criterion( empty );
    criterions_.clear();

    set_channel_sheme(channel_sheme::default_sheme());

    set_sel_location(0);
}

void factory_impl::set_location_setup(std::vector<location::Setup> &setups)
{
    LOG("set location setup");

    bool changed = false;
    if (setups.size() != location_setups_.size())
        changed = true;
    else
    {
        for(unsigned index=0; index < setups.size(); ++index)
        {
            if (setups[index]!=location_setups_[index])
                changed = true;
        }
    }

    if (!changed) return;

    std::set<object_id> oldies, newbies;

    std::map<object_id, location_pack>::iterator iter;
    for(iter = location_nodes_.begin(); iter != location_nodes_.end(); ++iter)
    {
        oldies.insert( iter->second.locator->get_location_id() );
    }

    if (setups.size()==0)
    {
        setups.insert(setups.begin(),
                      location::Setup::create(location::locZonal));
    }

    foreach(location::Setup s, setups)
    {
        newbies.insert( s.get_id() );

        location_pack pack;
        if (location_nodes_.find(s.get_id()) != location_nodes_.end())
        {
            pack = location_nodes_[s.get_id()];
            if (pack.locator->get_location() != s)
            {
                pack.locator->set_location(s);
            }
        }
        else
        {
            pack.locator.reset( new locator );
            pack.locator->set_location(s);
            pack.locator->set_parent( filter_.get() );

            pack.grader.reset( new grader );
            pack.grader->restart();
            pack.grader->set_parent( pack.locator.get() );
            pack.grader->add_parent( selector_.get() );
            location_nodes_[s.get_id()] = pack;
        }
    }

    // удаляем лишнее
    std::set<object_id> toRemove;
    std::set_difference(STL_II(oldies),
                        STL_II(newbies),
                        std::inserter(toRemove, toRemove.begin()));

    foreach(object_id id, toRemove)
        location_nodes_.erase(id);

    // устанавливаем номера локаций
    int max_number=0;
    foreach(Setup &loc, setups)
        max_number=std::max(loc.get_number(), max_number);
    foreach(Setup &loc, setups)
        if (!loc.get_number())
            loc.set_number(++max_number);

    location_setups_ = setups;

    // reset selector
    set_sel_location( get_sel_location() );

    Document().SetModified();
    location_slot().raise_signal();
}

std::vector< location::Setup > factory_impl::get_location_setup() const
{
    return location_setups_;
}

location::Setup factory_impl::get_location(object_id const &id)
{
    foreach(location::Setup s, location_setups_)
    {
        if (s.get_id() == id)
            return s;
    }

    return location::Setup();
}

unsigned factory_impl::get_sel_location() const
{
    return sel_location_;
}

void factory_impl::set_sel_location(unsigned sl)
{
    LOG("select location");

    std::vector< location::Setup > tmp=get_location_setup();

    object_id loc_id;

    sel_location_=sl;
    if (sl<tmp.size())
    {
        loc_id = tmp[sl].get_id();
        location_pack pack = location_nodes_[loc_id];
        if (pack.locator)
        {
            grader_->set_parent(pack.locator.get());
            grader_->add_parent(selector_.get());
        } else
        {
            grader_->set_parent(filter_.get());
            grader_->add_parent(selector_.get());
        }
    }
    else
    {
        grader_->set_parent(filter_.get());
        grader_->add_parent(selector_.get());
    }

    Document().SetModified();
    location_slot().raise_signal();
}

slot& factory_impl::location_slot()
{
    return location_slot_;
}

}

