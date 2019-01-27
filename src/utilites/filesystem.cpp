//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#pragma comment(lib, "Shell32.lib")
#include <SHLOBJ.H>
#include <shellapi.h>

#include "filesystem.h"
#include "strings.h"
#include "utilites.h"

#include "foreach.hpp"
#include "log.h"
#include "tokenizer.h"

namespace fs
{

std::string get_app_path(std::string const &file)
{
    char name[MAX_PATH];
    ::SHGetSpecialFolderPath(NULL, &name[0], CSIDL_APPDATA, true);

    std::string path = name;

    if (!file.empty())
        path += "\\" + file;

    return path;
}

std::string get_temp_filename()
{
    char name[MAX_PATH];
    ::GetTempFileName(".", "aera_", 0, name);
    return name;
}

unsigned get_filesize(std::string name)
{
    FILE *f = fopen(name.c_str(), "rb");
    fseek(f, 0, SEEK_END);
    unsigned result = ftell(f);
    fclose(f);
    return result;
}

std::string get_exefile_path(std::string const &file)
{
    char name[MAX_PATH];
    ::GetModuleFileName(NULL, &name[0], SIZEOF(name));

    std::string tmp=name;
    std::string path = strlib::rsplit(tmp, "\\");

    if (!file.empty())
        path += "\\" + file;

    return path;
}

std::string get_exefile_name()
{
    char name[MAX_PATH];
    ::GetModuleFileName(NULL, &name[0], SIZEOF(name));

    return name;
}

void create_directory( std::string path )
{
    ::CreateDirectory(path.c_str(), 0);
}

bool exists( std::string path)
{
    WIN32_FIND_DATA data;
    HANDLE find = ::FindFirstFile(path.c_str(), &data);
    bool result = find != INVALID_HANDLE_VALUE;
    ::FindClose(find);

    return result;
}

void startfile(std::string file, std::string options)
{
    std::string path = strlib::rsplit(file, "\\");
    options = file + " " + options;

    int result = (int)::ShellExecute(0,
                                     "open",
                                     file.c_str(),
                                     options.c_str(),
                                     path.c_str(),
                                     SW_SHOWNORMAL);

    if (result < 0) throw std::runtime_error("file not started");
}

const char SLASH = '\\';

bool is_complete(std::string path)
{
    return path.size() > 2
           && ( (path[1] == ':' && path[2] == SLASH) // "c:/"
                || (path[0] == SLASH && path[1] == SLASH) // "//share"
                || path[path.size()-1] == ':' );
}

bool has_root_name(std::string path)
{
    return path.size() > 1
           && ( path[1] == ':' // "c:"
                || path[path.size()-1] == ':' // "prn:"
                || (path[0] == SLASH && path[1] == SLASH) // "//share"
              );
}

bool has_root_directory(std::string path)
{
    return ( path.size()
             && path[0] == SLASH )  // covers both "/" and "//share"
           || ( path.size() > 2
                && path[1] == ':' && path[2] == SLASH ) // "c:/"
           ;
}

std::string root_name(std::string path)
{
    std::string::size_type pos( path.find( ':' ) );
    if ( pos != std::string::npos ) return path.substr( 0, pos+1 );
    if ( path.size() > 2 && path[0] == '\\' && path[1] == SLASH )
    {
        pos = path.find( SLASH, 2 );
        return path.substr( 0, pos );
    }

    return "";
}

std::string complete(std::string a, std::string b)
{
    assert( is_complete(b)
            && (is_complete(a) || !has_root_name(a)) ); // precondition

    if (a.empty() || is_complete(a)) return a;
    if ( !has_root_name(a) )
        return has_root_directory(a)
               ? root_name(b) + SLASH + a
               : b + SLASH +  a;

    return b + SLASH + a;

}

std::string system_complete(std::string ph)
{
    if ( ph.empty() ) return ph;
    char buf[MAX_PATH];
    char *pfn;
    std::size_t len = ::GetFullPathNameA( ph.c_str(),
                                          sizeof(buf), buf, &pfn );
    if ( !len )
    {
        throw std::logic_error("system_complete");
    }

    buf[len] = '\0';
    return std::string( buf );
}

void remove_file(std::string fname)
{
    ::DeleteFile(fname.c_str());
}

void split_path(std::string path, std::vector<std::string>& list)
{
    while( path.size() )
    {
        std::string head = tokenizer::head(path, std::string(1, SLASH));
        if (head.size())
            list.push_back( head );
    }
}

std::string join_path(std::vector<std::string> const& list)
{
    std::string path;

    foreach(std::string t, list)
    {
        path = tokenizer::join(path, std::string(1, SLASH), t );
    }
    return path;
}

std::string try_to_locate(std::string file, std::string project)
{
    std::vector<std::string> file_list, project_list;
    split_path(file, file_list);
    split_path(project, project_list);

    if (project_list.size())
        project_list.pop_back();
    project_list.push_back( file_list.back() );

    std::string str = join_path(project_list);
    if (exists(str))
        return str;

    return std::string();
}

}

