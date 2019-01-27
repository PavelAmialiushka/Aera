#pragma once

namespace monpac
{

class channel;

class Result
{
public:

    Result(std::vector<channel> const &result);

    unsigned size() const;
    channel const &channel(unsigned) const;

private:

    std::vector<monpac::channel> channels_;
};

}