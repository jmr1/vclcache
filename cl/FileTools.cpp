#include "FileTools.h"

#include <sstream>
#include <boost/filesystem.hpp>
#include <boost/wave.hpp>

#include "StringTools.h"
#include "CompilerTools.h"


namespace vclcache {

/*static*/ bool FileTools::read_file_source(const std::string &src_file_path, std::string &file_source, std::string &error, bool strip_comments)
{
    std::ostringstream ss;
        
    if(!boost::filesystem::exists(src_file_path))
    {
        ss << "Error: " << src_file_path << " file does not exist." << std::endl;
        error = ss.str().c_str();
        return false;
    }

    std::ifstream file_source_in(src_file_path, std::ios::in|std::ios::binary);
    if(!file_source_in.is_open())
    {
        ss << "Error openning file  " << src_file_path << std::endl;
        error = ss.str().c_str();
        return false;
    }

	ss << file_source_in.rdbuf() << '\0';
    file_source_in.close();

    file_source = ss.str().c_str();

    if(strip_comments)
    {
        try
        {
            file_source += "\n"; //add new line to avoid parser error (Unterminated C++ style comment)
            file_source = CompilerTools::strip_comments(file_source);
        }
        catch(boost::wave::cpplexer::lexing_exception &e)
        {
            ss.clear();
            ss.str("");
            ss << e.what() << " - " << e.description() << " - " << src_file_path <<  ":" << e.line_no() << ":" << e.column_no() << std::endl;
            error = ss.str().c_str();
            return false;
        }
        catch(boost::wave::cpplexer::cpplexer_exception &e)
        {
            ss.clear();
            ss.str("");
            ss << e.what() << " - " << e.description() << " - " << src_file_path <<  ":" << e.line_no() << ":" << e.column_no() << std::endl;
            error = ss.str().c_str();
            return false;
        }
        catch(std::exception &e)
        {
            ss.clear();
            ss.str("");
            ss << e.what() << std::endl;
            error = ss.str().c_str();
            return false;
        }
    }
        
    return true;
}

/*static*/ bool FileTools::find_path(const std::string &filename, const std::set<std::string> &folder_includes, std::string &output_path)
{
    for(std::set<std::string>::const_iterator fi_itor = folder_includes.begin(); fi_itor != folder_includes.end(); ++fi_itor)
    {
        std::string path(*fi_itor + "\\" + filename);
        StringTools::str_replace(path, "\\\\", "\\");
        if(boost::filesystem::exists(path))
        {
            output_path = path;
            break;
        }
    }

    if(output_path.empty())
        return false;

    return true;
}

/*static*/ bool FileTools::is_source_file(const std::string &src_file_path)
{
    // TODO: make sure src_file_path is really a path, then enable this
    //const boost::filesystem::path file_path(src_file_path);
    //const std::string ext(file_path.extension().string());
    
    //return ext == ".cpp" || ext == ".c" || ext == ".cc";
    return src_file_path.find(".cpp") != std::string::npos || src_file_path.find(".c") != std::string::npos || src_file_path.find(".cc") != std::string::npos;
}

/*static*/ bool FileTools::read_utf16le(const std::string &path, std::string &content, std::string &error)
{
    // reading the .rsp file
    std::stringstream ss;
    std::ifstream rsp_in(path, std::ios::binary|std::ios::in);
    if(!rsp_in.is_open())
    {
        ss << "Error opening file " << path << "! Exiting." << std::endl;
        error = ss.str().c_str();
        return false;
    }

	ss << rsp_in.rdbuf() << '\0';

	std::wstring ws((wchar_t*)ss.str().c_str());
    content = std::string(ws.begin()+1, ws.end());

    // End of reading the .rsp file
    rsp_in.close();

    return true;
}

}