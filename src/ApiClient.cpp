
#include "QueryFactory.h"
#include "../includes/ApiClient.h"
#include "../includes/errors.h"

#include <boost/asio/use_future.hpp>

#include <sstream>
namespace api= StrashBot::API;

void api::Client::prepare(const std::string& jsonFilepath){
    this->apiInfos= std::move(api::readFromJsonFile(jsonFilepath));
}

void api::Client::submit(std::shared_ptr<Request> req, const std::string& endpoint){
    req->setEndpoint(endpoint);
    this->submit(req);
}



api::QueryPair api::Client::requestEndPoint(const std::string& endpoint, std::shared_ptr< std::function< void(const api::Response& response) > > f){
    EndPoint ep= this->apiInfos.endpoints[endpoint];
    auto qp= api::QueryPairFactory::create(ep.verb, f);

    this->submit(qp.first, endpoint);

    return qp;
}

boost::beast::tcp_stream api::Client::prepareRequest(boost::asio::io_context& ioc){
    const std::string domain= this->apiInfos.base_url;
    const unsigned int port= this->apiInfos.port;

    boost::asio::ip::tcp::resolver resolver(ioc);   // These objects perform our I/O
    boost::beast::tcp_stream stream(ioc);           // These objects perform our I/O

    const auto results= resolver.resolve(domain, std::to_string(port)); // Look up the domain name

    stream.expires_after(std::chrono::seconds(QueryTimeout));
    stream.connect(results);    // Make the connection on the IP address we get from a lookup

    return std::move(stream);
}

boost::beast::ssl_stream<boost::beast::tcp_stream> api::Client::prepareSSLRequest(boost::asio::io_context& ioc){
    const std::string domain= this->apiInfos.base_url;
    const unsigned int port= this->apiInfos.port;

    // The SSL context is required, and holds certificates
    boost::asio::ssl::context ctx(boost::asio::ssl::context::sslv23_client);

    // This holds the root certificate used for verification
//    load_root_certificates(ctx);

    // Verify the remote server's certificate
//    ctx.set_verify_mode(boost::asio::ssl::verify_peer);

    // These objects perform our I/O
    boost::asio::ip::tcp::resolver resolver(ioc);
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream(ioc, ctx);


    // Look up the domain name
    auto const results = resolver.resolve(domain, std::to_string(port));
    // Make the connection on the IP address we get from a lookup
    auto& lowest_layer= boost::beast::get_lowest_layer(stream);
    lowest_layer.expires_after(std::chrono::seconds(QueryTimeout));
    auto Future = lowest_layer.async_connect(results, boost::asio::use_future);
    if(Future.wait_for(std::chrono::seconds(QueryTimeout)) == std::future_status::timeout){
        throw api::ConnectTimeoutException(domain, port);
    }
    else{
        // Perform the SSL handshake
        stream.handshake(boost::asio::ssl::stream_base::client);
    }

    return std::move(stream);
}


void api::Client::processRequest(std::shared_ptr<api::Request> req){
    boost::asio::io_context ioc;                    // The io_context is required for all I/O

    boost::beast::error_code ec;
    if(this->apiInfos.ssl){
        boost::beast::ssl_stream<boost::beast::tcp_stream>&& ssl_stream= this->prepareSSLRequest(ioc);

        ec= requestHandle(
            req,
            ssl_stream
        );

        ssl_stream.shutdown(ec);
    }
    else{
        boost::beast::tcp_stream&& stream= this->prepareRequest(ioc);

        ec= requestHandle(
            req,
            stream
        );

        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);// Gracefully close the socket
    }

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec &&
        ec != boost::beast::errc::not_connected &&
        ec != boost::asio::ssl::error::stream_truncated
    ){
        throw boost::beast::system_error{ec};
    }
}

