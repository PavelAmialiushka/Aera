#pragma once

namespace data
{

enum raw_type_t
{
    CR_NONE     =0,
    CR_DATE     =1,
    CR_SOFTWARE =2,
    CR_TITLE    =3,
    CR_COMPLEX_RECORD=4,

    CR_COMMENT=CR_TITLE,

    CR_ABORT    =5,
    CR_RESUME   =6,
    CR_PAUSE    =7,
    CR_STOP     =8,
    CR_TIMEMARK =9,
    CR_THRESHOLD=10,
};


//////////////////////////////////////////////////////////////////////////

struct raw_record
{
    int         id_;
    raw_type_t  type_;
    double      time_;
    bool		submessage_;

    unsigned    string_size_;
    char       *string_;

    unsigned    data_size_;
    char       *data_;
};

//////////////////////////////////////////////////////////////////////////

}
