#pragma once

#include "singleton.h"
#include "strings.h"

class language
{
    typedef std::map<std::string, std::tstring> Chapter;
    typedef std::map<std::string, Chapter> Book;

    bool  load_and_use_;

    std::string file;
    std::string name;
    std::string short_name;

    Book    contents;

    language(language const &);
    void operator=(language const &);

public:
    language(std::string file, bool load = true);

    void process(std::string lang);
    void save();

    std::string explain(std::string id, std::string name, std::string cat);

    void set_name(std::string);

    std::string get_short_name() const
    {
        return short_name;
    }

    std::string get_name() const
    {
        return name;
    }
};

struct language_info
{
    std::string ref;
    std::string name;
};

class localizator : public utils::singleton<localizator>
{
public:

    localizator();
    ~localizator();

    void set_language(std::string lang);
    std::string get_current_language() const;

    std::vector<language_info> get_langs() const;

    //////////////////////////////////////////////////////////////////////////


    void				process_window(HWND, std::string);
    void				process_menu(HMENU menu, std::string = "");
    std::string process_string(std::string, std::string);

private:

    class impl;
    shared_ptr<impl> pimpl_;
};

inline localizator &Localizator()
{
    return *localizator::instance();
}

#define _lis(str) str
#define _lis2(str) localizator::instance()->process_string(str, "file: " __FILE__)

#define _ls(str)  localizator::instance()->process_string(str, "file: " __FILE__)
#define _lcs(str) localizator::instance()->process_string(str, "file: " __FILE__).c_str()
#define _lm(str, idd)  localizator::instance()->process_menu(str, std::string("menu: ") + #idd)
#define _lw(str, idd)  localizator::instance()->process_window(str, std::string("dialog: ") + #idd)
