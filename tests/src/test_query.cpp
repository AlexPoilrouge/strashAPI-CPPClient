#include <gtest/gtest.h>

#include <Query.h>
#include <QueryFactory.h>


TEST(Query_test, basicCreateQueryTest){
    auto req= StrashBot::API::RequestFactory::create(StrashBot::API::RequestVerb::get);

    req->queryParameters()["parameter1"]= "okay";
    req->queryParameters()["p2"]="i";

    EXPECT_TRUE(
        (req->queryParametersString() == "?parameter1=okay&p2=i")
        || (req->queryParametersString() == "?p2=i&parameter1=okay")
    );

    //next test body
    json source_json= json::parse(R"(
        {
            "field1": 5,
            "field2": "yes"
        }
    )");

    req->body()= source_json;
//    EXPECT_EQ(source_json.dump(), *req->body());
    EXPECT_EQ(req->body()["field1"], 5);
    EXPECT_EQ(req->body()["field2"], "yes");
}

