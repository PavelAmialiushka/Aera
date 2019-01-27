//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

//#pragma comment(lib, "Shell32.lib")


//#include "shlobj.h"
//#include "utilites/application.h"
//#include "utilites/utilites.h"
//
//
//#include "strings.h"
//#include "tokenizer.h"
//
//namespace utils
//{
//
//fs::path get_app_path()
//{
//  char name[MAX_PATH];
//  ::SHGetSpecialFolderPath(NULL, &name[0], CSIDL_APPDATA, true);
//  fs::path pathname=fs::path(&name[0], fs::native);
//  pathname /= "AeraTools";
//  if (!fs::exists(pathname)) fs::create_directory(pathname);
//  return pathname;
//}
//
//fs::path get_exefile_name()
//{
//  char name[MAX_PATH];
//
//  ::GetModuleFileName(NULL, &name[0], SIZEOF(name));
//  fs::path exename=fs::path(&name[0], fs::native);
//  return exename;
//}
//
//
//fs::path get_exefile_path()
//{
//  return get_exefile_name().branch_path();
//}
//
//
//std::string get_local_file_name(std::string filename)
//{
//  return (get_exefile_path() / filename).native_file_string();
//}
//
//std::string make_local_path(std::string patha, std::string pathb)
//{
//	std::string prj =to_lower( pathb );
//	std::string data=to_lower( patha );
//
//	std::string x, y;
//	do {
//		x=tokenizer::head(data, "\\");
//		y=tokenizer::head(prj, "\\");
//	} while(x==y && data.size() && prj.size());
//
//	if (x.find(":")!=std::string::npos)
//	{
//		return x+"\\"+data;
//	} else {
//		data.insert(0, data.size() ? x+"\\" : x);
//
//		while(prj.size())
//		{
//			data.insert(0, "..\\");
//			tokenizer::head(prj, "\\");
//		}
//		return data;
//	}
//}
//
//}
