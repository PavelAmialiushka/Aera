#pragma once

#include "node.h"
#include "aecollection.h"
#include "blank_slice.h"
#include "aeslice.h"
#include "tddslice.h"

#include "loader.h"
#include "locator.h"

#include "utilites/unittest.h"

namespace nodes
{

class fake_node : public node
{

public:
    struct fake_node_config : process::config
    {
        data::ae_collection collection_;
        data::tdd_collection tdd_collection_;
    } config_;

    // время, канал, райзтайм
    fake_node(double* record, int size)        
    {        
        aera::chars chars[]= {
            aera::C_Time,
            aera::C_Channel,
            aera::C_Amplitude,
            aera::C_Risetime
        };
        config_.collection_.set_typestring(chars, SIZEOF(chars));
        while( size > 0 )
        {
            config_.collection_.copy_append_record(record);
            record += SIZEOF(chars);
            size -= SIZEOF(chars);
        }
        restart_using(config_);
    }

    ~fake_node()
    {
        detach_host();
    }



    struct processor
            : process::processor_t<
                fake_node_config,
                nodes::result,
                nodes::result>
    {
        virtual bool process()
        {
            result->ae.reset( new data::ae_slice(&config->collection_) );
            result->tdd.reset( new data::tdd_slice(&config->tdd_collection_) );
            result->raw.reset( new data::blank_slice );

            return true;
        }
    };


    void setup(process::hostsetup &s)
    {
        s.need_parent = 0;
    }

    process::processor *create_processor()
    {
        return new processor;
    }

    static
    void append_dt(std::vector<double>& v, double ch, double t, double dt, double rt = 0.0)
    {
        v.push_back(t + dt / 1e6 );
        v.push_back(ch);
        v.push_back(50);
        v.push_back(rt);
    }
};


class fake_factory
{
    struct event
    {
        const char* where;
        int    level;
        int    basetime;
        double x, y;
    };

    class fake_event
    {
        fake_factory *self;
        double        basetime;
    public:
        fake_event(fake_factory* factory, double time)
            : self(factory)
            , basetime(time)
        {
        }

        fake_event& at(int channel, double dt)
        {
            fake_node::append_dt(self->data_, channel, basetime, dt);
            return *this;
        }

        fake_event& ensure_none()
        {
            event& s = self->signaldata_.back();
            s.level = 0;
            return *this;
        }

        fake_event& ensure(double x)
        {
            event& s = self->signaldata_.back();
            s.level = 1;
            s.x = x;
            return *this;
        }

        fake_event& ensure(double x, double y)
        {
            event& s = self->signaldata_.back();
            s.level = 2;
            s.x = x;
            s.y = y;
            return *this;
        }

    };
    friend class fake_event;

    std::vector<event> signaldata_;
    std::vector<double> data_;

    location::ZonalSetup* setup_;
    int basetime;
public:

    fake_factory(location::ZonalSetup* loc)
        : setup_(loc)
        , basetime(0)
    {
    }

#define imitate_event() imitate_event_(HERE)

    fake_event imitate_event_( const char* loc)
    {
        event s;
        s.basetime = basetime++;
        s.where = loc;
        s.level = 0;
        signaldata_.push_back(s);

        return fake_event(this, s.basetime);
    }


    shared_ptr<fake_node> fakenode_;
    shared_ptr<locator> locator_;

    void prepare_nodes()
    {
        fakenode_.reset( new fake_node(&data_[0], data_.size()) );

        locator_.reset( new locator );
        location::Setup setup(setup_);
        locator_->set_location(setup);

        locator_->set_parent( fakenode_.get() );
    }

    void run()
    {
        prepare_nodes();
        analize_result();
    }

    void analize_result()
    {
        nodes::presult r = process::get_result<nodes::result>(locator_);

        pslice slice=r->ae;
        int slicesize = slice->size();

        int sliceindex = 0;
        for(int index = 0; index < signaldata_.size(); ++index)
        {
            event s = signaldata_[index];

            if (s.level == 0)
            {
                // не должно быть результата
                if (sliceindex >= slicesize)
                {
                    // ok
                }
                else {
                    double time = slice->get_value(s.basetime);
                    UnitTest::Test::assertTrueMessageHere(
                                fabs(time - s.basetime)>1e-3,
                                "expected no event, but event is registered",
                                s.where);
                }

            } else
            {
                UnitTest::Test::assertTrueMessageHere(
                            sliceindex < slicesize,
                            "expected event, but event hasnt been registered",
                            s.where);
                if (s.level == 1)
                {
                    double x = slice->get_value(s.basetime, aera::C_CoordX);
                    UnitTest::Test::assertTrueMessageHere(
                                fabs(x - s.x) < 1e-3,
                                strlib::strf("expected %.3f, but registered %.3f",
                                             s.x, x),
                                s.where);
                } else if (s.level == 2)
                {
                    double x = slice->get_value(s.basetime, aera::C_CoordX);
                    double y = slice->get_value(s.basetime, aera::C_CoordY);
                    UnitTest::Test::assertTrueMessageHere(
                                fabs(x - s.x) + fabs(y - s.y) < 1e-3,
                                strlib::strf("expected %.3f,%.3f, but registered %.3f,%.3f",
                                             s.x, s.y, x, y),
                                s.where);
                }

                ++sliceindex;
            }
        }
        UnitTest::Test::assertTrueMessageHere(
                    sliceindex >= slicesize,
                    strlib::strf("extra event registered after ending test (%d events registered)",
                                 slicesize),
                    signaldata_.back().where);
    }
};


}

