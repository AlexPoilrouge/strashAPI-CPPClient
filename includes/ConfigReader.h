#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <string>
#include <map>
#include <memory>
#include <future>
#include <thread>

#include "utils.h"


#include <nlohmann/json.hpp>
#include <jwt/jwt.hpp>
#include <jwt/parameters.hpp>


using json= nlohmann::json;

namespace StrashBot {
namespace API {
    struct TokenHandler;

    struct EndPoint{
        std::string path;
        std::shared_ptr<TokenHandler> token;
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

        inline std::string getEndpointUrl(const std::string& endpointName){
            if(this->endpoints.find(endpointName)==this->endpoints.end()) return "";
            return this->getRootUrl()+'/'+this->endpoints[endpointName].path;
        }
    };


    Infos readFromJsonFile(const std::string& jsonFilePath, bool exceptionThrow= true);

    inline std::future<Infos> async_readFromJsonFile(const std::string& jsonFilePath, bool exceptionThrow= true){
        return std::async(&readFromJsonFile, jsonFilePath, true);
    }

    struct TokenHandler {
        using Payload= std::unordered_map<std::string, std::string>;

        virtual ~TokenHandler() = default;

        virtual std::string signature(Payload payload= {}) const =0;

        static std::shared_ptr<TokenHandler> createFromJSONData(const json& json_data);
    };

    class FromString_TokenHandler : public TokenHandler {
    public:
        inline FromString_TokenHandler(const std::string& jwt_string) : _signature(jwt_string) {}

        ~FromString_TokenHandler() {};

        inline std::string signature(Payload payload= {}) const {return this->_signature;}
    protected:
        std::string _signature;
    };

    class WithKeyString_TokenHandler : public TokenHandler {
    public:
        inline WithKeyString_TokenHandler(const std::string& key_string, const jwt::params::detail::algorithm_param& algorithm= jwt::params::algorithm("RS256"))
            : _key(key_string), _algo(algorithm)
        {}

        std::string signature(Payload payload= {}) const;

    protected:
        std::string _key;
        jwt::params::detail::algorithm_param _algo;
    };

    class WithKeyFile_TokenHandler : public TokenHandler {
    public:
        WithKeyFile_TokenHandler(const std::string& keyfilePath, const jwt::params::detail::algorithm_param& algorithm= jwt::params::algorithm("RS256"));

        std::string signature(Payload payload= {}) const;

    protected:
        std::string _path;
        std::string _key;
        jwt::params::detail::algorithm_param _algo;
    };
}
}


#endif //_CONFIG_READER_H_

