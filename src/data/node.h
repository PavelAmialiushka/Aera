#pragma once

#include "data/Traits.h"
#include "data/read_access.h"
//#include "data/nodeFactory.h"

#include "data/processes.h"

#include "collection.h"

#include "slice.h"
#include "hitset.h"

#include "data_fwrd.h"

namespace ae
{
class selection;
}

namespace nodes
{

using data::slice;
using data::pslice;
using data::pae_slice;

using data::pchannel_map;

using hits::phitset;

MAKE_SHARED(result);
class result
        : public process::rslt
{
    typedef process::rslt base;

public:
    pslice ae;
    pslice raw;
    pslice tdd;
    pslice ae_sub;

    phitset selection;
    pchannel_map working_channels;
    pchannel_map selected_channels;

public:

    void inherite_from(process::prslt parent);

public:
    static presult create(process::prslt parent);
    virtual ~result() {}
};

template<class T>
shared_ptr<T> search_result(process::prslt p, T* =0)
{
    for(unsigned index = 0; index < p->cache.size(); ++index)
    {
        process::prslt parent_result = p->cache[index];
        if (shared_ptr<T> u = boost::dynamic_pointer_cast<T>(parent_result))
        {
            return u;
        } else
        {
            shared_ptr<T> r = search_result<T>(parent_result);
            if (r) return r;
        }
    }

    return shared_ptr<T>();
}


/** ¬ыполн€ет обработку и предоставл€ет ае данные.
 *
 * »з наследников класса строитс€ цепочка обработки ae данных.
 * каждый из элементов цепочки берет данные у своего родител€
 * и измен€ет их - фильтрует, сортирует, создает новые данны и
 * т.п.
 */

class node
        : public process::host_t<nodes::result>
{
public:
    typedef shared_ptr<node> pnode_t;

public:

    /**
     * сохран€ет данные в файл
     */
    void save_data(std::string filename);

    void serialization(serl::archiver &ar);

};

}
