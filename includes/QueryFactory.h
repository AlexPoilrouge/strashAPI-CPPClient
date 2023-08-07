#ifndef QUERYFACTORY_H
#define QUERYFACTORY_H

#include <boost/beast/http.hpp>

#include <memory>
#include <utility>

#include "utils.h"

namespace StrashBot {
namespace API {

class Request;
class Response;
class ResponseHandler;
class RequestFactory{
public:
    static std::shared_ptr<Request> create(boost::beast::http::verb verb= boost::beast::http::verb::get);

    static std::shared_ptr<Request> retry(const Request& req);
};


using QueryPair = std::pair<std::shared_ptr<Request>, std::shared_ptr<ResponseHandler> >;

class QueryPairFactory{
public:

    static QueryPair create(
        RequestVerb verb,
        std::shared_ptr< std::function< void(const Response& response) > > f
    );

    static inline QueryPair create(
         std::shared_ptr< std::function< void(const Response& response) > > f
    ){ return QueryPairFactory::create( RequestVerb::unknown, f); }

    inline static QueryPair create( std::shared_ptr<Request> req, std::shared_ptr<ResponseHandler> rh){
        return std::make_pair(req, rh);
    }
};

}
}

#endif // QUERYFACTORY_H
