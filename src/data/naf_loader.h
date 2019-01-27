#ifndef NAFDATAFILE_H
#define NAFDATAFILE_H

#include "data/data_fwrd.h"
#include "data/collection.h"
#include "data/datafactory.h"
#include "data/collection_writer.h"

#include "data/naf_record.h"


#include "utilites/streams.h"

namespace data
{

class naf_loader : public loader
{
public:
    naf_loader(std::string path);
    bool check_file_format();

    virtual void get_active_channels(channel_map& result) const;
    virtual bool read(data::pwriter_t writer);
    virtual int get_progress(int* size) const;

    void read_ae_block_title();
    void read_ae_block(data::pwriter_t writer);

    void read_tdd_block_title();
    void read_tdd_block(data::pwriter_t writer);

    void read_raw_block_title();
    void read_raw_block(data::pwriter_t writer);

private:
    fs::istream stream_;
    naf_record  record_;
    unsigned size_;
    unsigned pos_;
    boost::array<bool, 256>     channels_;

    // что именно сейчас считывавается
    enum { base, ae, tdd, raw};
    int record_mode_;
    int last_record_mode_;

    // количество записей во в пакете
    int record_count_;

    // размер одной записи
    int record_size_;

    friend class naf_record;
};

}

#endif // NAFDATAFILE_H
