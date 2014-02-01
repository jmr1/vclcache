#pragma once

#include <string>
#include <vector>
#include <set>

namespace vclcache {

class CompilerTools
{
public:
    static std::string strip_comments(std::string const& input);
    static void tokenize_params(const std::string &rsp_content, std::vector<std::string> &tokens);
    static void retrive_obj_path(const std::vector<std::string> &tokens, std::string &obj_path);
    static bool get_compilation_include_paths(const std::string &input, std::set<std::string> &folder_includes, std::string &error);
    static bool get_file_hash_includes(const std::string &file_source, std::set<std::string> &hash_includes, std::string &error);
    static void get_file_hash_includes(const std::string &file_source, std::set<std::string> &hash_includes);
};

}