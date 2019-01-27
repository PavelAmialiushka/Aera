#include "stdafx.h"

#include "data/collection_writer.h"

namespace data
{

collection_writer::collection_writer(collection *m)
    : master_(m)
{
}

collection_writer::~collection_writer()
{
    master_->write_unlock();
}

void collection_writer::clear()
{
    master_->ae_collection_.clear();
    master_->raw_collection_.clear();
    master_->tdd_collection_.clear();
}

void collection_writer::set_typestring(const aera::chars *chars, unsigned count)
{
    master_->ae_collection_.set_typestring(chars, count);
}

void collection_writer::set_channel_count(unsigned c)
{
    master_->tdd_collection_.set_channel_count(c);
}

void collection_writer::set_tdd_common(const aera::chars *chars, unsigned count)
{
    master_->tdd_collection_.set_common_types(chars, count);
}

void collection_writer::set_tdd_channel(const aera::chars *chars, unsigned count)
{
    master_->tdd_collection_.set_channel_types(chars, count);
}

void collection_writer::append_ae_record(double *record)
{
    master_->ae_collection_.append_allocated_record(record);
}

double *collection_writer::allocate_tdd_record(unsigned sz)
{
    return master_->tdd_collection_.allocate_record(sz);
}

double *collection_writer::allocate_ae_record(unsigned sz)
{
    return master_->ae_collection_.allocate_record(sz);
}

void collection_writer::append_raw_record(const char *begin, const char *end, const raw_info &inf)
{
    master_->raw_collection_.append_record(begin, end, inf);
}

void collection_writer::append_tdd_record(double *data)
{
    master_->tdd_collection_.append_allocated_record(data);
}



}
