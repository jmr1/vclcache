#include "ProcessRunner.h"

#include <Poco/Pipe.h>

namespace vclcache {

ProcessRunner::ProcessRunner(int argc, char** argv)
{
    for(int x = 1; x < argc; ++x)
        args_.push_back(argv[x]);
}

void ProcessRunner::run(const std::string &proc)
{
    Poco::ProcessHandle process = Poco::Process::launch(proc, args_, NULL, NULL, NULL);
    process.wait();
}

}
