#include "source_file_info.h"

#include <sstream>

namespace vclcache {

void source_file_info::set_hash(size_t value) 
{ 
    hash_ = value;
    std::ostringstream ss;
    ss << hash_;
    hash_string_ = ss.str().c_str();
}

}