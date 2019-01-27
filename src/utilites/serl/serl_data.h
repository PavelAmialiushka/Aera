#pragma once

#include "element.h"

namespace serl
{

class data : public element_factory
{
public:
    struct data_record;

    data(const std::string &src);
    data(std::string &src);
    ~data();

    pelement root_node() const;

    void do_save_data();

private:

    char* load_data(char* src, pelement);
    std::string save_data(pelement);

    void load_zipped_data(std::string src, pelement);
    void save_zipped_data(std::string &result, pelement);

private:
    pelement root_;
    std::string *result_;
    std::string  source_;
};

}
