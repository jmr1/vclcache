#include "CompilerTools.h"

#include <boost/regex.hpp>
#include <boost/wave/cpplexer/cpp_lex_token.hpp>
#include <boost/wave/cpplexer/cpp_lex_iterator.hpp>
#include <boost/filesystem.hpp>

#include "string_finder_pred.h"
#include "StringTools.h"
#include "FileTools.h"

namespace vclcache {

/*static*/ std::string CompilerTools::strip_comments(std::string const& input) 
{
    std::string output;
    typedef boost::wave::cpplexer::lex_token<> token_type;
    typedef boost::wave::cpplexer::lex_iterator<token_type> lexer_type;
    typedef token_type::position_type position_type;

    position_type pos;

    lexer_type it = lexer_type(input.begin(), input.end(), pos, 
        boost::wave::language_support(
            boost::wave::support_cpp|boost::wave::support_option_long_long));
        
    lexer_type end = lexer_type();
    for (;it != end; ++it) 
        if (*it != boost::wave::T_CCOMMENT && *it != boost::wave::T_CPPCOMMENT) 
            output += std::string(it->get_value().begin(), it->get_value().end());

    return output;
}

/*static*/ void CompilerTools::tokenize_params(const std::string &rsp_content, std::vector<std::string> &tokens)
{
    StringTools::tokenize2(rsp_content, " ", tokens);
}

/*static*/ void CompilerTools::retrive_obj_path(const std::vector<std::string> &tokens, std::string &obj_path, std::string &obj_file)
{
    std::string path;
    std::vector<std::string>::const_iterator fo_itor = std::find_if(tokens.begin(), tokens.end(), string_finder_pred("/Fo"));
    if(fo_itor != tokens.end())
        path = ((*fo_itor).substr(3));

    StringTools::str_replace(path, "\\\\", "\\");
    StringTools::str_replace(path, "//", "//");
    StringTools::str_replace(path, "\"", "");

    boost::filesystem::path full_path(path);
    obj_path = full_path.branch_path().string();
    obj_file = full_path.filename().string();
}

/*static*/ bool CompilerTools::get_compilation_include_paths(const std::string &input, std::set<std::string> &folder_includes, std::string &error)
{
    try
    {
        boost::regex folder_include_regex("/I"
            "\\\"?"
            "([A-Za-z0-9\\.\\\\/_+-]*)"
            "\\\"?");
        boost::smatch what;
        std::string::const_iterator start = input.begin();
        std::string::const_iterator end = input.end();
        while (boost::regex_search(start, end, what, folder_include_regex))
        {
            std::string s(what[1].first, what[1].second);
            StringTools::str_replace(s, "\\\\", "\\");
            folder_includes.insert(s);
            // Update the beginning of the range to the character following the whole match
            start = what[0].second;
        }
    }
    catch(std::exception &e)
    {
        error = e.what();
        return false;
    }

    return true;
}

/*static*/ bool CompilerTools::get_file_hash_includes(const std::string &file_source, std::set<std::string> &hash_includes, std::string &error)
{
    try
    {
        boost::regex include_regex("^[[:space:]]*" // possibly leading whitespace
            "#include"
            "[[:space:]]*" // possibly whitespace
            "\\\""
            "[[:space:]]*" // possibly whitespace
            "([A-Za-z0-9\\.\\\\/_+-]*)"
            "[[:space:]]*" // possibly whitespace
            "\\\"");

        boost::smatch what;
        std::string::const_iterator start = file_source.begin();
        std::string::const_iterator end = file_source.end();
        while (boost::regex_search(start, end, what, include_regex))
        {
            std::string s(what[1].first, what[1].second);
            StringTools::str_replace(s, "\\\\", "\\");
            hash_includes.insert(s);
            // Update the beginning of the range to the character following the whole match
            start = what[0].second;
        }
    }
    catch(std::exception &e)
    {
        error = e.what();
        return false;
    }

    return true;
}

/*static*/ void CompilerTools::get_file_hash_includes(const std::string &file_source, std::set<std::string> &hash_includes)
{
    typedef boost::wave::cpplexer::lex_token<> token_type;
    typedef boost::wave::cpplexer::lex_iterator<token_type> lexer_type;
    typedef token_type::position_type position_type;

    position_type pos;

    lexer_type it = lexer_type(file_source.begin(), file_source.end(), pos, 
        boost::wave::language_support(
            boost::wave::support_cpp|boost::wave::support_option_long_long));

    lexer_type end = lexer_type();
    for (;it != end; ++it) 
        if (*it == boost::wave::T_PP_INCLUDE) 
            hash_includes.insert(std::string(it->get_value().begin(), it->get_value().end()));
}

}
