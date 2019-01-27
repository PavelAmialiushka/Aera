#pragma once

namespace data
{

class collection;

enum
{
    MK_RAW_SETUP   =0x01,
    MK_RAW_TESTING =0x02,
    MK_RAW_ANY     =0x03,
    MK_AE_TESTING  =0x04,
    MK_TDD_DATA    =0x10,
    MK_SORTED      =0x20,
    MK_TESTING     =MK_AE_TESTING|MK_RAW_TESTING|MK_SORTED,

    MK_ALL_DATA		 =MK_TESTING|MK_RAW_SETUP|MK_TDD_DATA,
};

}