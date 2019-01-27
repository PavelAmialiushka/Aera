#include "StdAfx.h"

#include "data/processes.h"

//////////////////////////////////////////////////////////////////////////

#include "utilites\unittest.h"

//////////////////////////////////////////////////////////////////////////

//using namespace process;


// sample classes

class sample_host
        : public process::host
{
public:
    // струстура, которая хранит настройки
    struct sample_cfg : process::config
    {
        int  counter;
    } p;

    struct sample_data : process::rslt
    {
        int value;
    };

    struct p1 : process::processor_t<sample_cfg, sample_data, sample_data>
    {
        bool process()
        {
            int value = source ? source->value : 0;
            for(int index=0; index < config->counter; ++index)
            {
                ++value;
                if (!check_status(index, config->counter))
                    return false;
            }

            result->value = value;
            return true;
        }
    };

    shared_ptr<sample_data> get_result(bool wait=true)
    {
        return boost::dynamic_pointer_cast<sample_data>(host::get_result_(wait));
    }

    int need_parent;
    void setup(process::hostsetup& s)
    {
        s.name = "test";
        s.auto_delete = false;
        s.need_parent = need_parent;
    }

    process::processor* create_processor()
    {
        return new p1;
    }

    void restart()
    {
        restart_using(p);
    }

public:
    void set_int(int ix)
    {
        p.counter = ix;

        restart();
    }

    sample_host(int np = 0)
    {
        need_parent = np;
    }

    ~sample_host()
    {
        detach_host();
    }
};

class UNITTEST_SUITE( procedures_tests )
{
public:
    UNITTEST( start1 )
    {
        sample_host s1;
        s1.set_int(10);

        shared_ptr<sample_host::sample_data> result = s1.get_result();
        assertTrue(!!result);
        assertEqual(result->value, 10);
    }

    UNITTEST( start2 )
    {
        sample_host s1;
        s1.set_int(10);

        sample_host s2(1);
        s2.set_int(24);

        shared_ptr<sample_host::sample_data> result = s1.get_result();
        assertTrue(!!result);
        assertEqual(result->value, 10);

        s2.set_parent(&s1);
        shared_ptr<sample_host::sample_data> result3 = s2.get_result();
        assertTrue(!!result3);
        assertEqual(result3->value, 34);
    }

} INSTANCE;
//////////////////////////////////////////////////////////////////////////



