#pragma once

#include <string>
#include <set>

namespace vclcache {

class FileTools
{
public:
    static bool read_file_source(const std::string &src_file_path, std::string &file_source, std::string &error, bool strip_comments);
    static bool find_path(const std::string &filename, const std::set<std::string> &folder_includes, std::string &output_path);
    static bool is_source_file(const std::string &src_file_path);
    static bool read_utf16le(const std::string &path, std::string &content, std::string &error);
};

}
