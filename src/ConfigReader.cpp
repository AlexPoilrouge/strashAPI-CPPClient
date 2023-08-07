#include "../includes/ConfigReader.h"

#include "../includes/errors.h"

#include <iostream>
#include <fstream>


namespace api= StrashBot::API;

#define KEYNAME_API_INFO "api-info"
#define KEYNAME_ENDPOINTS "endpoints"
#define KEYNAME_TOKEN "token"
#define KEYNAME_PATH "path"
#define KEYNAME_METHOD "method"
#define KEYNAME_TOKEN_SIGNATURE "signature"
#define KEYNAME_TOKEN_ALGORITHM "algorithm"
#define KEYNAME_TOKEN_KEY_STRING "key"
#define KEYNAME_TOKEN_KEY_FILE "key_file"

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

        std::shared_ptr<api::TokenHandler> main_token;
        if(jep.contains(KEYNAME_TOKEN)){
            try{
                main_token= api::TokenHandler::createFromJSONData(jep[KEYNAME_TOKEN]);
            }
            catch(api::BadTokenKeyFilePathForJWT e){
                throw e;
            }
            catch(std::exception e){
                throw api::TokenReadingError(e);
            }
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
                try{
                    ep.token= api::TokenHandler::createFromJSONData(value[KEYNAME_TOKEN]);
                }
                catch(api::BadTokenKeyFilePathForJWT e){
                    throw e;
                }
                catch(std::exception e){
                    throw api::TokenReadingError(e);
                }
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
    if((!api::fileExists(jsonFilePath)) && exceptionThrow){
        throw api::NonExistantConfigFileException(jsonFilePath);
    }

    api::Infos infos;

    try{
        std::ifstream ifs(jsonFilePath);
        json jf= json::parse(ifs);

        _basicInfosSet(infos, jf);

        _endpointsInfoSet(infos, jf);
    }
    catch(json::exception e){
        if(exceptionThrow) throw api::ConfigJSONParsingError(e, jsonFilePath);
        else std::cerr  << "Error parsin basic JSON infos for StrashBot::API from setting file '"
                        << jsonFilePath << "' : " << e.what() << '\n';
    }
    catch (api::BadTokenKeyFilePathForJWT e){
        if(exceptionThrow) throw e;
        else std::cerr  << "Error reading token infos for StrashBot::API from setting file '"
                      << jsonFilePath << "' : " << e.what() << '\n';
    }
    catch (std::exception e){
        if(exceptionThrow) throw api::BadReadConfigFileException(e, jsonFilePath);
        else std::cerr  << "Error reading basic infos for StrashBot::API from setting file '"
                      << jsonFilePath << "' : " << e.what() << '\n';
    }

    return infos;
}

void _invoke_readFromJsonFile(std::promise<api::Infos>&& inPromise, const std::string& jsonFilePath){
    inPromise.set_value(api::readFromJsonFile(jsonFilePath));
}


std::string api::WithKeyString_TokenHandler::signature(api::TokenHandler::Payload payload) const {
    jwt::jwt_object obj {this->_algo, jwt::params::payload(payload), jwt::params::secret(this->_key)};

    return obj.signature();
}

api::WithKeyFile_TokenHandler::WithKeyFile_TokenHandler(const std::string& keyfilePath, const jwt::params::detail::algorithm_param& algorithm)
    : _path(keyfilePath), _algo(algorithm)
{
    if((!api::fileExists(keyfilePath))){
        throw api::BadTokenKeyFilePathForJWT(keyfilePath);
    }

    std::ifstream ifs(this->_path);
    this->_key.assign( (std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()) );
}

std::string api::WithKeyFile_TokenHandler::signature(api::TokenHandler::Payload payload) const {
    jwt::jwt_object obj {this->_algo, jwt::params::payload(payload), jwt::params::secret(this->_key)};

    return obj.signature();
}

std::shared_ptr<api::TokenHandler> api::TokenHandler::createFromJSONData(const json& json_data) {
    jwt::params::detail::algorithm_param algo {"RS256"};
    if(json_data.contains(KEYNAME_TOKEN_ALGORITHM)){
        algo= jwt::algorithm {json_data[KEYNAME_TOKEN_ALGORITHM]};
    }
    if(json_data.is_string()){
        return std::shared_ptr<api::TokenHandler>(
            new api::FromString_TokenHandler(json_data.get<const json::string_t*>()->c_str())
        );
    }
    else if(json_data.contains(KEYNAME_TOKEN_SIGNATURE)){
        return std::shared_ptr<api::TokenHandler>(
            new api::FromString_TokenHandler(json_data[KEYNAME_TOKEN_SIGNATURE])
        );
    }
    else if(json_data.contains(KEYNAME_TOKEN_KEY_STRING)){
        return std::shared_ptr<api::TokenHandler>(
            new api::WithKeyString_TokenHandler(
                json_data[KEYNAME_TOKEN_KEY_STRING],
                algo
            )
        );
    }
    else if(json_data.contains(KEYNAME_TOKEN_KEY_FILE)){
        return std::shared_ptr<api::TokenHandler>(
            new api::WithKeyFile_TokenHandler(
                json_data[KEYNAME_TOKEN_KEY_FILE],
                algo
            )
        );
    }
    else return nullptr;
}
