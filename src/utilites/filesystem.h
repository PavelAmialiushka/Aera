//////////////////////////////////////////////////////////////////////////
//
// utilites library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////

#pragma once

namespace fs
{

std::string get_exefile_name();
std::string get_exefile_path(std::string const& = std::string());
std::string get_app_path(std::string const& = std::string());
std::string get_temp_filename();

unsigned get_filesize(std::string);

void create_directory( std::string );
bool exists( std::string );
std::string try_to_locate(std::string file, std::string project);

void startfile(std::string filename, std::string options="");

void remove_file(std::string);

bool is_complete(std::string);
std::string complete(std::string, std::string b);
std::string system_complete(std::string);

}
