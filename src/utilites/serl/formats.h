#pragma once

#include "archive.h"
#include "exception.h"

#include "serl_text.h"
#include "serl_data.h"

namespace serl
{

enum { format_text, format_data };

template<class T> bool save_to_file(std::string filename, int fmt, T &t)
{
    std::string contents;

    if (fmt==format_text)
    {
        save_archiver(new serl::text(contents)).serial(t);
    }
    else
    {
        save_archiver(new serl::data(contents)).serial(t);
    }

    std::ofstream stream(filename.c_str(), std::ios::out | (fmt==format_text ? 0 : std::ios::binary) );
    stream.write(contents.data(), contents.size());

    return !!stream;
}

template<class T> bool load_from_file(std::string filename, int fmt,  T &t)
{
    std::ifstream stream(filename.c_str(), std::ios::in | (fmt==format_text ? 0 : std::ios::binary));
    if (stream)
    {
        std::string file;

        char buffer[131072];
        while (stream)
        {
            stream.read(buffer, SIZEOF(buffer));
            file.append(buffer, stream.gcount());
        }

        // backup current serup
        std::string temporal_storage;
        serl::save_archiver(new serl::data(temporal_storage)).serial(t);

        try
        {
            if (fmt!=format_text)
            {
                serl::load_archiver(new serl::data(file)).serial(t);
            }
            else
            {
                serl::load_archiver(new serl::text(file)).serial(t);
            }

            return true;
        }
        catch (serl::error &)
        {
            serl::load_archiver(new serl::data(temporal_storage)).serial(t);
        }
    }
    return false;
}

}
