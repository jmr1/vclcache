#pragma once

#include <string>

namespace vclcache {

class string_finder_pred
{
public:
    string_finder_pred(const std::string &value)
        : str_(value)
    {}

    bool operator()(const std::string &value)
    {
        return value.find(str_) != std::string::npos;
    }

private:
    std::string str_;
};

}