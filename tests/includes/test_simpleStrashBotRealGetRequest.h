#ifndef TEST_SIMPLESTRASHBOTREALGETREQUEST_H
#define TEST_SIMPLESTRASHBOTREALGETREQUEST_H

#include <gtest/gtest.h>

#include "QueryFactory.h"
#include "Query.h"
#include "ApiClient.h"

#include <memory>
#include <functional>

#ifdef REALSTRASHTEST

class _SimpleRelGetStrashBotKartInfo : public testing::Test {
protected:
    void SetUp() override {
        this->cli= new StrashBot::API::Client();

        this->cli->prepare(
            std::filesystem::current_path().generic_string()+"/data/real_strashbot.test.json"
        );

        StrashBot::API::ResponseHandler::TaskFunctionType f=
            std::bind(&_SimpleRelGetStrashBotKartInfo::ProcessResponse, this, std::placeholders::_1);

        qp= std::move(
            StrashBot::API::QueryPairFactory::create(
                StrashBot::API::RequestVerb::get,
                std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(f)
            )
        );

        qp.first->setEndpoint("info");
    }

    void TearDown() override {
        delete this->cli;
    }

    struct TestInfoRespData{
        std::string gotAddress;
        unsigned int code;
    };

    TestInfoRespData tirp;
    bool query_started= false;
    unsigned int nb_procExecution= 0;
    std::mutex m;
    std::condition_variable cv;

    StrashBot::API::Client* cli;
    StrashBot::API::QueryPair qp;

    void ProcessResponse(const StrashBot::API::Response& reponse){
        std::unique_lock lock(m);
        cv.wait(lock, [&](){return this->query_started;});

        ++this->nb_procExecution;
        this->tirp.code= reponse.status_code;
        if(this->tirp.code==200){
            this->tirp.gotAddress= reponse.responseBody["address"];
        }

        cv.notify_one();
    }
};

#endif //REALSTRASHTEST

#endif //TEST_SIMPLESTRASHBOTREALGETREQUEST_H
