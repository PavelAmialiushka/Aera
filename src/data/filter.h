#pragma once

#include "data_fwrd.h"

#include "data/node.h"
#include "data/stage.h"


#include "data/hitset.h"

namespace nodes
{

struct filter_item
{
    std::string fstring;
    phitset     fset;
};

struct filter_result;
typedef shared_ptr<filter_result> pfilter_result;
struct filter_result : nodes::result
{
    std::vector<filter_item> filters;

    static pfilter_result create(presult parent);
};

class filter
    : public node
    , public serl::serializable
    , public process::host_listener
{
public:
    filter();
    ~filter();

    void operator=(const filter &other)
    {
        on_serial_create();
    }

    void clear();
    void add_filter(const std::string &filters);
    void append_set(phitset hs);
    std::vector<filter_item> get_filter_set() const;

    void set_filter_string(std::string s);
    void set_filter_set(const std::vector<filter_item> &);
    void set_negative_flag(bool neg);

    void on_serial_create();
    void serialization(serl::archiver &ar);

    void safe_on_finish(process::prslt);

public:
    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    presult get_result(bool wait=true);

    void restart();

private:

    struct config : process::config
    {
        // параметры фильтрации
        // используется между потоками
        std::vector<filter_item>  filters;
        bool                      negative;
    } config_;

    struct result;
    struct processor;
};

//////////////////////////////////////////////////////////////////////////

//class selector : public node
//{
//public:

//    selector()
//    {
//        create_job();
//    }

//    ~selector()
//    {
//        remove_job_and_wait();
//    }

//    ready_state process()
//    {
//        return thrd::ready;
//    }

//public: // node
//    void get_result(presult& r)
//    {
//        get_parent()->get_result(r);
//    }
//};

}

