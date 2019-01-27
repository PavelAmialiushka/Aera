#pragma once

namespace data
{

class dta_saver
{
public:
    dta_saver(std::string path);
private:

    std::ofstream stream_;

};

}