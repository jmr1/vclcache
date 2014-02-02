#pragma once

#include <vector>

namespace vclcache {

class source_file_info;

class Cache
{
public:
    Cache(std::ostream &trace)
        : trace_(trace)
    {}


    bool use_cache(std::vector<source_file_info> &files_info_vec);
    void cache_files(std::vector<source_file_info> &files_info_vec, bool use_hash);

private:
    std::ostream &trace_;
};


}
