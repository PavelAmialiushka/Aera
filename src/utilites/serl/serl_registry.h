#pragma once

#include "element.h"

namespace serl
{

class registry_node;

class registry
        : public element_factory
{    
public:

    registry(HKEY key, std::string path);
    ~registry();

    // virtual
    pelement root_node() const;

private:
    void do_save_data();

private:
    void save_group(HKEY base_key, std::string key_name, pelement element);
    void load_group(HKEY base_key, std::string key_name, pelement element);

private:
    HKEY        root_key_;
    std::string key_name_;
    pelement    root_;
};

}
