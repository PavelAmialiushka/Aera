#pragma once

#include "data/Syntaxer.h"
#include "data/hitset.h"

#include "processes.h"
#include "select_action.h"

using boost::shared_ptr;

//////////////////////////////////////////////////////////////////////////

/**
 * @brief временная нода, которая подборку хитов пометки их как "выбранных".
 *
 * Нода запускается выполняет перебор всех хитов добавляет/удаляет их
 * к выделению, а после обработки удаляет себя в on_remove
 */

MAKE_SHARED_STRUCT(selection_result);

struct selection_result : process::rslt
{
    shared_ptr<hits::hitset>	selection;
};

///////////////////////////////////////////////////////////////////////////////

class selection_maker
        : public process::host
        , public process::host_listener

{
public:

    selection_maker(select_action a);
    ~selection_maker();

    void setup(process::hostsetup&);
    process::processor* create_processor();

    virtual void safe_on_finish(process::prslt);

private:

    class processor;
    struct config : process::config
    {
        select_action action;       
    } config_;
};
