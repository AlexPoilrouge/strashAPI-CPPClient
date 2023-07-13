#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <string>
#include <map>
#include <memory>
#include <future>
#include <thread>

#include "utils.h"


namespace StrashBot {
namespace API {

    struct EndPoint{
        std::string path;
        std::shared_ptr<std::string> token; //same token can be shared across endpoints
        RequestVerb verb;

        inline bool operator==(const EndPoint& ep) const {
            return this->path==ep.path && this->token==ep.token && this->verb==ep.verb;
        }
    };

    struct Infos {
        std::string base_url= "strashbot.fr";
        std::string root= "";
        unsigned int port= 0;
        bool ssl= false;

        std::map<std::string,EndPoint> endpoints;

        inline std::string getServerBaseUrl(){
            return base_url+(port?(":"+std::to_string(port)):"");
        }
        inline std::string getRootUrl(){
            return this->getServerBaseUrl()+(root.empty()?"":("/"+root));
        }

        std::pair<std::string, std::string> getEndpointUrl(const std::string& endpointName);
    };


    Infos readFromJsonFile(const std::string& jsonFilePath, bool exceptionThrow= true);

    inline std::future<Infos> async_readFromJsonFile(const std::string& jsonFilePath, bool exceptionThrow= true){
        return std::async(&readFromJsonFile, jsonFilePath, true);
    }
}
}


#endif //_CONFIG_READER_H_

