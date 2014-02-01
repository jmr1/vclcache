#pragma once

#include <string>
#include <ostream>

namespace vclcache {

template <typename T>
class Logger
{
public:
    Logger(T &out)
        : out_(out)
    {}

    T& operator<<(const std::string &str)
    {
        out_ << str;
        return out_;
    }

    T& operator<<(size_t value)
    {
        out_ << value;
        return out_;
    }

    T& operator*()
    {
        return out_;
    }

    // this is the type of std::cout
    typedef std::basic_ostream<char, std::char_traits<char> > CoutType;

    // this is the function signature of std::endl
    typedef CoutType& (*StandardEndLine)(CoutType&);

    // define an operator<< to take in std::endl
    T& operator<<(StandardEndLine manip)
    {
        // call the function, but we cannot return it's value
        manip(out_);
        return out_;
    }

private:
    T &out_;
};

}