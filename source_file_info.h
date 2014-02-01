#pragma once

#include <string>
#include <map>

#include "global_info.h"

namespace vclcache {

class source_file_info
{
public:
    source_file_info(global_info &gi)
        : hash_(0), 
          cache_used_(false), 
          gi_(gi), 
          file_needs_recompilation_(true), 
          last_modification_time_(0)
    {}

    const std::string get_obj_fullpath() const { return gi_.obj_path_ + "\\" + obj_filename_; }
    const std::string get_obj_fullpath_cached() const { return gi_.cachedir_ + "\\" + obj_filename_; }

    const std::string get_obj_fullpath_cached_mod() const { return gi_.cachedir_ + "\\" + obj_filename_hash_; }

    void set_cache_used(bool value) { cache_used_ = value; }
    bool get_cache_used() const { return cache_used_; }

    void set_src_filename(const std::string &value) { src_filename_ = value; }
    std::string get_src_filename() const { return src_filename_; }

    void set_last_modification_time(time_t value) { last_modification_time_ = value; }
    time_t get_last_modification_time() const { return last_modification_time_; }

    void set_hash(size_t value);
    size_t get_hash() const { return hash_; }
    std::string get_hash_string() const { return hash_string_; }

    void set_obj_filename(const std::string &value) { obj_filename_ = value; }
    std::string get_obj_filename() const { return obj_filename_; }

    void set_hash_obj_filename(const std::string &value) { obj_filename_hash_ = value; }
    std::string get_hash_obj_filename() const { return obj_filename_hash_; }

    void set_file_needs_recompilation(bool value) { file_needs_recompilation_ = value; }
    bool get_file_needs_recompilation() const { return file_needs_recompilation_; }

    std::map<std::string, std::string>& get_hash_include_files_to_cache() { return hash_include_files_to_cache_; }

    std::map<std::string, time_t>& get_hash_includes_last_mod_time() { return hash_includes_last_mod_time_; }

private:
    std::string src_filename_;
    std::string obj_filename_;
    std::string obj_filename_hash_;
    size_t hash_;
    std::string hash_string_;
    bool cache_used_;
    const global_info &gi_;
    bool file_needs_recompilation_;
    time_t last_modification_time_;
    std::map<std::string, std::string> hash_include_files_to_cache_;
    std::map<std::string, time_t> hash_includes_last_mod_time_;
};

}