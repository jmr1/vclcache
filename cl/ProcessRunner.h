#pragma once

#include <string>
#include <Poco/Process.h>

namespace vclcache {

class ProcessRunner
{
public:
    ProcessRunner(int argc, char** argv);

    void run(const std::string &proc);

private:
    Poco::Process::Args args_;
    
};

}
