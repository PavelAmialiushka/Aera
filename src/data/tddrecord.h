#pragma once

typedef const double *tdd_record;

struct tdd_record_header
{
    unsigned short total_length;
    unsigned short common_length;
    unsigned short channel_length;
};
