#ifndef QUERY_H
#define QUERY_H

#include <nlohmann/json.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <fstream>
#include <iostream>
#include <map>


using json= nlohmann::json;

namespace StrashBot {
namespace API {

class RequestFactory;
class Client;
class Infos;
class ThreadPool;

class Request;
class ResponseHandler;


struct Response {
    json responseBody;
    unsigned int status_code;
    std::shared_ptr<Request> spawner;

    static inline Response create(std::string bodyString, unsigned int status_code, std::shared_ptr<Request> req){
        return {json::parse(bodyString), status_code, req};
    }

    inline bool isClientError() const {
        return responseBody.contains("client_error");
    }
};


class Request {
    friend Client;
    friend RequestFactory;
    friend ThreadPool;
public:
    enum TokenChannel {
        BODY,
        HEADER,
        QUERY_PARAM
    };
protected:
    using QueryParameters= std::map<std::string, std::string>;
    using Headers= std::map<std::string, std::string>;
    using TokenPayload= std::unordered_map<std::string, std::string>;

private:
    QueryParameters _queryParams;
    Headers _headers;
    TokenPayload _tokenPayload;
    TokenChannel _tokenChannel= TokenChannel::BODY;
    json _body;
    std::string endpoint;

    unsigned int request_id= 0;
    unsigned int tryNum= 1;

    static unsigned int id;

public:
    inline Request(boost::beast::http::verb verb= boost::beast::http::verb::get) : verb(verb), request_id(++Request::id) {}
    inline Request(const Request& r) = default;

    inline json& body(){return this->_body;}
    inline QueryParameters& queryParameters(){return this->_queryParams;}
    inline Headers& headers(){return this->_headers;}
    inline TokenPayload& tokenPayload(){return this->_tokenPayload;}
    inline void setTokenChannel(TokenChannel tc){this->_tokenChannel= tc;}
    inline TokenChannel getTokenChannel() const {return this->_tokenChannel;}

    inline void setEndpoint(const std::string& ep){
        this->endpoint= ep;
    }
    inline std::string getEndpoint() const {return this->endpoint;}

    inline unsigned int getRequestID() const {return this->request_id;}

    inline void setResonseHandler(std::shared_ptr<ResponseHandler> rh){
        this->responder= rh;
    }

    std::string queryParametersString() const;

    inline boost::beast::http::verb getVerb() const {return this->verb;}


protected:
    boost::beast::http::verb verb= boost::beast::http::verb::get;

    std::shared_ptr<ResponseHandler> responder= nullptr;


    boost::beast::http::request<boost::beast::http::string_body> getBeastRequest(const Infos& infos) const;
};



class ResponseHandler {
public:
    using TaskFunctionType = std::function< void(const Response& response) >;

    inline ResponseHandler(std::shared_ptr< TaskFunctionType > func) :
        processing_func(func)
    {}

    inline void processResponse(const Response& response) { (*this->processing_func)(response); }
private:
    std::shared_ptr< TaskFunctionType > processing_func;
};

}
}

#endif // QUERY_H
