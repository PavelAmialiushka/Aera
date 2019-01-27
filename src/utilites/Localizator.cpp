#include "stdafx.h"

#include "others/wtladdons.h"

#include "utilites/foreach.hpp"
#include "utilites/filesystem.h"
#include "utilites/utilites.h"

#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
namespace bfs = boost::filesystem;

#include "Localizator.h"

#include <sstream>
#include "utilites/log.h"

#include "json/json.h"

/////////////////////////////////////////////////////////////////////////

language::language(std::string file, bool load)
    : file(file)    
    , load_and_use_(load)
{    
    short_name = strlib::to_lower(file);
    short_name = strlib::replace(short_name, "/", "\\");
    short_name = strlib::head( strlib::rtail(short_name, "\\"), ".");

    process(file);
}

void language::process(std::string lang)
{
    using namespace strlib;

    if (fs::exists(lang))
    {
        std::string chapter_name;

        std::tifstream inf(lang.c_str());
        std::string file;

        std::string line;
        while(std::getline(inf, line))
            file += line;

        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(file, root, false))
        {
            std::string err = reader.getFormatedErrorMessages();
            assert(!"");
            return;
        }

        if (!root.isObject())
            return;

        name = root.get("language", "none").asString();

        Json::Value chaps = root.get("chapters", "");
        for(unsigned index=0 ; index < chaps.size(); ++index)
        {
            chapter_name = chaps[index].get("id", "").asString();

            Json::Value data = chaps[index].get("recs", "");
            for(unsigned jindex=0; jindex < data.size(); ++jindex)
            {
                std::string key = data[jindex]["key"].asString();
                std::string value = data[jindex]["value"].asString();
                contents[chapter_name][key] = value;
            }
        }
    }
}

void language::save()
{
}

void language::set_name(std::tstring n)
{
    name = n;
}

std::string language::explain(std::string id, std::tstring name, std::string cat)
{
    //id = strlib::trim_both(id);

    std::string answer;
    if (!contents.count(cat) || !contents[cat].count(id))
    {
        // если не найдено соответствия ничего не делаем
    }
    else
    {
        // найдено соответствие
        answer = strlib::head(contents[cat][id], ";;");
    }

    if (!load_and_use_ || answer.empty())
    {
        // если ответ пустой позвращаем исходную строку
        return name;
    }

    return contents[cat][id];
}

//////////////////////////////////////////////////////////////////////////

#include "utilites/foreach.hpp"

class localizator::impl
{
    std::map<std::string, std::map<std::string, std::pair<std::string, std::string> > > default_;
    std::map<std::string, shared_ptr<language> > langs_;

    std::string clang_;


public:

    impl()
    {
        set_language("");
        search_langs();
    }

    ~impl()
    {
        std::vector<language_info> langs = get_langs();
        foreach(language_info la, langs)
        {
            get_by_name(la.ref).save();
        }
    }

    void search_langs()
    {
        std::vector<std::string> tries;
        tries.push_back("langs");

#ifndef NDEBUG
        tries.push_back("..\\txt");
#endif
        tries.push_back(".");

        std::string dpath;
        foreach(std::string const& pat, tries)
        {
            dpath = fs::get_exefile_path(strlib::strf("%s\\%s.lng", pat, clang_));
            if (fs::exists( dpath )) break;
        }

        shared_ptr<language> ldefault( new language(dpath, false) );
        ldefault->set_name("English (default)");
        langs_[ clang_ ] = ldefault;

        if (!dpath.empty())
        {
            bfs::path path=bfs::path( strlib::rhead(dpath, "\\"), bfs::native);
            bfs::directory_iterator index=bfs::directory_iterator( path );
            for (; index!=bfs::directory_iterator(); ++index)
            {
                if (bfs::exists(*index) && !bfs::is_directory(*index))
                {
#if _MSC_VER > 1200
                    std::string name=index->path().string();
#else
                    std::string name=index->leaf();
#endif
                    std::transform(STL_II(name), name.begin(), tolower);

                    std::string name1 = strlib::rtail(name, "\\");
                    if (!name1.empty()) name = name1;

                    std::string ref = strlib::rhead(name, ".");
                    std::string ext = strlib::rtail(name, ".");
                    if (ext=="lng")
                    {
                        shared_ptr<language> ln;
                        if (ref != ldefault->get_short_name())
                        {
                            std::string fname = (path / name).string();
                            ln.reset( new language( fname ) );
                            langs_[ ln->get_short_name() ] = ln;
                        }
                    }
                }
            }
        }
    }

    struct pw_hh
    {
        localizator::impl *this_;
        std::string cat;
    };

    static BOOL CALLBACK _process_window(HWND hwnd, LPARAM lp)
    {
        pw_hh *hh= reinterpret_cast<pw_hh *>(lp);
        hh->this_->process_control(hwnd, hh->cat);
        return true;
    }

    void process_window(HWND hwnd, std::string cat)
    {
        MakeWindowText( hwnd ) = process_string("", (const char *)MakeWindowText( hwnd ), cat);

        pw_hh hh;
        hh.this_=this;
        hh.cat=cat;

        ::EnumChildWindows(
            hwnd,
            &_process_window,
            reinterpret_cast<LPARAM>(&hh));
    }

    void process_control(HWND hwnd, std::string cat)
    {
        CWindow wnd(hwnd);

        std::string name=CWindowText(hwnd);

        if (name.size())
        {
            std::string loc_name=process_string("", name, cat);
            MakeWindowText(hwnd)=loc_name;
        }
    }

    static std::string get_item_string(HMENU m, int index)
    {
        CMenuHandle menu(m);
        std::vector<char> tmp(menu.GetMenuStringLen(index, MF_BYPOSITION)+1);
        menu.GetMenuString(index, &tmp[0], tmp.size(), MF_BYPOSITION);
        return std::string(&tmp[0]);
    }

    static int get_item_id(HMENU menu, int index)
    {
        CMenuItemInfoEx mii(MIIM_ID|MIIM_SUBMENU|MIIM_DATA|MIIM_STATE);
        ::GetMenuItemInfo(menu, index, true, mii);

        if (mii.wID==(unsigned)mii.hSubMenu)
        {
            if (mii.dwItemData==0)
            {
                return -1;
            }
            return mii.dwItemData;
        }
        return mii.wID;
    }

    void process_menu(HMENU menu, std::string cat = "")
    {
        assert(menu);

        CMenuHandle mn(menu);
        unsigned count=mn.GetMenuItemCount();
        for (unsigned index=0; index<count; ++index)
        {
            CMenuItemInfoEx mii(MIIM_ID|MIIM_SUBMENU|MIIM_DATA|MIIM_TYPE|MIIM_STATE);
            mn.GetMenuItemInfo(index, true, mii);
            if (!(mii.fType&MFT_SEPARATOR))
            {
                std::string name=get_item_string(menu, index);
                std::string newname=process_single_line(std::string(), name, cat);

                mii.SetString(newname);
                mn.SetMenuItemInfo(index, true, mii);

                if (mii.hSubMenu)
                {
                    process_menu(mii.hSubMenu, cat /*+ " / " + name*/);
                };
            }
        }
    }

    std::string process_string(std::string id, std::string default_value, std::string cat)
    {
        // рассматриваем текущую строку
        std::string string=default_value;

        // отдельная строка обрабатывается здесь
        string = process_single_line(id, string, cat);

        return string;
    }

    std::string process_single_line(std::string &id, std::string &default_value, std::string cat)
    {
        std::string string=default_value;

        if (id.empty())
        {
            // если идентификатор не указан, то
            // либо это строка до символа #
            if (string.find("#")!=std::string::npos)
            {
                id = strlib::split(string, "#");

                // удаляем ненужный символ из меню
                id = strlib::replace(id, "&", "");
            }
        }

        // string # const_part
        // string \t const_part
        std::string const_part =strlib::tail(string, "#");
        if (const_part.empty())
        {
            //const_part = strlib::tail(string, "\t");
            //if (const_part.size()) const_part.insert(0, "\t");
            //string = strlib::head(string, "\t");
        }
        else
        {
            string=strlib::head(string, "#");
        }

        // получаем результат
        std::string result = get_localized_name(id, string, cat) + const_part;

        // возвращаем исходное "родное" значение
        // зачем?
        //default_value = string + const_part;

        // и локализованную версию
        return result;
    }

    //
    // строка является служебной и не должна переводиться
    bool is_auxilary(std::string const &news)
    {
        static const char *ts="-0123456789";
        return !(news.size()>=2 && strchr(ts, news[0])==0 && strchr(ts, news[1])==0);
    }

    //
    // пытаемся найти по строке, если не нашли, то по числу
    std::string get_localized_name(std::string id, std::string &default_value, std::string const &cat)
    {
        // если строка уже переводилась, то определяем id исходной строки
        if (id.empty() && default_.count(cat) && default_[cat].count(default_value))
        {
            tie(id, default_value) = default_[cat][default_value];
        }

        // возвращаемое значение
        std::string name = default_value;
        if (id.empty()) id = name;

        // игнорируем строки в начале
        // которых находятся цифры
        if (is_auxilary(id))
            return name;

        // не  переводим пустое имя
        if (name.empty())
            return name;

        // определяем имя в текущем словаре
        name = current_language().explain( id, name, cat );

        // запоминаем перевод, чтобы при смене языка востановить
        // значения по умолчанию
        default_[ cat ] [ name ] = std::make_pair(id, default_value);

        return name;
    }

//////////////////////////////////////////////////////////////////////////

    void set_language(std::string lang)
    {
        if (langs_.count(lang)) clang_ = lang;
        else clang_="en";
    }

    language &current_language() const
    {
        assert(langs_.find( clang_ ) != langs_.end());
        return *langs_.find( clang_ )->second;
    }

    language &get_by_name(std::string la)
    {
        if (langs_.count(la))
            return *langs_[la];

        assert("no lanuage found");
        return *langs_.begin()->second;
    }

    std::vector<language_info> get_langs() const
    {
        std::vector<language_info> result;

        std::map<std::string, shared_ptr<language> >::const_iterator
        index = langs_.begin();
        for (; index != langs_.end(); ++index)
        {
            language &lng = *index->second;

            language_info inf;
            inf.name = lng.get_name();
            inf.ref = lng.get_short_name();
            result.push_back(inf);
        }

        return result;
    }
    friend BOOL CALLBACK _process_window(HWND hwnd, LPARAM lp);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

using namespace strlib;

localizator::localizator()
    : pimpl_(new impl())
{
}

localizator::~localizator()
{
}

void localizator::set_language(std::string lang)
{
    pimpl_->set_language(lang);
}

std::string localizator::get_current_language() const
{
    return pimpl_->current_language().get_short_name();
}


std::vector<language_info> localizator::get_langs() const
{
    return pimpl_->get_langs();
}


//////////////////////////////////////////////////////////////////////////

void        localizator::process_window(HWND hwnd, std::string cat)
{
    pimpl_->process_window(hwnd, cat);
}

void        localizator::process_menu(HMENU menu, std::string cat)
{
    if (menu)
        pimpl_->process_menu(menu, cat);
}

std::string localizator::process_string(std::string str, std::string cat)
{
    if (startswith(cat, "file:"))
    {
        cat = replace(to_lower(cat), "\\", "/");
        cat = "file: src" + tail(cat, "src");
    }

    return pimpl_->process_string("", str, cat);
}

//////////////////////////////////////////////////////////////////////////
