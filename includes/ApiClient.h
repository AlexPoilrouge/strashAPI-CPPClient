#ifndef APICLIENT_H
#define APICLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "ConfigReader.h"
#include "ThreadPool.h"
#include "Query.h"

#include <mutex>
#include <list>

#include <cstdint>


#define STRASHAPI_CLIENT_THREAD_NUMBER 2
#define CLIENT_STRASHAPI_INT_VERSION 0
#define CLIENT_STRASHAPI_USERAGENT_STRING "strashapi_cpp-client"

namespace StrashBot {
namespace API {

using QueryPair = std::pair<std::shared_ptr<Request>, std::shared_ptr<ResponseHandler> >;

constexpr unsigned int QueryTimeout= 10;

class Client{
    friend ThreadPool;
public:
    Client() : apiInfos(), thread_pool(this, STRASHAPI_CLIENT_THREAD_NUMBER) {}
    ~Client() = default;

    void prepare(const std::string& jsonFilepath);

    inline void submit(std::shared_ptr<Request> req){
        this->thread_pool.enqueue(req);
    }
    inline void submit(std::shared_ptr<Request> req, const std::string& endpoint);

    QueryPair requestEndPoint(const std::string& endpoint, std::shared_ptr< std::function< void(const Response& response) > > f);

protected:
    Infos apiInfos;
    std::list< std::shared_ptr<Request> > pending_requests;

    void processRequest(std::shared_ptr<Request> req);
private:

    ThreadPool thread_pool;

    boost::beast::tcp_stream prepareRequest(boost::asio::io_context& ioc);
    boost::beast::ssl_stream<boost::beast::tcp_stream> prepareSSLRequest(boost::asio::io_context& ioc);

    template <class STREAM>
    boost::beast::error_code requestHandle(std::shared_ptr<Request> req, STREAM&& stream){
        boost::beast::error_code ec;

        auto beastRequest= req->getBeastRequest(this->apiInfos);

        boost::beast::http::write(stream, beastRequest, ec);    // Send the HTTP request to the remote host

        boost::beast::http::response<boost::beast::http::string_body> res; // Declare a container to hold the response

        boost::beast::flat_buffer buffer;   // This buffer is used for reading and must be persisted

        boost::beast::http::read(stream, buffer, res);  // Receive the HTTP response

        std::string str_response;
        {
            std::ostringstream oss;
            oss << res.body();
            str_response= std::move(oss.str());
        }

        auto response= Response::create(str_response, res.result_int(), req);

        if(req->responder){
            req->responder->processResponse(response);
        }
        else{
            std::cout << str_response;
        }

        return ec;
    }
};

}
}

#endif // APICLIENT_H
