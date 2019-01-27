#pragma once

#include "element.h"

namespace serl
{

class text
        : public element_factory
{
public:
    text(const std::string &);
    text(std::string &);
    ~text();

    // virtual
    pelement root_node() const;

    void do_save_data();

private:
    void load_text(std::string&, pelement);
    void save_text(std::string&, pelement, int level = 0);


private:
    pelement     root_;
    std::string *result_;
    std::string  source_;
};

}
