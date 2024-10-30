#pragma once

#include "http_server.h"
#include "handlers_tools.h"
#include "ApiHandler.h"
#include "model.h"
#include <boost/json.hpp>
#include <boost/asio.hpp>
#include <filesystem>
#include <optional>
#include <variant>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;
namespace net = boost::asio;
namespace sys = boost::system;
using namespace std::literals;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;
using EmptyResponse = http::response<http::empty_body>;
using http_server::StringRequest;
bool IsSubPath(fs::path path, fs::path base);

class RequestHandler : public std::enable_shared_from_this<RequestHandler> {
public:
    using Strand = net::strand<net::io_context::executor_type>;
    explicit RequestHandler(fs::path root, Strand api_strand, std::shared_ptr<ApiHandler> api_handler)
        : root_{root}
        , api_strand_(api_strand)
        , api_handler_(api_handler) {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        auto version = req.version();
        auto keep_alive = req.keep_alive();

        try {
            if (IsApiRequest(req)) {
                auto handle = [self = shared_from_this(), send,
                               req = std::forward<decltype(req)>(req), version, keep_alive] {
                    try {
                        // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                        assert(self->api_strand_.running_in_this_thread());
                        return send(self->api_handler_->HandleApiRequest(req));
                    } catch (...) {
                        send(self->ReportServerError(version, keep_alive));
                    }
                };
                return net::dispatch(api_strand_, handle);
            }
            // Возвращаем результат обработки запроса к файлу
            return std::visit(
                [&send](auto&& result) {
                    send(std::forward<decltype(result)>(result));
                },
                HandleFileRequest(req));
        } catch (...) {
            send(ReportServerError(version, keep_alive));
        }
    }

private:
    using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;
    
    FileRequestResult HandleFileRequest(const StringRequest& req) const;
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;
    bool IsApiRequest(const StringRequest& request) const;

    FileResponse MakeFileResponse(const std::string file, unsigned http_version, bool keep_alive) const;

    std::string BadRequest() const;
    StringResponse NotFound(unsigned http_version, bool keep_alive) const;

    fs::path root_;
    Strand api_strand_;
    std::shared_ptr<ApiHandler> api_handler_;
};

}  // namespace http_handler

