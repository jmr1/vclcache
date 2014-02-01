#pragma once

#include <vector>

namespace vclcache {

class source_file_info;
class ProcessRunner;

class Cache
{
public:
    Cache(std::ostream &trace)
        : trace_(trace), cache_used_(false)
    {}


    void use_cache(std::vector<source_file_info> &files_info_vec);
    void cache_files(std::vector<source_file_info> &files_info_vec, bool use_hash, ProcessRunner &processRunner);

private:
    std::ostream &trace_;
    bool cache_used_;
};


}
