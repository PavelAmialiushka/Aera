#pragma once

#include "utilites/singleton.h"
#include "utilites/foreach.hpp"

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/signals.hpp>

#include <boost/date_time/date.hpp>
#include <boost/date_time/time.hpp>
#include <boost/date_time/time_duration.hpp>
#include <boost/thread/condition.hpp>

#include <map>

namespace process
{

struct tick_timer
{
    LARGE_INTEGER counter;

    tick_timer()
    {
        reset();
    }

    DWORD elapsed() const
    {
        LARGE_INTEGER current;
        ::QueryPerformanceCounter(&current);

        return DWORD((current.QuadPart - counter.QuadPart) / 1000);
    }

    void reset()
    {
        LARGE_INTEGER current;
        ::QueryPerformanceCounter(&current);

        counter = current;
    }
};

///////////////////////////////////////////////////////////////////////////////

class host;

struct hostsetup
{
    int   need_parent;
    bool  need_config;
    bool  auto_delete;
    double weight;
    int   priority;
    std::string name;
};

///////////////////////////////////////////////////////////////////////////////

MAKE_SHARED(processor);

///////////////////////////////////////////////////////////////////////////////

MAKE_SHARED(config);
class config
{
public:
    virtual ~config() {}
};

///////////////////////////////////////////////////////////////////////////////

MAKE_SHARED(rslt);
class rslt
{
public:
    std::vector<prslt> cache;
    pconfig            config;

public:
    virtual ~rslt() {}

    virtual void configure_from(pconfig);
    virtual void inherite_from(prslt);
};

///////////////////////////////////////////////////////////////////////////////

class host_listener
{
public:
    virtual void unsafe_on_restart() {}
    virtual void unsafe_on_finish() {}

    virtual void safe_on_restart() {}
    virtual void safe_on_finish(prslt) {}
};

///////////////////////////////////////////////////////////////////////////////



MAKE_SHARED_STRUCT(fiber_info);
struct fiber_info
{
    boost::function1<void, pfiber_info> fiber;
    int     state;
    double  progress;
    processor *proc;
    tick_timer timer;
};

///////////////////////////////////////////////////////////////////////////////

class server
        : public utils::singleton<server, 14>
{
    friend class processor;

    // все данные, которые необходимы для выполнения задачи

    struct exeblock
    {
        host*      host;
        int        execution_id;
        pconfig    config;
        std::vector<prslt> sources;
        pprocessor proc;
    };

    struct hostdata
            : hostsetup
    {
        host*      self;
        std::vector<host*> parents;
        std::vector<host*> children;

        double     progress;
        int        pause_count;
        bool       visibility;

        std::map<int, int> states;

        int _state() const;
        int& _state();
        void _drop_state(int id);

        pconfig    config;
        prslt      result;

        int        estimate_time;
        double     estimate_perc;

        std::vector<process::host_listener*>     listeners;

        int    execution_id;
        std::vector<exeblock> executors;
        std::vector<function0<void> > callbacks;
    };

    static bool byState(hostdata const&, hostdata const&);
    static bool byPriority(hostdata *, hostdata*);

    bool check_status(exeblock& exe, int index, int size);
    bool _check_status(exeblock& exe, int index, int size);

    hostdata* _try_to_find_host(host* self) const;
    hostdata& _find_host(host* self);
    void _try_to_restart(host *self);
    bool _select_fiber(pfiber_info& fiber, process::processor* selproc=0);
    bool _select_executor(exeblock& exe);
    void _thread_function();
    void _attach_child_to_parent(host* child, host* parent, bool override=true);
    void _detach_child_from_parent(host* child);

    void _execute_exeblock(exeblock exe, boost::mutex::scoped_lock &lock);
    void _execute_fiber(pfiber_info info, boost::mutex::scoped_lock &lock);

    bool _is_visible(host* child) const;

    double _get_progress(int &loc, std::string& stat) const;
    void _collect_callbacks(std::vector<function0<void> > &v);
    void _collect_hosts_to_remove(std::vector<host*> &r);

    void _change_pause_count(host*, int delta);



    void create_fiber(processor*, pfiber_info);
    bool join_fibers(processor*);

public:

    server();
    ~server();

public:

    // фукнции, которые запускаются только из главного потока
    void restart_procedure(host* self, pconfig cfg);
    void set_parent(host* self, host* parent);
    void add_parent(host* self, host* parent);
    void detach_host(host*);
    prslt get_result(const host*, bool wait = true);
    bool is_ready(const host* h);

    void set_visibility(host*, bool);

    void resume(host*);
    void pause(host*);

    void attach_on_restart(host* self, boost::function0<void> f);
    void attach_on_finish(host* self, boost::function1<void, prslt> f);
    void attach_listener(host* self, host_listener* l);
    void detach_listener(host* self, host_listener* l);

    double get_progress();
    double get_progress(int &loc, std::string& stat);
    bool is_failure() const;
    std::string take_failure();
    void register_failure(const char* msg);

    void add_signal_update_progress(function0<void> fn, int id);

private:

    std::vector<shared_ptr<boost::thread> > threads_;

    mutable boost::mutex mutex_;
    int     main_thread_id_;
    volatile bool interrupted_;
    bool    process_failure_;
    bool    low_priority_started_;
    boost::array<char, 1024> failure_description_;
    std::set<pprocessor> active_processors_;

    std::map<host*, hostdata> hosts_;

    tick_timer           update_timer;
    boost::signal0<void> update_progress;

    boost::condition    some_task_created_;
    boost::condition    some_processor_ended_;
    boost::condition    some_fiber_ended_;
};

///////////////////////////////////////////////////////////////////////////////

class processor
{
    friend class server;
    // обратка для вызова проверки статуса
    boost::function2<bool, int, int> check_status_f;
    boost::function2<bool, int, int> check_status_nolock_f;

protected:
    tick_timer   timer;

    // нужен для оптимизации скорости
    int          hidden_counter;

public:

    std::vector<pfiber_info> fibers;
    int     thread_count;
    std::string status_helper;

    host*   host_;

public:
    virtual ~processor() {}

    bool check_status(int index, int size);
    bool check_status(int index, int size, int range, int range_count);

    bool check_status(pfiber_info info, int index, int begin, int end);

    void create_fiber(boost::function1<void, pfiber_info>);
    bool join_fibers();

    virtual
    bool do_process(pconfig, std::vector<prslt> const&, prslt&) = 0;
};

///////////////////////////////////////////////////////////////////////////////

template <class Tcfg, class Tout, class Tin, class Tin2 = rslt>
class processor_t : public processor
{
    typedef shared_ptr<Tcfg> config_type;
    typedef shared_ptr<Tin>  source_type;
    typedef shared_ptr<Tin2> source2_type;
    typedef shared_ptr<Tout> result_type;

    virtual bool process()=0;
protected:
    config_type config;
    source_type source;
    source2_type source2;
    result_type result;

    virtual
    bool do_process(pconfig raw_config, std::vector<prslt> const& raw_source, prslt &raw_result)
    {        
        config = boost::dynamic_pointer_cast<Tcfg>(raw_config);
        if (raw_config && !config )
        {
            assert(!"incorrect template parameters");
            return false;
        }

        if (raw_source.size())
        {
            source = boost::dynamic_pointer_cast<Tin>(raw_source[0]);
            if (!source)
            {
                assert(!"incorrect template parameters");
                return false;
            }

            if (raw_source.size() > 1)
            {
                source2 = boost::dynamic_pointer_cast<Tin2>(raw_source[1]);
                if (!source2)
                {
                    assert(!"incorrect template parameters");
                    return false;
                }
            }
        }

        result.reset( new Tout );
        result->configure_from( config );
        if (source)
            result->inherite_from( source );

        timer.reset();

        bool r = process();
        raw_result = result;

        return r;
    }
};

///////////////////////////////////////////////////////////////////////////////

class host
{
    friend class server;

    bool detached;

public:

    host()
        : detached(false)
    {
    }

    virtual ~host()
    {
        assert(detached);
    }


    // запуск
    void restart_using(pconfig p)
    {
        server::instance()->restart_procedure(this, p);
    }

    template<class configT>
    void restart_using(configT const& t)
    {
        restart_using(pconfig(new configT(t)));
    }

    void detach_host()
    {
        assert(!detached);
        server::instance()->detach_host(this);
    }

    void attach_listener(host_listener* ls)
    {
        server::instance()->attach_listener(this, ls);
    }

    void detach_listener(host_listener* ls)
    {
        server::instance()->detach_listener(this, ls);
    }

    void set_parent(host* parent)
    {
        server::instance()->set_parent(this, parent);
    }

    void add_parent(host* parent)
    {
        server::instance()->add_parent(this, parent);
    }

    prslt get_result_(bool wait = true) const
    {
        return server::instance()->get_result(this, wait);
    }

    bool is_ready() const
    {
        return server::instance()->is_ready(this);
    }

    void set_visibility(bool v)
    {
        server::instance()->set_visibility(this, v);
    }

    std::string name()const;

    virtual void setup(hostsetup&)=0;
    virtual processor* create_processor()=0;

private:
    void get_parent() const {
        // нельзя вовращать родителя, поскольку неизвестно его время жизни
    }

};

template<class T>
shared_ptr<T> get_result(const host* h, bool wait = true, T* =0)
{
    return boost::dynamic_pointer_cast<T>(h->get_result_(wait));
}

template<class T, class U>
shared_ptr<T> get_result(shared_ptr<U> h, bool wait = true, T* =0)
{
    return get_result<T>(h.get(), wait, 0);
}

template<class T>
shared_ptr<T> get_parent_result(const host *h, bool wait = true, T* =0)
{
    prslt r0 = h->get_result_(wait);
    if (!r0) return shared_ptr<T>();

    std::deque<prslt> deq;
    deq.push_back(r0);
    while(deq.size())
    {
        r0 = deq.front(); deq.pop_front();

        shared_ptr<T> r = boost::dynamic_pointer_cast<T>(r0);
        if (r) return r;

        foreach(prslt x0, r0->cache)
        {
            deq.push_back(x0);
        }
    };

    return shared_ptr<T>();
}

template<class T, class U>
shared_ptr<T> get_parent_result(shared_ptr<U> h, bool wait = true, T* =0)
{
    return get_parent_result<T>(h.get(), wait);
}

template<class R> class host_t : public host
{
    typename R result;
public:
    shared_ptr<R> get_result(bool wait = true)
    {
        return process::get_result<R>(this, wait);
    }
};

///////////////////////////////////////////////////////////////////////////////

class suspend_lock
{
    host* self;
public:
    suspend_lock(host* s)
        : self(s)
    {
        server::instance()->pause(self);
    }

    template<class T>
    suspend_lock(shared_ptr<T> p)
        : self(p.get())
    {
        server::instance()->pause(self);
    }

    ~suspend_lock()
    {
        server::instance()->resume(self);
    }
};




}
