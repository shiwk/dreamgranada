#include <gtest/gtest.h>
#include <clients3.hpp>
#include <iostream>
#include <future>
#include <reqresp.hpp>

namespace ghttp = granada::http;

struct GResponse
{
    GResponse(const boost::system::error_code &ec, const std::string &statusCode, const std::string &content)
        : ec(ec), statusCode(statusCode), content(content) {}
    boost::system::error_code ec;
    std::string statusCode;
    std::string content;
};


TEST(ClientsTests, Test_Get_PLAIN_HTTP)
{
    auto request = std::make_shared<ghttp::Request>(ghttp::Method::GET, "www.ifconfig.me", "/", "curl", "close", "", false);
    EXPECT_EQ(ghttp::Method::GET, request->method);
    EXPECT_EQ("/", request->path);
    EXPECT_EQ("curl", request->user_agent);
    EXPECT_EQ("close", request->connection);
    EXPECT_EQ("", request->body);
    EXPECT_EQ("GET", ghttp::methodToString(request->method));
    EXPECT_EQ("curl", request->headers["User-Agent"]);
    EXPECT_EQ("close", request->headers["Connection"]);

    std::promise<GResponse> prom;
    auto fut = prom.get_future();
    auto context = std::make_shared<asio::io_context>();
    auto guard = asio::make_work_guard(context);
    ghttp::asyncRequest(context, request, [&prom, &guard](const boost::system::error_code &ec, ghttp::ResponsePtr &response)
                                    {
                                        prom.set_value({ec, response->statusCode, response->content});
                                        guard.reset(); 
                                    },
                                        [&prom, &guard](const boost::system::error_code &ec)
                                     {
                                         prom.set_value({ec, "", ""});
                                        guard.reset(); });

    std::future<void> f = std::async(std::launch::async, [&]
                                     { context->run(); });
    auto futStatus = fut.wait_for(std::chrono::seconds(10));
    assert(futStatus == std::future_status::ready);
    auto gresponse = fut.get();
    EXPECT_NE("", gresponse.content);
    LOG_DEBUG_FMT("Response content: {}", gresponse.content);
    EXPECT_EQ("200", gresponse.statusCode);
}