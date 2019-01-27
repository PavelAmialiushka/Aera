//////////////////////////////////////////////////////////////////////////
//
// data library
//
// Written by Pavel Amialiushka
// No commercial use permited.
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#include "rtf_outputer.h"

#include "stage.h"
#include "channel.h"
#include "criterion.h"

#include "utilites/Localizator.h"

namespace monpac
{

static std::string cvt(int x)
{
    char buffer [30]; itoa(x, buffer, 10);
    return buffer;
}

static std::string cvt(unsigned x)
{
    return cvt((int)x);
}

rtf_outputter::rtf_outputter(bool in)
    : intensivity_(in)
{

}

// строка таблицы должна выгл€деть так:

// ѕараметры, определ€ющие вид строки
// \\clvertalc\\cellx100\\cellx200

// ѕараметры, определ€ющие содержание строки
// VALUE \\cell VALUE2\\cell

class rtf_table
{
public:
    rtf_table(std::string dec="", std::string def="")
        : currx_(0),
          declare_(dec),
          define_(def)
    {}

    void append_cell(std::string text, int width)
    {
        currx_+=width;
        declare_+=
            "\\clvertalc"
            "\\clbrdrt\\brdrs\\brdrw10 "
            "\\clbrdrl\\brdrs\\brdrw10 "
            "\\clbrdrb\\brdrs\\brdrw10 "
            "\\clbrdrr\\brdrs\\brdrw10 "
            "\\cellx"+cvt(currx_);
        define_ +=" " + text + "\\cell";
    }

    std::string get_text() const
    {
        return declare_+"\n"+define_+"\\row";
    }
private:
    int				  currx_;
    std::string declare_;
    std::string define_;
};

enum
{

    step_notmal	=600,

    step_small	=400,
    step_large	=1000,
    step_double	=step_large+step_small,
};


void rtf_outputter::out_header()
{
    buffer_ <<
            "{\\rtf1\\ansi\\ansicpg1251\\deff0\\deflang1049\n"
            "{\\fonttbl\n"
            "  {\\f0\\froman\\fprq2\\fcharset204{\\*\\fname Times New Roman;}Times New Roman CYR;}\n"
            "  {\\f1\\fswiss\\fprq2\\fcharset204 Tahoma;}\n"
            "  }\n"
            "{\\*\\generator Msftedit 5.41.15.1503;}\n"
            "\\viewkind4\\uc1\n"
            "\\trowd\\trgaph15\\trleft-60\\trpaddl15\\trpaddr15\\trpaddfl3\\trpaddfr3\n";

    rtf_table table("", "\\pard\\intbl\\nowidctlpar\\qc\\b0\\f0\\fs20");

    table.append_cell("#", step_notmal);

    if (!intensivity_)
    {
        for (unsigned index=0; index<get_holds_count(); ++index)
        {
            table.append_cell(criterion_.get_holds().at(index).name, step_notmal);
        }

        table.append_cell(_lcs("MP-crt1#Hit during holds"),		step_large);

        table.append_cell(_lcs("MP-crt2#Total hits"),					step_double);
        table.append_cell(_lcs("MP-crt3#Hits with Ampl.>65"),	step_double);
        table.append_cell(_lcs("MP-crt4#Duration"),						step_double);
        table.append_cell(_lcs("MP-crtall#Monpac"),						step_notmal);
    }
    else
    {
        table.append_cell(_lcs("MP-crt2#Total hits"),					step_notmal);
        table.append_cell(_lcs("MP-sevr#Severity"),						step_large);
        table.append_cell(_lcs("MP-hidx#H.index"),						step_large);
    }
    table.append_cell(_lcs("MP-zip#ZIP"),										step_notmal);

    buffer_ << table.get_text();
}

std::string rtf_outputter::get_line_format(channel const &chan)
{
    rtf_table table(
        "\\trowd\\trgaph15\\trleft-60\\trpaddl15\\trpaddr15\\trpaddfl3\\trpaddfr3\n",
        "\\pard\\intbl\\nowidctlpar\\qc\\b0 \n");

    table.append_cell("$CHN", step_notmal);

    if (!intensivity_)
    {
        for (unsigned i=0; i<get_holds_count(); ++i)
        {
            table.append_cell("$HLD"+cvt(i), step_notmal);
        }
        table.append_cell("$HLDX",	step_large);
        table.append_cell("$TTL",		step_large);
        table.append_cell("$TTLX",	step_small);
        table.append_cell("$A65",		step_large);
        table.append_cell("$A65X",  step_small);
        table.append_cell("$DUR",		step_large);
        table.append_cell("$DURX",	step_small);
        table.append_cell("$MPX",		step_notmal);
    }
    else
    {
        table.append_cell("$TTL",		step_notmal);
        table.append_cell("$SEV",		step_large);
        table.append_cell("$HIX",		step_large);
    }
    table.append_cell("$ZIP",			step_notmal);

    return table.get_text();
}

void rtf_outputter::out_finish()
{
    buffer_ << "\\pard\\f0\\par}\n";
}

}