#pragma once

#include <string>
#include <Poco/Process.h>

namespace vclcache {

class ProcessRunner
{
public:
    ProcessRunner(int argc, char** argv);

    bool run(const std::string &proc, std::string &error);

private:
    Poco::Process::Args args_;
};

}
