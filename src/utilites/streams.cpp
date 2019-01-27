#include "stdafx.h"

#include "streams.h"
#include "filesystem.h"

#include "boost/iostreams/device/mapped_file.hpp"

namespace fs
{

class file_buffer : public streambuffer
{
    unsigned file_size_;
    unsigned page_size_;
    boost::iostreams::mapped_file_source source_;
    boost::iostreams::mapped_file_params params_;
public:
    file_buffer(std::string path)
    {
        open(path);
    }

    void open(std::string path)
    {
        file_size_ = fs::get_filesize(path);

        unsigned block_alignment = source_.alignment();
        page_size_ = 0x100000; // мегабайт
        page_size_ = page_size_ - page_size_ % block_alignment;

        // не более размера файла
        page_size_ = std::min(page_size_, file_size_);

        params_.path = path;
        params_.length = page_size_;
        params_.mode = std::ios::in;
        params_.offset = 0;

        source_.open(params_);
    }

    unsigned size() const
    {
        return file_size_;
    }

    bool get_data(unsigned pos, stream_page& page)
    {
        if (pos >= file_size_)
        {
            page.base_offset = 0;
            page.base = page.begin = page.end = 0;
            return false;
        }

        // позиция начала страницы
        unsigned aligned_pos = pos - pos % source_.alignment();

        // смещение от начала страницы
        unsigned delta = pos - aligned_pos;

        // оставшийся размер страницы с учетом смещения
        unsigned page_size = std::min<unsigned>(source_.size(), file_size_ - aligned_pos);

        source_.close();

        params_.offset = aligned_pos;
        params_.length = page_size;
        source_ = boost::iostreams::mapped_file_source(params_);

        page.base_offset = aligned_pos;
        page.base = source_.data();
        page.begin = page.base + delta;
        page.end = page.base + page_size;

        return true;
    }

    bool get_more_data(unsigned need_size, stream_page& page)
    {        
        unsigned offset = page.base_offset + (page.begin - page.base);
        if (!get_data(offset, page))
            return false;
        if (page.end - page.begin < need_size)
            return false;
        return true;
    }

private:

};

bool istream::get_more_data(unsigned size)
{
    if (!buffer_)
        return false;

    buffer_->get_more_data(size, page_);
    return page_.size() >= size;

}

istream::istream(std::string path)
    : okstate_(true)
{
    buffer_.reset( new file_buffer(path) );
    buffer_->get_data(0, page_);
}

istream::istream(std::vector<char> const &vector)
    : okstate_(true)
{
    buffer_.reset();
    page_.base_offset = 0;

    if (vector.empty())
    {
        static char dummy[1];
        page_.begin = dummy;
        page_.base = dummy;
        page_.end = dummy;
    }
    else
    {
        page_.begin = &vector[0];
        page_.base = page_.begin;
        page_.end = page_.base + vector.size();
    }
}

istream::istream(const char *data, unsigned size)
    : okstate_(true)
{
    buffer_.reset();
    page_.base_offset = 0;
    page_.base = page_.begin = data;
    page_.end = page_.base + size;
}

void istream::seek(unsigned pos)
{    
    if (buffer_)
    {
        okstate_ = buffer_->get_data(pos, page_);
    } else
    {
        if ( pos < page_.size())
        {
            okstate_ = true;
            page_.begin = page_.base + pos;
        } else
            okstate_ = false;
    }
}

unsigned istream::size() const
{
    if (buffer_)
        return buffer_->size();
    return page_.size();
}

unsigned istream::offset() const
{
    return page_.base_offset + (page_.begin - page_.base);
}

void istream::seek_back(unsigned pos)
{
    assert( offset() >= pos );
    seek(offset() - pos);
}




}
