#include "ThreadPool.h"
#include "ApiClient.h"
#include "Query.h"
#include "errors.h"

#include <functional>

namespace api= StrashBot::API;


api::ThreadPool::~ThreadPool(){
    // stop thread pool, notify threads to finish remaining tasks
    {
        std::unique_lock<std::mutex> lock(this->mtx);
        this->stop= true;
    }
    this->cv.notify_all();
    for(auto & worker : this->workers){
        worker.join();
    }
}

api::ThreadPool::ThreadPool(api::Client* client, size_t nbThreads) : stop(false), apiClient(client){
    for(size_t i=0; i<nbThreads; ++i){
        std::thread worker([this](){
            while(true){
                std::shared_ptr<api::Request> request;
                // pop task from queue and exec
                {
                    std::unique_lock lock(mtx);
                    cv.wait(lock, [this](){return stop || !this->requests.empty();});

                    if(stop && this->requests.empty()) return;
                    // even if stop is true, exec unti task queue empty

                    request= requests.front();
                    this->requests.pop();
                }
                try{
                    this->apiClient->processRequest(request);
                }
                catch(api::ConnectTimeoutException e){
                    auto response= api::Response::create(
                        "{\"client_error\": \"connection_timeout\"}",
                        0, request);

                    if(request->responder){
                        request->responder->processResponse(response);
                    }
                }
                catch(boost::beast::system_error e){
                    auto response= api::Response::create(
                        std::string("{\"client_error\": \"api_comm_error\", \"info\": \"")+e.what()+"\"}",
                        0, request
                    );

                    if(request->responder){
                        request->responder->processResponse(response);
                    }
                }
                catch(std::exception err){
                    auto response= api::Response::create(
                        std::string("{\"client_error\": \"error\", \"info\": \"")+err.what()+"\"}",
                        0, request
                    );

                    if(request->responder){
                        request->responder->processResponse(response);
                    }
                }
            }
        });
        this->workers.emplace_back(std::move(worker));
    }
}

void api::ThreadPool::enqueue(std::shared_ptr<api::Request> req){
    {
        std::unique_lock lock(this->mtx);

        if(this->stop)
            throw std::runtime_error("The thread pool has been stop.");

        //wrapper for no returned value
        requests.emplace(req);
    }
    this->cv.notify_one();
}
