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

    ProcessRunner processRunner(argc, argv);

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
        trace << "vclcache mode: file hashing." << std::endl;
    else
        trace << "vclcache mode: file last modified time." << std::endl;

    
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

    std::string error;
    std::string cmd_content;
    if(argc == 2)
    {
        const std::string rsp_file(std::string(argv[1]).substr(1));
        trace << "Opening " << rsp_file << std::endl;
        if(!FileTools::read_utf16le(rsp_file, cmd_content, error))
        {
            trace << error << std::endl;
            return 1;
        }
        trace << "Rsp file content: " << cmd_content << std::endl;

        CompilerTools::tokenize_params(cmd_content, gi.tokens_);
        trace << "-- Begin tokenized compilation parameters --" << std::endl;
        std::copy(gi.tokens_.begin(), gi.tokens_.end(), std::ostream_iterator<std::string>(*trace, "\n"));
        trace << "-- End tokenized compilation parameters --" << std::endl;
        trace << std::endl;
    }
    else
    {
        for(int x = 1; x < argc; ++x)
            gi.tokens_.push_back(argv[x]);

        trace << "-- Begin tokenized compilation parameters --" << std::endl;
        std::copy(gi.tokens_.begin(), gi.tokens_.end(), std::ostream_iterator<std::string>(*trace, "\n"));
        trace << "-- End tokenized compilation parameters --" << std::endl;
        trace << std::endl;
    }
    trace << std::endl;

    const std::string compilation_params = cmd_content;
    trace << "Compilation parameters: " << compilation_params << std::endl;
    trace << std::endl;

        
    bool strip_comments = true;
    char *pStripComments = std::getenv("VCLCACHE_STRIP_COMMENTS");
    if(pStripComments && boost::iequals(pStripComments, "false"))
        strip_comments = false;

    std::string obj_filename;
    CompilerTools::retrive_obj_path(gi.tokens_, gi.obj_path_, obj_filename);

    
    std::vector<source_file_info> files_info_vec;
    std::vector<std::string>::const_iterator files_itor = std::find_if(gi.tokens_.begin(), gi.tokens_.end(), FileTools::is_source_file);
    for(; files_itor != gi.tokens_.end(); ++files_itor)
    {
        if(!FileTools::is_source_file(*files_itor))
            continue;

        source_file_info sfi(gi);
        if(!obj_filename.empty() && obj_filename.find(".obj") != std::string::npos)
            sfi.set_obj_filename(obj_filename);
        SourceAnalyzer srcAnalyzer(*trace, gi, sfi, strip_comments);

        if(!srcAnalyzer.prepare_source_information(*files_itor, compilation_params, use_hash))
            continue;

        if(boost::filesystem::exists(sfi.get_obj_fullpath_cached_mod()))
        {
            trace << "Cached .obj file exist: " << sfi.get_obj_fullpath_cached_mod() << std::endl;

            std::set<std::string> folder_includes;
            std::set<std::string> hash_includes;
            if(!srcAnalyzer.prepare_for_analysis(cmd_content, folder_includes, hash_includes))
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
                if(!srcAnalyzer.prepare_for_analysis(cmd_content, folder_includes, hash_includes))
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

