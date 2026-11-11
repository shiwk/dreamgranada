#include "clients3.hpp"
#include "client_stub.hpp"
#include "logger.hpp"
#include <boost/asio/ssl/host_name_verification.hpp>

using namespace boost;
namespace http = granada::http;

void http::asyncRequest(http::io_contextPtr &io_context, http::RequestPtr &request, http::ResponseHandler &&requestCallback, http::ErrorHandler &&errorHandler)
{
    auto https = request->https;
    auto resolver = std::make_shared<tcp::resolver>(*io_context);

    if (https)
    {
        auto context = http::createContext<http::sSock>(io_context, request, std::move(requestCallback), std::move(errorHandler));
        context->prepare(request);
        if (SSL_set_tlsext_host_name(context->sock->native_handle(), request->host.c_str()) != 1)
        {
            LOG_ERROR("Failed to set SNI hostname.");
        }
        // Verify the peer certificate *and* that it matches the host name.
        // Without this the client accepts any certificate (MITM).
        context->sock->set_verify_mode(ssl::verify_peer);
        context->sock->set_verify_callback(asio::ssl::host_name_verification(request->host));
        resolver->async_resolve(request->host, HTTPS, [context, resolver](const error_code &err, tcp::resolver::results_type endpoints)
                                { http::onResolve(err, endpoints, context); });
    }
    else
    {
        auto context = http::createContext<http::tSock>(io_context, request, std::move(requestCallback), std::move(errorHandler));
        context->prepare(request);
        resolver->async_resolve(request->host, HTTP, [context, resolver](const error_code &err, tcp::resolver::results_type endpoints)
                                { http::onResolve(err, endpoints, context); });
    }
}
