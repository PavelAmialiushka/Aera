#include "stdafx.h"

#include "application.h"
#include "help.h"
#include <boost/filesystem/operations.hpp>

#include <SHELLAPI.H>

#define HELPFILE "UserManual.chm"
#define HELPPATH1 "../txt/Manual/"
#define HELPPATH2 "../../help/"

std::string get_help_filename()
{
    std::string path=utils::get_exefile_path(HELPFILE);

    if (!fs::exists(path))
        path = utils::get_exefile_path(HELPPATH1 HELPFILE);
    if (!fs::exists(path))
        path = utils::get_exefile_path(HELPPATH2 HELPFILE);
    return path;
}

void help(HWND h, const char *i)
{
    std::string topic=i;
    ::ShellExecute(GetActiveWindow(),
                   NULL,
                   "hh.exe",
                   ("\"mk:@MSITStore:"+get_help_filename()+"::/"+topic+".htm\"").c_str(),
                   NULL,
                   SW_SHOWMAXIMIZED);
}
