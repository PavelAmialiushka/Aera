#include "stdafx.h"

#include "TestFile.h"

#include "filesystem.h"
#include <boost/filesystem/operations.hpp>

std::string get_test_filename(std::string path)
{
    std::string curr=fs::get_exefile_path();
    for (; !curr.empty();)
    {
        std::string full = curr + "\\" + path;
        if (fs::exists(full)) return full;

        full = curr + "\\demo\\" + path;
        if (fs::exists(full)) return full;

        curr=strlib::rhead(curr, "\\");
    }
    throw fault(HERE);
}

std::string get_test_filename(int x)
{
    switch (x)
    {
    default:
    case 0: return get_test_filename("DtaTest.data");
    case 1: return get_test_filename("Medium_test.data");
    case 2: return get_test_filename("LARGE_test.data");
    case 3: return get_test_filename("wf.data");
    case 4: return get_test_filename("demo.data");
    case 11: return get_test_filename("Alcor_sample_data_2.naf");
    case 12: return get_test_filename("Alcor_sample_data_3.naf");
    }
}
