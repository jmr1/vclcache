#include "ProcessRunner.h"

#include <Poco/Pipe.h>

namespace vclcache {

ProcessRunner::ProcessRunner(const std::string &arg)
{
    args_.push_back(arg);
}

void ProcessRunner::run(const std::string &proc)
{
    Poco::ProcessHandle process = Poco::Process::launch(proc, args_, NULL, NULL, NULL);
    process.wait();
}

}
