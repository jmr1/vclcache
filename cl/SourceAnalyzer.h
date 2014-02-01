#pragma once

#include <ostream>
#include <set>
#include <map>

#include "global_info.h"
#include "source_file_info.h"

namespace vclcache {

class SourceAnalyzer
{
public:
    SourceAnalyzer(std::ostream &trace, 
                   global_info &gi, 
                   source_file_info &sfi,
                   bool strip_comments)
        : trace_(trace), 
          gi_(gi), 
          sfi_(sfi),
          strip_comments_(strip_comments)
    {}

    // 'last modified time' version
    bool source_file_needs_recompilation_lmt(time_t obj_file_last_modification_time, 
                                         const std::set<std::string> &folder_includes, 
                                         std::set<std::string> &hash_includes);

    // 'hash' version
    void source_file_needs_recompilation_hash(const std::set<std::string> &folder_includes, 
                                         std::set<std::string> &hash_includes, 
                                         std::set<std::string> &hash_includes_hashed);

    bool prepare_source_information(const std::string &src_file, 
                                    const std::string &compilation_params, 
                                    bool hash_method);

    bool prepare_for_analysis(const std::string &rsp_content, 
                              std::set<std::string> &folder_includes,
                              std::set<std::string> &hash_includes);

    // 'last modified time' version
    void analyze_source_and_dependencies_lmt();

    // 'hash' version
    void analyze_source_and_dependencies_hash(std::set<std::string> &folder_includes,
                                         std::set<std::string> &hash_includes);

    // 'last modified time' version
    void analyze_source_and_dependencies_cached_lmt(std::set<std::string> &folder_includes,
                                                    std::set<std::string> &hash_includes);

    // 'hash' version
    void analyze_source_and_dependencies_cached_hash(std::set<std::string> &folder_includes,
                                                std::set<std::string> &hash_includes);

private:
    std::ostream &trace_;
    global_info &gi_;
    source_file_info &sfi_;
    std::string file_source_;
    std::string file_path_;
    bool strip_comments_;
};

}

