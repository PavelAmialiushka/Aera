#include "StdAfx.h"

#include "data/node.h"
#include "data/loader.h"
#include "utilites/serl/Archive.h"

#include "data/dta_saver.h"
#include "data/parametric.h"

#include "data/read_access.h"

serl::mark_as_rtti_abstract<nodes::node> _node_;


namespace nodes
{

void node::save_data(std::string filename)
{
    presult r = get_result(false);
    pslice slice=
            r->raw->merge(
            r->ae->merge(
            r->tdd));

    data::dta_saver saver( filename );
    for (unsigned index=0; index<slice->size(); ++index)
    {
        saver.write(slice, index);
    }
}

void node::serialization(serl::archiver &ar)
{
}

void result::inherite_from(process::prslt parent)
{
    rslt::inherite_from(parent);

    if (presult p = boost::dynamic_pointer_cast<result>(parent))
    {
        ae = p->ae;
        ae_sub= p->ae_sub;
        tdd = p->tdd;
        raw = p->raw;
        working_channels = p->working_channels;
        selected_channels = p->selected_channels;
        selection = p->selection;
    }
}

presult result::create(process::prslt parent)
{
    result* r = new result;

    r->inherite_from(parent);
    return presult(r);
}

}
