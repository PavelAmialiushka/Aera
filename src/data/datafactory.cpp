#include "stdafx.h"

#include "data/datafactory.h"

#include "data/dta_loader.h"
#include "data/naf_loader.h"

namespace data
{
//////////////////////////////////////////////////////////////////////////

shared_ptr<loader> factory::get_loader(std::string path)
{
    shared_ptr<dta_loader> dta_file;
    dta_file.reset(new dta_loader(path));
    if (dta_file->check_file_format())
        return dta_file;

    shared_ptr<naf_loader> naf_file;
    naf_file.reset( new naf_loader(path));
    if (naf_file->check_file_format())
        return naf_file;

    return dta_file;
}

void loader::init(data::pwriter_t)
{
}

}
