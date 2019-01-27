#include "stdafx.h"

#include "serl/serl_data.h"

#include <numeric>

#ifdef _WIN64
#include "cryptopp70/cryptlib.h"
#include "cryptopp70/gzip.h"
#else
#include "cryptopp/cryptlib.h"
#include "cryptopp/gzip.h"
#endif

using namespace CryptoPP;

namespace serl
{

//////////////////////////////////////////////////////////////////////////

data::data(const std::string &src)
    : source_(src)
    , result_(0)
{
    root_.reset( new element("") );
    if (src.size())
        load_zipped_data(src, root_);
}

data::data(std::string &src)
    : result_(&src)
{
    root_.reset( new element("") );

    if (src.size())
        load_zipped_data(src, root_);
}

data::~data()
{
}

pelement data::root_node() const
{
    return root_;
}

void data::do_save_data()
{
    if (result_)
    {
        save_zipped_data(*result_, root_);
    }
}

/*
    Запись в формате

    4	Общая длина N0
    4	Длина	имени N
    N	Имя
    Остальная запись
*/

struct data::data_record
{
    int record_len;
    int name_len;
};

char* data::load_data(char* begin, pelement base)
{
    data_record *rec = (data_record*)begin;

    char* start = begin + sizeof(data_record);
    char* fin= begin + rec->record_len + 4;

    std::string name(start, start + rec->name_len);
    start += rec->name_len;

    pelement self = !name.empty()
            ? base->lookup(name)
            : base;

    while(start < fin)
    {
        start = load_data(start, self);
    }

    return fin;
}

std::string data::save_data(pelement element)
{
    std::vector<std::string> items;
    items.push_back( std::string("-", 8) );

    std::string name = element->name();
    items.push_back( name );

    for(unsigned index=0; index < element->size(); ++index)
    {
        std::string data = save_data(element->at(index));
        items.push_back(data);        
    }

    unsigned size = 0;
    for(unsigned index=0; index < items.size(); ++index)
        size += items[index].size();
    std::vector<char> buffer(size);

    char* pointer = &buffer[0];
    for(unsigned index=0; index < items.size(); ++index)
        pointer = std::copy(STL_II(items[index]), pointer);

    pointer = &buffer[0];
    data_record* record = (data_record*)pointer;
    record->record_len=buffer.size()-4;
    record->name_len=name.size();

    return std::string(pointer, pointer + buffer.size());
}

void data::load_zipped_data(std::string src, pelement element)
{
    std::string result;
    try
    {
        StringSource(src, true, new Gunzip( new StringSink(result)));
    }
    catch (Gunzip::Err &)
    {
        throw serl::error(HERE);
    }

    load_data( (char*)result.data(), element );
}

void data::save_zipped_data(std::string &result, pelement element)
{
    std::string src = save_data(element);
    StringSource(src, true, new Gzip( new StringSink(result)));
}

}
