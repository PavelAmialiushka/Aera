#pragma once

#include <boost/lexical_cast.hpp>

//////////////////////////////////////////////////////////////////////////

class MRUList
{
public:
    enum { iMaxItems=10 };

    MRUList();

    void add_to_list(std::string item);
    void remove_from_list(std::string item);

    std::string get_item(unsigned index) const;

    void set_language(std::string lang);
    std::string get_language() const;

    void UpdateMenu(CMenuHandle menu, int idc, bool leave=false);

    void save_to_registry();
    void read_from_registry(std::string name);
    void serialization(serl::archiver &ar);

private:
    std::vector<std::string> list_;
    std::string key_name_;
    std::string lang_;
    int					IDC;
    bool				bLeaveFirst;
};

//////////////////////////////////////////////////////////////////////////
