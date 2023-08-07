#ifndef UTILS_H
#define UTILS_H


#include <boost/beast/http.hpp>

#include <iostream>
#include <filesystem>


namespace StrashBot{
namespace API{

using RequestVerb = boost::beast::http::verb;


inline bool fileExists(const std::string& path){
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

}
}

#endif // UTILS_H
