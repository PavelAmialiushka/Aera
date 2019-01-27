#pragma once

class colors
{
public:
    enum indexes_t { mixture=65, normal, selected, };

    static int by_chanel(int chanel);
    static int channel_by_color(int color);
};


