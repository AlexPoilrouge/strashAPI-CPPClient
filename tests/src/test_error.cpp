#include "test_error.h"

#include "errors.h"

#include <filesystem>


TEST(Bad_ConfigTest, BadFileConfigLoad){
    EXPECT_THROW({
        StrashBot::API::Infos apiInfo1= StrashBot::API::readFromJsonFile(
            std::filesystem::current_path().generic_string()+"/data/not_exist.json"
        );
    }, StrashBot::API::NonExistantConfigFileException);

    EXPECT_THROW({
        StrashBot::API::Infos apiInfo2= StrashBot::API::readFromJsonFile(
            std::filesystem::current_path().generic_string()+"/data/actually_not.test.json"
        );
    }, StrashBot::API::ConfigJSONParsingError);
}

TEST_F(_TestApiError, BadInfoInConfig){
    std::mutex m;
    std::condition_variable cv;
    unsigned int queryStep= 0;

    json resp;
    bool clientError= false;
    unsigned int status_code= 0;

    auto query_pair= StrashBot::API::QueryPairFactory::create(
        StrashBot::API::RequestVerb::get,
        std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(
            [&](const StrashBot::API::Response& response){
                std::unique_lock lock(m);
                cv.wait(lock, [&](){return queryStep>0;});

                status_code= response.status_code;
                clientError= response.isClientError();
                resp= response.responseBody;

                queryStep= 2;

                cv.notify_one();
            }
            )
        );

    query_pair.first->setEndpoint("user-get");

    {
        this->client->submit(query_pair.first);
        queryStep= 1;

        cv.notify_one();

        std::unique_lock lock(m);
        cv.wait(lock, [&](){return queryStep>1;});
    }

    EXPECT_EQ(status_code, 0);
    EXPECT_TRUE(clientError);
    EXPECT_EQ(resp["client_error"], "connection_timeout");
}

TEST_F(_TestApiError2, BadInfoInConfig2){
    std::mutex m;
    std::condition_variable cv;
    unsigned int queryStep= 0;

    json resp;
    bool clientError= false;
    unsigned int status_code= 0;

    auto query_pair= StrashBot::API::QueryPairFactory::create(
        StrashBot::API::RequestVerb::get,
        std::make_shared<StrashBot::API::ResponseHandler::TaskFunctionType>(
            [&](const StrashBot::API::Response& response){
                std::unique_lock lock(m);
                cv.wait(lock, [&](){return queryStep>0;});

                status_code= response.status_code;
                clientError= response.isClientError();
                resp= response.responseBody;

                queryStep= 2;

                cv.notify_one();
            }
            )
        );

    query_pair.first->setEndpoint("whatsup");

    {
        this->client->submit(query_pair.first);
        queryStep= 1;

        cv.notify_one();

        std::unique_lock lock(m);
        cv.wait(lock, [&](){return queryStep>1;});
    }

    EXPECT_EQ(status_code, 0);
    EXPECT_TRUE(clientError);
}

TEST(Bad_ConfigTest, BadTokenKeyFile){
    EXPECT_THROW({
        StrashBot::API::Infos apiInfo1= StrashBot::API::readFromJsonFile(
            std::filesystem::current_path().generic_string()+"/data/no_key_bogus.test.json"
        );
    }, StrashBot::API::BadTokenKeyFilePathForJWT);
}
