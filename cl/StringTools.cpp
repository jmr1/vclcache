#include "StringTools.h"

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

namespace vclcache {

/*static*/ void StringTools::str_replace(std::string &s, const std::string &search, const std::string &replace) 
{
    for(size_t pos = 0; ; pos += replace.length()) 
	{
        pos = s.find(search, pos);
        if(pos == std::string::npos) 
            break;

        s.erase(pos, search.length());
        s.insert(pos, replace);
    }
}

/*static*/ void StringTools::tokenize(const std::string &input, const std::string &separator, std::vector<std::string> &output)
{
    boost::char_separator<char> sep(separator.c_str());
    typedef boost::tokenizer< boost::char_separator<char> > tokenizer;
    tokenizer tok(input, sep);
    for(tokenizer::iterator beg = tok.begin(); beg != tok.end(); ++beg)
    {
        std::string tmp(*beg);
        boost::trim(tmp);
        output.push_back(tmp);
    }
}

/*static*/ void StringTools::tokenize2(const std::string &input, const std::string &separator, std::vector<std::string> &output)
{
    boost::algorithm::split(output, input, boost::algorithm::is_any_of(separator));
}

}