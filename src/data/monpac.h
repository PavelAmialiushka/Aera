//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "data_fwrd.h"
#include "node.h"
#include "stage.h"

#include "criterion.h"
#include "monpac_result.h"

namespace monpac
{

using data::pslice;

class channel;
class calculator;
class outputter;
//////////////////////////////////////////////////////////////////////////

MAKE_SHARED_STRUCT(monpac_result);

struct monpac_result : process::rslt
{
    // results
    std::vector<shared_ptr<calculator> > channels;
    std::vector<channel> ch_info_;
    criterion            criterion_;
};

class classifier
    : public process::host
{
public:

    classifier(criterion const &);
    ~classifier();

    //////////////////////////////////////////////////////////////////////////

    criterion get_criterion() const;

private:
    virtual void setup(process::hostsetup&);
    virtual process::processor* create_processor();

    void restart();

private:

    struct config : process::config
    {
        criterion criterion_;
    } config_;

    class processor;
    friend class processor;

};

}
