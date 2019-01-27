#include "stdafx.h"

#include "serl/serl_registry.h"

#include <boost/tuple/tuple.hpp>
#include <boost/lexical_cast.hpp>
#include "utilites/tokenizer.h"

namespace serl
{

//////////////////////////////////////////////////////////////////////////

namespace
{

std::string decorate(std::string string)
{
    string=strlib::replace(string, "\\", "$#92");
    return string;
}

std::string undecorate(std::string string)
{
    string=strlib::replace(string, "$#92", "\\");
    return string;
}

std::string extract_value_name(std::string name, std::string &value_name)
{
    value_name=strlib::split(name, "\\");
    return name;
}
}

//////////////////////////////////////////////////////////////////////////

registry::registry(HKEY key, std::string name)
    : root_key_(key)
    , key_name_(name)
{
    root_.reset(new element(""));
    load_group(key, key_name_, root_);
}

registry::~registry()
{
}

pelement registry::root_node() const
{
    return root_;
}

void registry::do_save_data()
{
    save_group(root_key_, key_name_, root_);
}

void registry::save_group(HKEY base_key, std::string key_name, pelement element)
{
    // открываем группу
    CRegKey current_key;
    current_key.Create(base_key, key_name.c_str());

    if (current_key.Open(base_key, key_name.c_str())!=ERROR_SUCCESS)
        return;

    for(unsigned index=0; index < element->size(); ++index)
    {
        pelement child = element->at(index);
        if (child->is_folder())
        {
            save_group(current_key, decorate(child->name()), child);
        } else if (child->is_named_value())
        {
            std::string value = child->value();
            ::RegSetValueEx(current_key, decorate(child->name()).c_str(), NULL, REG_SZ,
                            (LPBYTE)value.c_str(), value.size());
        }
    }
}

void registry::load_group(HKEY base_key, std::string key_name, pelement element)
{
    // открываем группу
    CRegKey current_key;
    if (current_key.Open(base_key, key_name.c_str())!=ERROR_SUCCESS)
        return;

    // загрузка групп
    TCHAR sub_key_name[256];
    for (int index=0; ERROR_SUCCESS==RegEnumKey(current_key, index, sub_key_name, SIZEOF(sub_key_name)); ++index)
    {
        pelement child=element->lookup( undecorate(sub_key_name) );
        load_group(current_key, sub_key_name, child);
    }

    // загрузка значений
    TCHAR value_name[16384];
    DWORD value_name_size=SIZEOF(value_name);
    DWORD value_type=0;
    for (DWORD index=0;
            ERROR_SUCCESS==RegEnumValue(current_key, index, value_name, &value_name_size, 0, &value_type, 0, 0);
            ++index, value_name_size=SIZEOF(value_name))
    {
        std::string name = undecorate(value_name);
        switch (value_type)
        {
        case REG_DWORD:
        {
            DWORD value=0;
            DWORD value_size = sizeof value;
            ::RegQueryValueEx(current_key, value_name, NULL, NULL,
                              (LPBYTE)value, &value_size);

            element->insert( name, boost::lexical_cast<std::string>(value) );
        }; break;
        case REG_SZ:
        case REG_EXPAND_SZ:
        {
            TCHAR value[16384]= {0};
            DWORD value_size=SIZEOF(value);
            ::RegQueryValueEx(current_key, value_name, NULL, NULL,
                              (LPBYTE)value, &value_size);

            element->insert( name, value );
        }; break;
        }
    }
}


}
