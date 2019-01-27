#pragma once

#include "aera/wtladdons.h"

class CToolbarButtons
{
    std::vector<CTBButton> buttons_;
    std::vector<std::string> strings_;
    int bitmap_;
public:

    CToolbarButtons(int bitmap)
        : bitmap_(bitmap)
    {
    }

    void Append(int id=-1, int bitmap=0, std::string str="", int style=TBSTYLE_SEP, int state=TBSTATE_ENABLED)
    {
        unsigned strid=-1;
        if (str.size())
        {
            strid=std::find(STL_II(strings_), str)-strings_.begin();
            if (strid>=strings_.size()) strings_.push_back(str);
        }
        buttons_.push_back( CTBButton(id, bitmap, strid, style, state) );
    }

    void Apply(CToolBarCtrl &view)
    {
        std::string string;
        foreach(std::string item, strings_)
        {
            string+=item+'\0';
        }
        view.AddStrings(string.c_str());

        if (!view.GetButtonCount())
        {
            view.AddBitmap(1, bitmap_);
            view.AddButtons(buttons_.size(), &buttons_[0]);
        }
        view.AutoSize();
    }
};
