#pragma once

#include <string>
#include <vector>
#include <map>

namespace vclcache {

class global_info
{
public:
    std::string cachedir_;
    std::string obj_path_;
    std::vector<std::string> tokens_;
    std::map<std::string, std::string> include_file_to_hash_map_;
};

}