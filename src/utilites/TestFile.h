#pragma once

#include "utilites\unittest.h"

std::string get_test_filename(std::string path);

enum
{
    easy_file = 0,
    medium_file = 1,
    large_file = 2,
    wave_file = 3
};

std::string get_test_filename(int x);
