#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <future>
#include <thread>
#include <vector>
#include <queue>

namespace StrashBot {
namespace API {

class Request;

class Client;
// https://www.cnblogs.com/sinkinben/p/16064857.html

class ThreadPool{
public:
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool &&) = delete;

    ThreadPool(Client* client, size_t nbThreads);
    virtual ~ThreadPool();

    void enqueue(std::shared_ptr<Request> req);

private:
    std::vector<std::thread> workers;
    std::queue<std::shared_ptr<Request>> requests;

    //protect 'tasks' queue and stop flag
    std::mutex mtx;
    std::condition_variable cv;
    bool stop;

    Client* apiClient;
};

}
}

#endif // THREADPOOL_H
