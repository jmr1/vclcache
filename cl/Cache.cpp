#include "Cache.h"

#include <boost/filesystem.hpp>
#include <Poco/Process.h>
#include <Poco/Pipe.h>

#include "source_file_info.h"

namespace vclcache {

bool Cache::use_cache(std::vector<source_file_info> &files_info_vec)
{
    bool cache_used = true;
    for(std::vector<source_file_info>::iterator file_itor = files_info_vec.begin(); file_itor != files_info_vec.end(); ++file_itor)
    {
        if(!file_itor->get_file_needs_recompilation())
        {
            trace_ << "File " << file_itor->get_obj_fullpath_cached_mod() << " [ " << file_itor->get_obj_fullpath_cached() << " ] " << " exist, using cache." << std::endl;
            boost::filesystem::copy_file(file_itor->get_obj_fullpath_cached_mod(), file_itor->get_obj_fullpath(), boost::filesystem::copy_option::overwrite_if_exists);

            if(boost::filesystem::last_write_time(file_itor->get_obj_fullpath()) < file_itor->get_last_modification_time())
            {
                trace_ << "Last modification time of " << file_itor->get_obj_fullpath_cached() << " is older than " << file_itor->get_src_filename() << ", updating timestamp." << std::endl;
                boost::filesystem::last_write_time(file_itor->get_obj_fullpath(), time(NULL));
            }

            file_itor->set_cache_used(true);
        }
        else
        {
            cache_used = false;
            trace_ << "File " << file_itor->get_obj_fullpath_cached_mod() << " [ " << file_itor->get_obj_fullpath_cached() << " ] " << " does not exist, need to compile" << std::endl;
        }
    }

    return cache_used;
}


void Cache::cache_files(std::vector<source_file_info> &files_info_vec, bool use_hash)
{
    for(std::vector<source_file_info>::iterator file_itor = files_info_vec.begin(); file_itor != files_info_vec.end(); ++file_itor)
    {
        if(boost::filesystem::exists(file_itor->get_obj_fullpath()))
        {
            trace_ << "Obj file " << file_itor->get_obj_fullpath() << " exist, caching as " << file_itor->get_obj_fullpath_cached_mod();
            boost::filesystem::copy_file(file_itor->get_obj_fullpath(), file_itor->get_obj_fullpath_cached(), boost::filesystem::copy_option::overwrite_if_exists);
            boost::filesystem::rename(file_itor->get_obj_fullpath_cached(), file_itor->get_obj_fullpath_cached_mod());
            trace_ << " : done." << std::endl;

            if(use_hash && !file_itor->get_hash_include_files_to_cache().empty())
            {
                std::map<std::string, std::string>::const_iterator hiftc_itor = file_itor->get_hash_include_files_to_cache().begin();
                for(; hiftc_itor != file_itor->get_hash_include_files_to_cache().end(); ++hiftc_itor)
                {
                    if(!boost::filesystem::exists(hiftc_itor->first))
                    {
                        trace_ << "Creating new hash file " << hiftc_itor->first << " for " << hiftc_itor->second;
                        const std::string tmp_name(hiftc_itor->first + ".tmp");
                        std::ofstream f(tmp_name);
                        f.close();
                        boost::filesystem::rename(tmp_name, hiftc_itor->first);
                        trace_ << " : done." << std::endl;
                    }
                }
            }
        }
        else
        {
            trace_ << "File " << file_itor->get_obj_fullpath() << " does not exist, couldn't cache." << std::endl;
        }
    }

}

}