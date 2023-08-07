#ifndef TEST_ERROR_H
#define TEST_ERROR_H

#include <gtest/gtest.h>

#include "QueryFactory.h"
#include "Query.h"
#include "ApiClient.h"


class _TestApiError : public testing::Test {
protected:
    void SetUp() override {
        this->client= new StrashBot::API::Client();

        this->client->prepare(
            std::filesystem::current_path().generic_string()+"/data/bogus_config.test.json"
        );
    }

    void TearDown() override {
        delete this->client;
    }


    StrashBot::API::Client* client;
};
class _TestApiError2 : public testing::Test {
protected:
    void SetUp() override {
        try {
            this->client= new StrashBot::API::Client();

            this->client->prepare(
                std::filesystem::current_path().generic_string()+"/data/other_bogus.test.json"
                );
        }
        catch(std::exception e){
            std::cout << "===> " << e.what() << std::endl;

            throw std::exception();
        }
    }

    void TearDown() override {
        delete this->client;
    }


    StrashBot::API::Client* client;
};


#endif // TEST_ERROR_H
