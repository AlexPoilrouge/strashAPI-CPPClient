#include <gtest/gtest.h>

#include "ConfigReader.h"

#include <filesystem>


TEST(ConfigReader_test, bogusConfigLoad){
    StrashBot::API::Infos apiInfo= StrashBot::API::readFromJsonFile(
        std::filesystem::current_path().generic_string()+"/data/bogus_config.test.json"
    );

    EXPECT_EQ(apiInfo.base_url, "bogus.fr");
    EXPECT_EQ(apiInfo.root, "coolapi");
    EXPECT_EQ(apiInfo.port, 1534);
    EXPECT_TRUE(apiInfo.ssl);

    EXPECT_EQ(apiInfo.endpoints["user-get"].path, "user");
    EXPECT_EQ(*apiInfo.endpoints["user-get"].token, "t0k€nlol");
    EXPECT_EQ(apiInfo.endpoints["user-get"].verb, StrashBot::API::RequestVerb::get);

    EXPECT_EQ(apiInfo.endpoints["whatsup"].path, "what/is/up");
    EXPECT_EQ(*apiInfo.endpoints["whatsup"].token, "w@zzzzuuuuuup");
    EXPECT_EQ(apiInfo.endpoints["whatsup"].verb, StrashBot::API::RequestVerb::post);

    EXPECT_EQ(apiInfo.getServerBaseUrl(), "bogus.fr:1534");
    EXPECT_EQ(apiInfo.getRootUrl(), "bogus.fr:1534/coolapi");
    auto userget= apiInfo.getEndpointUrl("user-get");
    EXPECT_EQ(userget.first, "bogus.fr:1534/coolapi/user");
    EXPECT_EQ(userget.second, "t0k€nlol");
    auto whatsup= apiInfo.getEndpointUrl("whatsup");
    EXPECT_EQ(whatsup.first, "bogus.fr:1534/coolapi/what/is/up");
    EXPECT_EQ(whatsup.second, "w@zzzzuuuuuup");
}

TEST(ConfigReader_test, empty){
    StrashBot::API::Infos infos;

    EXPECT_EQ(infos.getRootUrl(), "strashbot.fr");
    EXPECT_EQ(infos.getRootUrl(), infos.getServerBaseUrl());
    EXPECT_FALSE(infos.ssl);
}

TEST(InvokeConfigReader_quickTest, invokeBogusConfigLoad){
    std::future<StrashBot::API::Infos> future= StrashBot::API::async_readFromJsonFile(
        std::filesystem::current_path().generic_string()+"/data/bogus_config.test.json"
    );

    StrashBot::API::Infos apiInfo= future.get();
    EXPECT_EQ(apiInfo.base_url, "bogus.fr");
    EXPECT_EQ(apiInfo.root, "coolapi");
    EXPECT_EQ(apiInfo.port, 1534);
    EXPECT_TRUE(apiInfo.ssl);
    EXPECT_EQ(apiInfo.getServerBaseUrl(), "bogus.fr:1534");
}
