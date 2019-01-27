#pragma once

enum
{
    selection_set,
    selection_add,
    selection_sub,
    selection_clear,
    selection_invert,
};

struct select_action
{
    select_action(int m = selection_clear, std::string s = std::string())
        : use_time_ranges(0)
        , mode(m)
        , filter(s)
    {
    }

    select_action(int m, std::string s, double a, double b)
        : use_time_ranges(0)
        , mode(m)
        , filter(s)
    {
        set_time_ranges(a, b);
    }

    void set_time_ranges(double from, double to)
    {
        range_begin=from;
        range_end=to;
        use_time_ranges=true;
    }

public:
    int mode;
    std::string filter;

    bool  use_time_ranges;
    double range_begin;
    double range_end;

};
