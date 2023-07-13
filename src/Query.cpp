#include "Query.h"
#include "QueryFactory.h"
#include "ApiClient.h"
#include "ConfigReader.h"


namespace api= StrashBot::API;


using json= nlohmann::json;

unsigned int api::Request::id= 0;


api::Request::Body& api::Request::Body::operator<<(const json& json){
    this->parent->body_str= json.dump();
    return (*this);
}

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
    const api::EndPoint& ep= infos.endpoints.at(this->getEndpoint());
    const std::string rootPath= "/"+((infos.root.empty())?ep.path:(infos.root+'/'+ep.path))+this->queryParametersString();

    boost::beast::http::request<boost::beast::http::string_body> beastreq(
        this->getVerb(),
        rootPath,
        11
    );

    beastreq.set(boost::beast::http::field::host, infos.base_url);
    beastreq.set(boost::beast::http::field::user_agent, CLIENT_STRASHAPI_USERAGENT_STRING);
    beastreq.set(boost::beast::http::field::content_type, "application/json");

    if(ep.token && !ep.token->empty()){
        beastreq.set("x-access-token", *ep.token);
    }

    for(const auto& kv : this->_headers){
        beastreq.set(kv.first, kv.second);
    }

    beastreq.body()= body_str;

    beastreq.prepare_payload();

    return beastreq;
}


