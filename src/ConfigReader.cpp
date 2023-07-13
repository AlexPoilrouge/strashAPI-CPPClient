#include "../includes/ConfigReader.h"

#include <iostream>
#include <fstream>

#include <nlohmann/json.hpp>

namespace api= StrashBot::API;

using json= nlohmann::json;

#define KEYNAME_API_INFO "api-info"
#define KEYNAME_ENDPOINTS "endpoints"
#define KEYNAME_TOKEN "token"
#define KEYNAME_PATH "path"
#define KEYNAME_METHOD "method"

void _basicInfosSet(api::Infos& infos, const json& j){
    std::string url= j[KEYNAME_API_INFO]["url"];
    std::string root= "";

    infos.base_url= std::move(url);

    if(j[KEYNAME_API_INFO].contains("root"))
        infos.root= j[KEYNAME_API_INFO]["root"];

    if(j[KEYNAME_API_INFO].contains("port"))
        infos.port= j[KEYNAME_API_INFO]["port"];

    if(j[KEYNAME_API_INFO].contains("ssl")){
        infos.ssl= j[KEYNAME_API_INFO]["ssl"];
    }

    if(infos.base_url.rfind("http://",0)==0){
        infos.port= infos.port? infos.port : 80;
        infos.base_url= infos.base_url.substr(7);
    }
    else if(infos.base_url.rfind("https://",0)==0){
        infos.port= infos.port? infos.port : 443;
        infos.base_url= infos.base_url.substr(8);
        infos.ssl = true;
    }
}

void _endpointsInfoSet(api::Infos& infos, const json& j){
    if(j.contains(KEYNAME_ENDPOINTS)){
        json jep= j[KEYNAME_ENDPOINTS];

        std::shared_ptr<std::string> main_token;
        if(jep.contains(KEYNAME_TOKEN)){
            main_token= std::make_shared<std::string>(jep[KEYNAME_TOKEN]);
        }
        api::RequestVerb verb= api::RequestVerb::get;
        
        for(auto& [key, value]: jep.items()){
            if(key==KEYNAME_TOKEN || key==KEYNAME_METHOD ||
                (!value.is_object()) || (!value.contains(KEYNAME_PATH))
            )
                continue;

            api::EndPoint ep;
            ep.path= value[KEYNAME_PATH];

            if(value.contains(KEYNAME_TOKEN)){
                ep.token= std::make_shared<std::string>(value[KEYNAME_TOKEN]);
            }
            else{
                ep.token= main_token;
            }

            if(value.contains(KEYNAME_METHOD)){
                std::string methodStr= value[KEYNAME_METHOD];
                std::transform(methodStr.begin(), methodStr.end(), methodStr.begin(), ::toupper);
                verb=
                        (methodStr=="PUT")      ?   api::RequestVerb::put
                    :   (methodStr=="POST")     ?   api::RequestVerb::post
                    :   (methodStr=="DELETE")   ?   api::RequestVerb::delete_
                    :                               api::RequestVerb::get;
            }
            ep.verb= verb;

            infos.endpoints.insert({key, ep});
        }
    }
}

api::Infos api::readFromJsonFile(const std::string& jsonFilePath, bool exceptionThrow){
    std::ifstream ifs(jsonFilePath);
    json jf= json::parse(ifs);

    api::Infos infos;
    if(!exceptionThrow){
        try{
            _basicInfosSet(infos, jf);

            _endpointsInfoSet(infos, jf);
        }
        catch (const std::exception& e){
            std::cout   << "Error reading basic infos for StrashBot::API from setting file '"
                        << jsonFilePath << "' : " << e.what() << '\n';
        }
    }
    else{
        _basicInfosSet(infos, jf);

        _endpointsInfoSet(infos, jf);
    }

    return infos;
}


std::pair<std::string, std::string> api::Infos::getEndpointUrl(const std::string& endpointName){
    if(this->endpoints.find(endpointName)==this->endpoints.end()) return std::make_pair("","");

    EndPoint ep= this->endpoints[endpointName];

    return std::make_pair(
        this->getRootUrl()+'/'+ep.path,
        (ep.token)?*ep.token:""
    );
}

void _invoke_readFromJsonFile(std::promise<api::Infos>&& inPromise, const std::string& jsonFilePath){
    inPromise.set_value(api::readFromJsonFile(jsonFilePath));
}
