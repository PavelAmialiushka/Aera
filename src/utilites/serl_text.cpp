#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////

#include "serl/element.h"
#include "serl/serl_text.h"

//////////////////////////////////////////////////////////////////////////
namespace serl
{

using namespace strlib;

//////////////////////////////////////////////////////////////////////////

namespace
{

std::string undecorate(std::string value)
{
    std::string result;
    for (unsigned index=0; index<value.size(); ++index)
    {
        char c=value[index];
        if (c=='\\')
        {
            if (index+1<value.size())
            {
                char c=value[index+1];
                if (c=='\\') result+='\\';
                else if (c=='0') result+='\0';
                else if (c=='n') result+='\n';
                else if (c=='x')
                {
                    char b[3];
                    strncpy(b, &value[index+2], 2); b[2] = 0;
                    int val = 0; sscanf(b, "%x", &val);

                    index+=2;
                    result+=(char)val;
                }
                ++index;
            }
            else result+=c;
        }
        else result+=c;
    }
    return result;
}

std::string decorate(std::string value)
{
    std::string result;
    result.reserve(value.size()*2);

    for (unsigned index=0; index < value.size(); ++index)
    {
        char c=value[index];
        if (c=='\\') result+="\\\\";
        else if (c=='\0') result+="\\0";
        else if (c=='\n') result+="\\n";
        else if (c=='=' || (0<= c && c < 32))
            result+= strlib::strf("\\x%02x", 0xFF&(unsigned)c);
        else result+=c;
    }
    return result;
}

}


text::text(const std::string &src)
    : result_(0)
    , source_(src)
{
    root_.reset( new element("") );

    std::string tmp = src;
    load_text(tmp, root_);
}

text::text(std::string &string)
    : result_(&string)
{
    root_.reset( new element("") );

    std::string tmp = string;
    load_text(tmp, root_);
}

void text::load_text(std::string& contents, serl::pelement element)
{
    int level0 = contents.find_first_not_of(' ');

    std::string value = strlib::trim_left(tokenizer::head(contents, "\n"));
    std::string name = tokenizer::head(value, "=");

    pelement child;
    if (!name.empty())
        child = element->lookup(undecorate(name));
    else
        child = element;

    if (!value.empty())
        child->set_value(undecorate(value));

    while (!contents.empty())
    {
        int level = contents.find_first_not_of(' ');

        if (level > level0)
            load_text(contents, child);
        else
            return;
    }
}

void text::save_text(std::string& contents, pelement element, int level /*=0*/)
{
    std::string header = element->is_named_value()
            ? decorate(element->name()) + "=" + decorate(element->value())
            : decorate(element->name());

    contents += std::string(level, ' ') + header + "\n";

    if (element->is_folder())
    {
        for(unsigned index=0; index < element->size(); ++index)
            save_text(contents, element->at(index), level+1);
    }
}


text::~text()
{
}

pelement text::root_node() const
{
    return root_;
}

void text::do_save_data()
{
    if (result_)
    {        
        *result_ = "";
        save_text(*result_, root_, 0);
    }
}

//////////////////////////////////////////////////////////////////////////


}
