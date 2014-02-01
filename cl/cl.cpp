#include <iostream>
#include <fstream>
#include <sstream>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "string_finder_pred.h"
#include "CompilerTools.h"
#include "FileTools.h"
#include "global_info.h"
#include "source_file_info.h"
#include "Logger.h"
#include "SourceAnalyzer.h"
#include "Cache.h"
#include "ProcessRunner.h"

using namespace vclcache;


#ifdef _DEBUG
#define USE_CONSOLE
#endif

int main(int argc, char** argv)
{
#ifndef USE_CONSOLE
    std::ofstream myfile;
    myfile.open("vclcache.log", std::ofstream::out|std::ofstream::app);
 
    Logger<std::ofstream> trace(myfile);
#else
    Logger<std::ostream> trace(std::cout);
#endif

    trace << "Current directory: " << boost::filesystem::current_path().string() << std::endl;
    trace << "Listing program arguments:" << std::endl;
    for(int x = 0; x < argc; ++x)
        trace << argv[x] << std::endl;
    trace << std::endl;

    if(argc < 2)
    {
        trace << "Error: not enough program arguments - " << argc << ". Exiting" << std::endl;
        return 1;
    }

    ProcessRunner processRunner(argv[1]);

    char *pCacheOff = std::getenv("VCLCACHE_OFF");
    if(pCacheOff && boost::iequals(pCacheOff, "true"))
    {
        trace << "VCLCACHE_OFF=true, will not be using cache, invoking cl_real.exe" << std::endl;
        processRunner.run("cl_real.exe");
        return 0;
    }

    bool use_hash = true;
    char *pCacheMode = std::getenv("VCLCACHE_MODE");
    if(pCacheMode && boost::iequals(pCacheMode, "timestamp"))
        use_hash = false;
    
    if(use_hash)
        trace << "CLcache mode: file hashing." << std::endl;
    else
        trace << "CLcache mode: file last modified time." << std::endl;

    std::string error;
    std::string rsp_content;
    const std::string rsp_file(std::string(argv[1]).substr(1));
    trace << "Opening " << rsp_file << std::endl;
    if(!FileTools::read_utf16le(rsp_file, rsp_content, error))
    {
        trace << error << std::endl;
        return 1;
    }
    trace << "Rsp file content: " << rsp_content << std::endl;
    trace << std::endl;

	std::string cachedir("C:\\.cache");
	char *pCacheDir = std::getenv("VCLCACHE_DIR");
	if(pCacheDir)
        cachedir = pCacheDir;

    trace << "Cache directory set to " << cachedir << std::endl;
    if(!boost::filesystem::exists(cachedir))
    {
        trace << "Cache directory does not exist. Creating in " << cachedir << std::endl;
        if(!boost::filesystem::create_directories(cachedir))
        {
            trace << "Error: failed creating cache directory in " << cachedir << ". Exiting." << std::endl;
            return 1;
        }
        trace << std::endl;
    }

    global_info gi;
    gi.cachedir_ = cachedir;

    // Substracting compilation parameters
    size_t pos = rsp_content.find("errorReport");
    if(pos != std::string::npos)
        pos = rsp_content.find(" ", pos);
    if(pos == std::string::npos)
    {
        trace << "Error: couldn't find errorReport parameter. Exiting." << std::endl;
        return 1;
    }
    const std::string compilation_params = rsp_content.substr(0, pos);
    trace << "Compilation parameters: " << compilation_params << std::endl;
    trace << std::endl;

        
    CompilerTools::tokenize_params(rsp_content, gi.tokens_);
    trace << "-- Begin tokenized compilation parameters --" << std::endl;
    std::copy(gi.tokens_.begin(), gi.tokens_.end(), std::ostream_iterator<std::string>(*trace, "\n"));
    trace << "-- End tokenized compilation parameters --" << std::endl;
    trace << std::endl;

    CompilerTools::retrive_obj_path(gi.tokens_, gi.obj_path_);

    
    std::vector<source_file_info> files_info_vec;
    std::vector<std::string>::const_iterator files_itor = std::find_if(gi.tokens_.begin(), gi.tokens_.end(), string_finder_pred("errorReport"));
    ++files_itor;
    for(; files_itor != gi.tokens_.end(); ++files_itor)
    {
        source_file_info sfi(gi);
        SourceAnalyzer srcAnalyzer(*trace, gi, sfi);

        if(!srcAnalyzer.prepare_source_information(*files_itor, compilation_params, use_hash))
            continue;

        if(boost::filesystem::exists(sfi.get_obj_fullpath_cached_mod()))
        {
            trace << "Cached .obj file exist: " << sfi.get_obj_fullpath_cached_mod() << std::endl;

            std::set<std::string> folder_includes;
            std::set<std::string> hash_includes;
            if(!srcAnalyzer.prepare_for_analysis(rsp_content, folder_includes, hash_includes))
                continue;

            if(use_hash)
                srcAnalyzer.analyze_source_and_dependencies_cached_hash(folder_includes, hash_includes);
            else
                srcAnalyzer.analyze_source_and_dependencies_cached_lmt(folder_includes, hash_includes);
        }
        else
        {
            trace << "Cached .obj file does NOT exist: " << sfi.get_obj_fullpath_cached_mod() << std::endl;
            if(use_hash)
            {
                trace << "Using 'hash' method." << std::endl;

                std::set<std::string> folder_includes;
                std::set<std::string> hash_includes;
                if(!srcAnalyzer.prepare_for_analysis(rsp_content, folder_includes, hash_includes))
                    continue;

                srcAnalyzer.analyze_source_and_dependencies_cached_hash(folder_includes, hash_includes);
            }
            else
            {
                srcAnalyzer.analyze_source_and_dependencies_lmt();
            }
        }

        if(sfi.get_file_needs_recompilation())
            trace << sfi.get_src_filename() << " marked for compilation." << std::endl;
        else
            trace << sfi.get_src_filename() << " NOT marked for compilation." << std::endl;
        trace << std::endl;

        files_info_vec.push_back(sfi);
    }

    Cache cache(*trace);
    cache.use_cache(files_info_vec);
    cache.cache_files(files_info_vec, use_hash, processRunner);

    trace << std::endl;
#ifndef USE_CONSOLE
    myfile.close();
#endif

	return 0;
}

