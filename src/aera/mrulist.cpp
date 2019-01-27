#include "stdafx.h"

#include "mrulist.h"

#include "utilites/serl/Archive.h"
#include "utilites/serl/serl_registry.h"
#include "utilites/serl/serl_text.h"

MRUList::MRUList()
    : bLeaveFirst(false)
    ,	IDC(0)
{}

void MRUList::add_to_list(std::string item)
{
    remove_from_list(item);
    list_.insert(list_.begin(), item);
    if (list_.size()>iMaxItems) list_.pop_back();
}

static bool icompare(std::string const &alpha, std::string const &bravo)
{
    return strlib::to_lower(alpha)==strlib::to_lower(bravo);
}

void MRUList::remove_from_list(std::string item)
{
    list_.erase(
        std::remove_if(
            STL_II(list_),
            bind(icompare, _1, boost::cref(item))),
        list_.end());
}

std::string MRUList::get_item(unsigned index) const
{
    debug::Assert<overflow>(index < list_.size(), HERE);
    return list_[index];
}

void MRUList::UpdateMenu(CMenuHandle menu, int idc, bool leave)
{
    IDC=idc;
    bLeaveFirst=leave;
    MENUITEMINFO mii= {sizeof(MENUITEMINFO)};
    mii.fMask=MIIM_SUBMENU|MIIM_STATE|MIIM_ID;
    for (int index=0; index<menu.GetMenuItemCount(); ++index)
    {
        menu.GetMenuItemInfo(index, true, &mii);
        if (mii.wID && mii.wID==(unsigned)mii.hSubMenu)
        {
            UpdateMenu(mii.hSubMenu, idc, leave);
        }
        else if (mii.wID==(DWORD)IDC)
        {
            for (int index=bLeaveFirst ? 1: 0; menu.DeleteMenu(index, MF_BYPOSITION); )
                ;

            if (list_.size())
            {
                for (unsigned index=0; index<list_.size(); ++index)
                {
                    std::string name=list_[index];
                    size_t ik=name.find_last_of("\\");
                    //if (ik!=std::string::npos)
                    //  name.erase(0, ik+1);

                    name=(index<9 ? "&" : "") + boost::lexical_cast<std::string>(index+1)+" "+name;

                    menu.AppendMenu(MF_STRING, IDC+index, name.c_str());
                }
            }
            else
            {
                menu.AppendMenu(MF_STRING|MFS_GRAYED, IDC, "none");
            }
            return;
        }
    }
}

void MRUList::serialization(serl::archiver &ar)
{
    ar.serial("Language", lang_);
    ar.serial_container("MostRecentlyUsed", list_);
}

void MRUList::set_language(std::string lang)
{
    lang_ = lang;
}

std::string MRUList::get_language() const
{
    return lang_;
}

void MRUList::save_to_registry()
{
    if (key_name_.size())
        serl::save_archiver(
            new serl::registry(HKEY_CURRENT_USER, key_name_.c_str()))
        .serial(*this);
}

void MRUList::read_from_registry(std::string name)
{
    key_name_=name;
    serl::load_archiver(
        new serl::registry(HKEY_CURRENT_USER, key_name_.c_str()))
    .serial(*this);
}
