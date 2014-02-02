#include "ProcessRunner.h"

#include <Poco/Exception.h>

namespace vclcache {

ProcessRunner::ProcessRunner(int argc, char** argv)
{
    for(int x = 1; x < argc; ++x)
        args_.push_back(argv[x]);
}

bool ProcessRunner::run(const std::string &proc, std::string &error)
{
    
    try
    {
        Poco::ProcessHandle process = Poco::Process::launch(proc, args_, NULL, NULL, NULL);
        process.wait();
    }
    catch(Poco::Exception &e)
    {
        error = e.displayText();
        return false;
    }
    catch(std::exception &e)
    {
        error = e.what();
        return false;
    }
    catch(...)
    {
        error = "ProcessRunner::run other error.";
        return false;
    }

    return true;
}

}
