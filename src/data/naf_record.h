#ifndef NAF_RECORD_H
#define NAF_RECORD_H

#pragma once

#include "data/collection_writer.h"
#include "data/traits.h"
#include "data/slice.h"

#include "data/hitset.h"
#include "utilites/streams.h"

namespace data
{

//////////////////////////////////////////////////////////////////////////
class naf_loader;
class naf_record
{
public:
    naf_record(class naf_loader *);
    //record(class dta_saver *);

    void read_buffer(std::istream& s, unsigned size);

    void interpret(data::pwriter_t writer);
    void save(pslice slice, unsigned index);
    void scan_for_basetime(fs::istream&);

public:
    void append_raw_record(data::pwriter_t writer, raw_info const& info);
    void interpret_hit_record_table(fs::istream&, data::pwriter_t writer);
    void interpret_tdd_record_table(fs::istream&, data::pwriter_t writer);

private:
    unsigned int   len_;
    unsigned int   id_;
    std::vector<char> buffer_;

    std::vector<aera::chars> naftypes_;
    std::vector<aera::chars> paramtypes_;

    hits::hitset loaded_records_;

    bool naftypes_initiated_;
    bool paramtypes_initiated_;

    __int64 basetime_ae_;
    __int64 basetime_tdd_;
    double prevtime_;

    naf_loader *ldr_;
    friend class naf_loader;

    friend std::istream &operator >>(std::istream &, naf_record &self);
};

}


#endif // NAF_RECORD_H
