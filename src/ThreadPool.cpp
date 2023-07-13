#include "ThreadPool.h"
#include "ApiClient.h"
#include "Query.h"

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

                this->apiClient->processRequest(request);
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
