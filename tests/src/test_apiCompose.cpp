
#include "test_apiCompose.h"


#ifdef TESTWITHCOMPOSE

TEST_F(_TestApiCompose, simpleGet1){
    std::mutex m;
    std::condition_variable cv;
    unsigned int queryStep= 0;

    std::string result_value="";
    unsigned int status_code= 0;

    auto query_pair= StrashBot::API::QueryPairFactory::create(
        StrashBot::API::RequestVerb::get,
        std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(
            [&](const StrashBot::API::Response& response){
                std::unique_lock lock(m);
                cv.wait(lock, [&](){return queryStep>0;});

                status_code= response.status_code;
                result_value= response.responseBody["result"];

                queryStep= 2;

                cv.notify_one();
            }
        )
    );
    query_pair.first->setEndpoint("simpleGet");

    {
        this->client->submit(query_pair.first);
        queryStep= 1;

        cv.notify_one();

        std::unique_lock lock(m);
        cv.wait(lock, [&](){return queryStep>1;});
    }

    EXPECT_EQ(status_code, 200);
    EXPECT_EQ(result_value, "okidoki");
}

TEST_F(_TestApiCompose, paramGet1){
    std::mutex m;
    std::condition_variable cv;
    unsigned int queryStep= 0;

    json resp;
    unsigned int status_code= 0;

    auto query_pair= StrashBot::API::QueryPairFactory::create(
        StrashBot::API::RequestVerb::get,
        std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(
            [&](const StrashBot::API::Response& response){
                std::unique_lock lock(m);
                cv.wait(lock, [&](){return queryStep>0;});

                status_code= response.status_code;
                resp= response.responseBody;

                queryStep= 2;

                cv.notify_one();
            }
        )
    );
    query_pair.first->setEndpoint("paramGet");

    query_pair.first->queryParameters()["param1"]= "something";
    query_pair.first->queryParameters()["param2"]="okay";

    {
        this->client->submit(query_pair.first);
        queryStep= 1;

        cv.notify_one();

        std::unique_lock lock(m);
        cv.wait(lock, [&](){return queryStep>1;});
    }

    EXPECT_EQ(status_code, 200);
    EXPECT_EQ(resp["response1"], "got something");
    EXPECT_EQ(resp["resp2"], "is okay");
}

TEST_F(_TestApiCompose, requestBody1){
    std::mutex m;
    std::condition_variable cv;
    unsigned int queryStep= 0;

    json resp;
    unsigned int status_code= 0;

    auto query_pair= StrashBot::API::QueryPairFactory::create(
        StrashBot::API::RequestVerb::put,
        std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(
            [&](const StrashBot::API::Response& response){
                std::unique_lock lock(m);
                cv.wait(lock, [&](){return queryStep>0;});

                status_code= response.status_code;
                resp= response.responseBody;

                queryStep= 2;

                cv.notify_one();
            }
        )
    );
    query_pair.first->setEndpoint("bodyPut");

    query_pair.first->body() << json::parse(R"(
        {
            "param": "yolo"
        }
    )");

    {
        this->client->submit(query_pair.first);
        queryStep= 1;

        cv.notify_one();

        std::unique_lock lock(m);
        cv.wait(lock, [&](){return queryStep>1;});
    }


    EXPECT_EQ(status_code, 200);
    EXPECT_EQ(resp["anwser"], "putted yolo");
}

TEST_F(_TestApiCompose, postHeader1){
    std::mutex m;
    std::condition_variable cv;
    unsigned int queryStep= 0;

    json resp;
    unsigned int status_code= 0;

    auto query_pair= StrashBot::API::QueryPairFactory::create(
        StrashBot::API::RequestVerb::post,
        std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(
            [&](const StrashBot::API::Response& response){
                std::unique_lock lock(m);
                cv.wait(lock, [&](){return queryStep>0;});

                status_code= response.status_code;
                resp= response.responseBody;

                queryStep= 2;

                cv.notify_one();
            }
        )
    );
    query_pair.first->setEndpoint("headerPost");

    query_pair.first->headers()["x-test-header"]= "pico";
    query_pair.first->headers()["x-another-header"]= "bello";

    {
        this->client->submit(query_pair.first);
        queryStep= 1;

        cv.notify_one();

        std::unique_lock lock(m);
        cv.wait(lock, [&](){return queryStep>1;});
    }

    EXPECT_EQ(status_code, 200);
    EXPECT_EQ(resp["result"], "confirmed pico-bello");
}

#endif //TESTWITHCOMPOSE
