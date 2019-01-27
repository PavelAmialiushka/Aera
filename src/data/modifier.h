#pragma once

#include "data/node.h"
#include "data/datafactory.h"

#include "parametric.h"
//////////////////////////////////////////////////////////////////////////

namespace nodes
{

class modifier : public node
{
public:
    modifier();
    ~modifier();

    /**
     * @brief ¬озвращает перечень активных каналов
     * (тех каналов, по которым производилс€ сбор данных)
     */
    void set_parametric_data(parametric *data);
    void get_parametric_data(parametric *data);

    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void restart();

private:
    struct config : process::config
    {
        parametric       parametrics_[4];
    } config_;

    struct processor;

};

}
