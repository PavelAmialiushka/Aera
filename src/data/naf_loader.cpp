#include "stdafx.h"

#include "data/naf_loader.h"

#include "data/data_error.h"

namespace data
{

naf_loader::naf_loader(std::string path)
    : record_(this),
      size_(0),
      pos_(0),
      record_mode_(0), last_record_mode_(0),
      stream_(path)
{
    size_ = stream_.size();

    std::fill(STL_II(channels_), false);
}

bool naf_loader::check_file_format()
{
    unsigned size = stream_.read_32();
    unsigned type = stream_.read_32();

    stream_.seek(0);
    return size>0 && type<=2;
}

void naf_loader::get_active_channels(channel_map& result) const
{
    result = channels_;
}

void naf_loader::read_ae_block_title()
{
    record_mode_ = ae;
    record_count_ = stream_.read_32();
    record_size_ = stream_.read_32();
}

void naf_loader::read_ae_block(data::pwriter_t writer)
{
    if (!stream_.reserve(record_size_))
        return;

    record_.interpret_hit_record_table(
                stream_.extract(record_size_),
                writer);
    stream_.skip(record_size_);

    if (--record_count_ == 0)
        record_mode_ = base;
}

void naf_loader::read_tdd_block_title()
{
    record_mode_ = tdd;
    record_count_ = stream_.read_32();
    record_size_ = stream_.read_32();

    assert(record_count_ >= 0);
    assert(record_size_ == 20);
}

void naf_loader::read_tdd_block(pwriter_t writer)
{
    while(record_count_)
    {
        if (!stream_.reserve(record_size_))
            return;

        record_.interpret_tdd_record_table(stream_, writer);
        --record_count_;
    }
    record_mode_ = base;
}

void naf_loader::read_raw_block_title()
{
    assert(0);
}

void naf_loader::read_raw_block(pwriter_t writer)
{
    assert(0);
}

bool naf_loader::read(data::pwriter_t writer)
{
    switch(record_mode_){
    case base:
    {
        // считываем запись из потока
        if (record_.basetime_ae_ == 0)
            record_.scan_for_basetime(stream_);

        // если чтение не удалось, вероятно конец файла
        unsigned int length = stream_.read_32();
        if (!stream_)
            return false;

        unsigned int id = stream_.read_32();

        switch(id)
        {
        case 0: read_ae_block_title(); break;
        case 2: read_tdd_block_title(); break;
        case 3: read_raw_block_title(); break;
        default:
            // некорректный файл, а именно вместо
            // заголовка блока вставлена следующая строчка данных
            stream_.seek_back(8);
            record_count_=1;
            record_mode_ = last_record_mode_;
        }
        break;
    }
    case ae:
        last_record_mode_ = ae;
        read_ae_block(writer);
        break;
    case tdd:
        last_record_mode_ = tdd;
        read_tdd_block(writer);
        break;
    }

    if (stream_)
    {
        // запонимаем для определения процента прочитанного
        pos_=stream_.offset();
        return true;
    }
    else
        return false;
}

int naf_loader::get_progress(int* size) const
{
    if (size) *size = size_ ? size_ : 1 ;
    return size_ ? pos_ : 1;
}

}
