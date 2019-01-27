#pragma once

#include "data/node.h"
#include "data/datafactory.h"

#include "data/syntaxer.h"
#include "data/stage.h"

#include "data/object_id.h"
#include "data/Collector.h"

//////////////////////////////////////////////////////////////////////////

#include "channel_sheme.h"

namespace nodes
{

struct grader_result
        : nodes::result
{
    shared_ptr<boost::array<char, 256> > active_channels;
    location::Setup               location;
};

class grader : public node
{
public:
    grader();
    ~grader();

public:

    void get_channel_list(data::channel_map&) const;

    void set_channel_sheme(channel_sheme const &sheme);
    void get_channel_sheme(channel_sheme &sheme) const;

    void set_stage(const stage &stg);
    stage get_stage() const;

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void restart();

protected:

    void _create_tree();

private:
    struct config : process::config
    {
        // параметры работы
        channel_sheme              sheme_;
        boost::array<bool, 256>    channels_;

        stage                      stage_;
    } config_;

    struct processor;

};

}
