#include "Query.h"
#include "QueryFactory.h"
#include "ApiClient.h"
#include "ConfigReader.h"
#include "errors.h"


namespace api= StrashBot::API;


using json= nlohmann::json;

unsigned int api::Request::id= 0;


std::string api::Request::queryParametersString() const{
    std::string ret;
    if(!this->_queryParams.empty()){
        ret= "?";
        bool start= true;
        for(const auto& kv : this->_queryParams){
            ret+= (start?"":"&") + kv.first + '=' + kv.second;
            start= false;
        }
    }
    return ret;
}


std::shared_ptr<api::Request> api::RequestFactory::create(boost::beast::http::verb verb){
    return std::make_shared<api::Request>(verb);
}


std::shared_ptr<api::Request> api::RequestFactory::retry(const api::Request& r){
    return std::make_shared<api::Request>(r);
}

api::QueryPair api::QueryPairFactory::create(
    boost::beast::http::verb verb,
    std::shared_ptr< std::function< void(const api::Response& response) > > f
){
    auto created_req= api::RequestFactory::create(verb);
    auto created_resp= std::shared_ptr<ResponseHandler>(new api::ResponseHandler(f));

    created_req->setResonseHandler(created_resp);

    return std::make_pair(
            created_req,
            created_resp
        );
}

boost::beast::http::request<boost::beast::http::string_body> api::Request::getBeastRequest(const api::Infos& infos) const{
    const std::string endpointName= this->getEndpoint();
    const api::EndPoint& ep= infos.endpoints.at(endpointName);
    std::string rootPath= "/"+((infos.root.empty())?ep.path:(infos.root+'/'+ep.path))+this->queryParametersString();

    RequestVerb verb= this->getVerb();
    if(verb==RequestVerb::unknown){
        if(ep.verb!=RequestVerb::unknown){
            verb= ep.verb;
        }
        else {
            std::cerr << "Unknown verb for request " << this->getRequestID() << " on endpoint '" << endpointName << "', assuming 'get' …" << std::endl;
            verb= RequestVerb::get;
        }
    }

    boost::beast::http::request<boost::beast::http::string_body> beastreq(
        verb,
        rootPath,
        11
    );

    json body_copy= this->_body;

    if(ep.token){
        auto tokenPayload= this->_tokenPayload;
        if(tokenPayload.find("auth")==tokenPayload.end()){
            tokenPayload["auth"]= "StrashAPIClient_cpp";
        }

        std::string sig= "bad_token";

        switch(this->getTokenChannel()){
        case HEADER:
            try{
                beastreq.set("x-access-token", ep.token->signature(tokenPayload));
            }
            catch(std::exception e){
                throw api::TokenGetSignatureError(endpointName, e);
            }

            break;
        case QUERY_PARAM:
            try{
                sig= ep.token->signature(tokenPayload);
            }
            catch(std::exception e){
                throw api::TokenGetSignatureError(endpointName, e);
            }

            rootPath+=
                ((rootPath.find('?') != std::string::npos)?
                        std::string("&")
                    :   std::string("?")
                         ) + "token="+sig;

            break;
        case BODY:
        default:
            try{
                body_copy["token"]= ep.token->signature(tokenPayload);
            }
            catch(std::exception e){
                throw api::TokenGetSignatureError(endpointName, e);
            }

            break;
        }
    }

    beastreq.set(boost::beast::http::field::host, infos.base_url);
    beastreq.set(boost::beast::http::field::user_agent, CLIENT_STRASHAPI_USERAGENT_STRING);
    beastreq.set(boost::beast::http::field::content_type, "application/json");

    for(const auto& kv : this->_headers){
        beastreq.set(kv.first, kv.second);
    }

    beastreq.body()= body_copy.empty()?"{}":body_copy.dump();

    beastreq.prepare_payload();

    return beastreq;
}


