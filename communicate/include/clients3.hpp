#ifndef CLIENTS3_HPP
#define CLIENTS3_HPP

#include "httpctx.hpp"
#include "logger.hpp"

using namespace boost;
namespace ssl = boost::asio::ssl;

namespace granada
{
    namespace http
    {
        extern void asyncRequest(io_contextPtr &, RequestPtr &, ResponseHandler &&, ErrorHandler &&);
    }
}

#endif