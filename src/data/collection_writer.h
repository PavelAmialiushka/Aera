#pragma once

#include "data/collection.h"

namespace data
{

class collection_writer
{
public:
    collection_writer(collection *m);
    ~collection_writer();

    void clear();

    double *allocate_ae_record(unsigned sz);
    void append_ae_record(double *);

    double *allocate_tdd_record(unsigned sz);
    void append_tdd_record(double *);

    void append_raw_record(const char *begin, const char *end, const raw_info &inf);

    void set_typestring(const aera::chars *chars, unsigned count);
    void set_channel_count(unsigned c);
    void set_tdd_common(const aera::chars *chars, unsigned count);
    void set_tdd_channel(const aera::chars *chars, unsigned count);

private:
    collection *master_;
};

typedef shared_ptr<collection_writer> pwriter_t;

}
