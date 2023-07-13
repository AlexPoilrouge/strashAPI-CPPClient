#ifndef TESTAPICOMPOSE_H
#define TESTAPICOMPOSE_H

#include <gtest/gtest.h>

#include "QueryFactory.h"
#include "Query.h"
#include "ApiClient.h"

#ifdef TESTWITHCOMPOSE

class _TestApiCompose : public testing::Test {
protected:
    void SetUp() override {
        this->client= new StrashBot::API::Client();

        this->client->prepare(
            std::filesystem::current_path().generic_string()+"/data/test-api.test.json"
        );
    }

    void TearDown() override {
        delete this->client;
    }


    StrashBot::API::Client* client;
};

#endif //TESTWITHCOMPOSE

#endif //TESTAPICOMPOSE_H
