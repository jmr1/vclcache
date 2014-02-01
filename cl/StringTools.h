#pragma once

#include <string>
#include <vector>

namespace vclcache {

class StringTools
{
public:
    static void str_replace(std::string &s, const std::string &search, const std::string &replace);
    static void tokenize(const std::string &input, const std::string &separator, std::vector<std::string> &output);
    static void tokenize2(const std::string &input, const std::string &separator, std::vector<std::string> &output);
};

}