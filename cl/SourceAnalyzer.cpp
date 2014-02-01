#include "SourceAnalyzer.h"

#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>

#include "StringTools.h"
#include "FileTools.h"
#include "CompilerTools.h"

namespace vclcache {


bool SourceAnalyzer::source_file_needs_recompilation_lmt(time_t obj_file_last_modification_time, 
                                                         const std::set<std::string> &folder_includes, 
                                                         std::set<std::string> &hash_includes)
{
    for(std::set<std::string>::const_iterator hi_itor = hash_includes.begin(); hi_itor != hash_includes.end(); ++hi_itor)
    {
        std::string existing_file_path;
        if(!FileTools::find_path(*hi_itor, folder_includes, existing_file_path))
        {
            trace_ << "Couldn't find path for " << *hi_itor << std::endl;
            continue;
        }

        std::map<std::string, time_t>::const_iterator hilmt_itor;
        if((hilmt_itor = sfi_.get_hash_includes_last_mod_time().find(existing_file_path)) != sfi_.get_hash_includes_last_mod_time().end())
        {
            if(hilmt_itor->second > obj_file_last_modification_time)
            {
                trace_ << "Found newer file than .obj: " << hilmt_itor->first << " -> " << hilmt_itor->second << std::endl;
                return true;
            }
            continue;
        }

        std::string time_as_string;
        time_t tm = boost::filesystem::last_write_time(existing_file_path);
        trace_ << existing_file_path << " last modification time: " << tm << std::endl;
        sfi_.get_hash_includes_last_mod_time()[existing_file_path] = tm;

        if(tm > obj_file_last_modification_time)
        {
            trace_ << "Found newer file than .obj: " << existing_file_path << " -> " << tm << std::endl;
            return true;
        }

        trace_ << "Opening " << existing_file_path << " for reading." << std::endl;
        std::string error;
        std::string file_source;
        if(!FileTools::read_file_source(existing_file_path, file_source, error, strip_comments_))
        {
            if(!error.empty())
                trace_ << error << std::endl;
            continue;
        }

        trace_ << "Getting #includes for " << existing_file_path << std::endl;
        error.clear();
        std::set<std::string> local_hash_includes;
        if(!CompilerTools::get_file_hash_includes(file_source, local_hash_includes, error))
        {
            if(!error.empty())
                trace_ << error << std::endl;
            continue;
        }
        std::copy(hash_includes.begin(), hash_includes.end(), std::ostream_iterator<std::string>(trace_, "\n"));
        trace_ << std::endl;
        if(source_file_needs_recompilation_lmt(obj_file_last_modification_time, folder_includes, local_hash_includes))
            return true;
    }

    return false;
}

void SourceAnalyzer::source_file_needs_recompilation_hash(const std::set<std::string> &folder_includes, 
                                                          std::set<std::string> &hash_includes, 
                                                          std::set<std::string> &hash_includes_hashed)
{
    for(std::set<std::string>::const_iterator hi_itor = hash_includes.begin(); hi_itor != hash_includes.end(); ++hi_itor)
    {
        std::string existing_file_path;
        if(!FileTools::find_path(*hi_itor, folder_includes, existing_file_path))
        {
            trace_ << "Couldn't find path for " << *hi_itor << std::endl;
            continue;
        }

        if(hash_includes_hashed.find(existing_file_path) != hash_includes_hashed.end())
            continue;

        trace_ << "Opening " << existing_file_path << " for reading." << std::endl;
        std::string error;
        std::string file_source;
        if(!FileTools::read_file_source(existing_file_path, file_source, error, strip_comments_))
        {
            if(!error.empty())
                trace_ << error << std::endl;
            continue;
        }

        std::string hash_path_cached;
        std::map<std::string, std::string>::const_iterator ifthm_itor;
        if((ifthm_itor = gi_.include_file_to_hash_map_.find(existing_file_path)) != gi_.include_file_to_hash_map_.end())
        {
            hash_path_cached = ifthm_itor->second;
        }
        else
        {
            boost::hash<std::string> string_hash;
            const size_t hash = string_hash(file_source);

            std::ostringstream ss;
            ss << hash;
            hash_path_cached = ss.str().c_str();
            gi_.include_file_to_hash_map_[existing_file_path] = hash_path_cached;
        }

        hash_includes_hashed.insert(existing_file_path);

        const std::string hash_path_cached_full = gi_.cachedir_ + "\\" + hash_path_cached;
        if(!boost::filesystem::exists(hash_path_cached_full))
            sfi_.get_hash_include_files_to_cache()[hash_path_cached_full] = existing_file_path;

        trace_ << "Getting #includes for " << existing_file_path << std::endl;
        error.clear();
        std::set<std::string> local_hash_includes;
        if(!CompilerTools::get_file_hash_includes(file_source, local_hash_includes, error))
        {
            if(!error.empty())
                trace_ << error << std::endl;
            continue;
        }
        std::copy(hash_includes.begin(), hash_includes.end(), std::ostream_iterator<std::string>(trace_, "\n"));
        trace_ << std::endl;
        source_file_needs_recompilation_hash(folder_includes, local_hash_includes, hash_includes_hashed);
    }
}

bool SourceAnalyzer::prepare_source_information(const std::string &src_file, 
                                                const std::string &compilation_params, 
                                                bool hash_method)
{
    std::string src_filename(src_file);
    StringTools::str_replace(src_filename, "\"", ""); // remove double quotas

    sfi_.set_src_filename(src_filename);
    trace_ << "Source file name and path: " << sfi_.get_src_filename() << std::endl;

    if(!FileTools::is_source_file(sfi_.get_src_filename()))
    {
        trace_ << "Not a source file, skipping " << sfi_.get_src_filename() << std::endl;
        return false;
    }

    trace_ << "Opening " << sfi_.get_src_filename() << " for reading." << std::endl;
    std::string error;
    if(!FileTools::read_file_source(sfi_.get_src_filename(), file_source_, error, strip_comments_))
    {
        if(!error.empty())
            trace_ << error << std::endl;
        return false;
    }

    sfi_.set_last_modification_time(boost::filesystem::last_write_time(sfi_.get_src_filename()));
    trace_ << sfi_.get_src_filename() << " last modification time: " << sfi_.get_last_modification_time() << std::endl;

    std::string to_hash;
    if(hash_method)
    {
        to_hash = sfi_.get_src_filename() + "\n" + file_source_ + "\n" + compilation_params;
    }
    else
    {
        std::ostringstream ss;
        ss << sfi_.get_last_modification_time();
        to_hash = sfi_.get_src_filename() + "\n" + file_source_ + "\n" + ss.str().c_str() + "\n" + compilation_params;
    }

    boost::hash<std::string> string_hash;
    sfi_.set_hash(string_hash(to_hash));

    boost::filesystem::path full_path(sfi_.get_src_filename());
    const std::string file(full_path.filename().string());
    file_path_ = full_path.branch_path().string();

    StringTools::str_replace(file_path_, "\\\\", "\\");
    trace_ << "Separated file name and path: " << file << ", " << file_path_ << std::endl;

    if(sfi_.get_obj_filename().empty())
        sfi_.set_obj_filename(full_path.stem().string() + ".obj");
    trace_ << "Obj file name: " << sfi_.get_obj_filename() << std::endl;

    sfi_.set_hash_obj_filename(sfi_.get_hash_string() + ".obj");
    trace_ << "Hash obj file name: " << sfi_.get_hash_obj_filename() << std::endl;

    return true;
}

bool SourceAnalyzer::prepare_for_analysis(const std::string &rsp_content, 
                                          std::set<std::string> &folder_includes,
                                          std::set<std::string> &hash_includes)
{
    trace_ << "Listing compilation folder includes: " << std::endl;
    std::string error;
    if(!CompilerTools::get_compilation_include_paths(rsp_content, folder_includes, error))
    {
        if(!error.empty())
            trace_ << error << std::endl;
        return false;
    }
    std::copy(folder_includes.begin(), folder_includes.end(), std::ostream_iterator<std::string>(trace_, "\n"));
    trace_ << std::endl;
    folder_includes.insert(file_path_);  // adding source file path

    trace_ << "Getting #includes for " << sfi_.get_src_filename() << std::endl;
    error.clear();
    if(!CompilerTools::get_file_hash_includes(file_source_, hash_includes, error))
    {
        if(!error.empty())
            trace_ << error << std::endl;
        return false;
    }
    std::copy(hash_includes.begin(), hash_includes.end(), std::ostream_iterator<std::string>(trace_, "\n"));
    trace_ << std::endl;

    return true;
}

void SourceAnalyzer::analyze_source_and_dependencies_cached_hash(std::set<std::string> &folder_includes,
                                                            std::set<std::string> &hash_includes)
{
    trace_ << "Starting 'hash' determining method if file needs recompilation." << std::endl;
    std::set<std::string> hash_includes_hashed;
    source_file_needs_recompilation_hash(folder_includes, hash_includes, hash_includes_hashed);

    std::string all_dep_hash = sfi_.get_hash_string();
    std::set<std::string>::const_iterator hih_itor = hash_includes_hashed.begin();
    for(; hih_itor != hash_includes_hashed.end(); ++hih_itor)
        all_dep_hash += *hih_itor;

    boost::hash<std::string> string_hash;
    std::ostringstream ss;
    ss << string_hash(all_dep_hash);
    const std::string all_dep_hash_hashed = gi_.cachedir_ + "\\" + ss.str().c_str();
    trace_ << "Source file with all header dependencies hashed: " << all_dep_hash_hashed << std::endl;

    sfi_.set_file_needs_recompilation(false);
    if(!sfi_.get_hash_include_files_to_cache().empty())
    {
        trace_ << sfi_.get_hash_include_files_to_cache().size() << " header dependency files modified, marked for recompilation." << std::endl;
        sfi_.set_file_needs_recompilation(true);
    }
                
    if(!boost::filesystem::exists(all_dep_hash_hashed))
    {
        trace_ << "Source file with all header dependencies does not exist, marked for recompilation." << std::endl;
        sfi_.set_file_needs_recompilation(true);
        sfi_.get_hash_include_files_to_cache()[all_dep_hash_hashed] = sfi_.get_src_filename();
    }
}

void SourceAnalyzer::analyze_source_and_dependencies_cached_lmt(std::set<std::string> &folder_includes,
                                                            std::set<std::string> &hash_includes)
{
    trace_ << "Starting 'last modified time' determining method if file needs recompilation." << std::endl;
    time_t obj_file_last_modification_time = boost::filesystem::last_write_time(sfi_.get_obj_fullpath_cached_mod());
    trace_ << sfi_.get_obj_fullpath_cached_mod() << " last modified time: " << obj_file_last_modification_time << std::endl;

    sfi_.set_file_needs_recompilation(source_file_needs_recompilation_lmt(obj_file_last_modification_time, folder_includes, hash_includes));
}

void SourceAnalyzer::analyze_source_and_dependencies_hash(std::set<std::string> &folder_includes,
                                                     std::set<std::string> &hash_includes)
{
    std::set<std::string> hash_includes_hashed;
    source_file_needs_recompilation_hash(folder_includes, hash_includes, hash_includes_hashed);

    std::string all_dep_hash = sfi_.get_hash_string();
    std::set<std::string>::const_iterator hih_itor = hash_includes_hashed.begin();
    for(; hih_itor != hash_includes_hashed.end(); ++hih_itor)
        all_dep_hash += *hih_itor;

    boost::hash<std::string> string_hash;
    std::ostringstream ss;
    ss << string_hash(all_dep_hash);
    const std::string all_dep_hash_hashed = gi_.cachedir_ + "\\" + ss.str().c_str();
    trace_ << "Source file with all header dependencies hashed: " << all_dep_hash_hashed << std::endl;
    trace_ << sfi_.get_hash_include_files_to_cache().size() << " header dependency files." << std::endl;

    sfi_.get_hash_include_files_to_cache()[all_dep_hash_hashed] = sfi_.get_src_filename();

    sfi_.set_file_needs_recompilation(true);
}

void SourceAnalyzer::analyze_source_and_dependencies_lmt()
{
    trace_ << "Using 'last modified time' method." << std::endl;
    sfi_.set_file_needs_recompilation(true);
}

}
