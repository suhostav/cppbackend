#pragma once
#include "http_server.h"
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
    explicit RequestHandler(fs::path root, Strand api_strand, model::Game& game)
        : root_{root}
        , api_strand_{api_strand}
        , game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
        const std::string_view method_str{req.method_string()};
        auto version = req.version();
        auto keep_alive = req.keep_alive();
        const auto text_response = [&req, this](http::status status, std::string_view text, std::string_view content_type) {
            return this->MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
        };

        StringResponse response{};

        if( method_str == HEAD_STR || method_str == GET_STR){
            std::string body;
            if( method_str == GET_STR){
                const std::string maps_list_request{"/api/v1/maps"};
                const std::string map_request{"/api/v1/maps/"};
                const std::string bad_api_request{"/api/"};
                std::string target{req.target()};
                target = DecodeUri(target);
                if(target == "/"s){
                    target = "/index.html"s;
                } 
                
                if(target.starts_with("/api/")){
                    auto handle = [self = shared_from_this(), send,
                                req = std::forward<decltype(req)>(req), version, keep_alive] {
                        try {
                            // Этот assert не выстрелит, так как лямбда-функция будет выполняться внутри strand
                            assert(self->api_strand_.running_in_this_thread());
                            return send(self->HandleApiRequest(req));
                        } catch (...) {
                            send(self->ReportServerError(version, keep_alive));
                        }
                    };
                    return net::dispatch(api_strand_, handle);
                } else if(IsSubPath(root_.string() + target, root_)){
                    try{
                        send(MakeFileResponse(root_.string() + target, req.version(), req.keep_alive()));
                    } catch(...){
                        send(NotFound(req.version(), req.keep_alive()));
                    }
                    return;
                } else {
                    send(
                        MakeStringResponse(http::status::bad_request, 
                        "Invalid file path."s, 
                        req.version(), 
                        req.keep_alive(), 
                        ContentType::TEXT_PLAIN)
                        );
                    return;
                }
            }
        } else {
            response = text_response(http::status::method_not_allowed, "Invalid method"sv, ContentType::TEXT_HTML);
            response.set("Allow"sv, "GET, HEAD"sv);
        }

        send(response);
    }

    static constexpr std::string_view HEAD_STR{"HEAD"};
    static constexpr std::string_view GET_STR{"GET"};
private:
    using FileRequestResult = std::variant<EmptyResponse, StringResponse, FileResponse>;
    FileRequestResult HandleFileRequest(const StringRequest& req) const;
    StringResponse HandleApiRequest(const StringRequest& request) const;
    StringResponse ReportServerError(unsigned version, bool keep_alive) const;

    struct ContentType {
        ContentType() = delete;
        constexpr static std::string_view OCTET_STREAM =   "text/html"sv;
        constexpr static std::string_view TEXT_HTML =   "text/html"sv;
        constexpr static std::string_view TEXT_CSS =   "text/css"sv;
        constexpr static std::string_view TEXT_PLAIN =   "text/plain"sv;
        constexpr static std::string_view TEXT_JAVASCRIPT =   "text/javascript"sv;
        constexpr static std::string_view APPLICATION_JSON =   "application/json"sv;
        constexpr static std::string_view APPLICATION_XML =   "application/xml"sv;
        constexpr static std::string_view IMAGE_PNG =   "image/png"sv;
        constexpr static std::string_view IMAGE_JPEG =   "image/jpeg"sv;
        constexpr static std::string_view IMAGE_GIF =   "image/gif"sv;
        constexpr static std::string_view IMAGE_BMP =   "image/bmp"sv;
        constexpr static std::string_view IMAGE_ICO =   "image/vnd.microsoft.icon"sv;
        constexpr static std::string_view IMAGE_TIFF =   "image/tiff"sv;
        constexpr static std::string_view IMAGE_SVG =   "image/svg+xml"sv;
        constexpr static std::string_view AUDIO_MP3 =   "audio/mpeg"sv;
    };

    StringResponse MakeStringResponse(http::status status,
                                        std::string_view body,
                                        unsigned http_version,
                                        bool keep_alive,
                                        std::string_view content_type = ContentType::TEXT_HTML
                                        ) const;
    FileResponse MakeFileResponse(const std::string file, unsigned http_version, bool keep_alive) const;
    std::string MapsListResponse() const;
    std::pair<bool, std::string> MapResponse(const std::string& map_id) const;

    boost::json::array GetRoadsArray(const model::Map& map) const;
    boost::json::array GetBuildingssArray(const model::Map& map) const;
    boost::json::array GetOfficesArray(const model::Map& map) const;

    std::string DecodeUri(const std::string& uri_str) const;
    char DecodeChar(std::istream& in) const;

    std::string BadRequest() const;
    StringResponse NotFound(unsigned http_version, bool keep_alive);

    fs::path root_;
    Strand api_strand_;
    model::Game& game_;
    static std::map<std::string, std::string_view> extensions_ ;
};

// template<class SomeRequestHandler>
// class LoggingRequestHandler {
//      static void LogRequest(const Request& r);
//      static void LogResponse(const Response& r);
// public:
//     LoggingRequestHandler(SomeRequestHandler& decorated): decorated_(decorated){}
//      Response operator () (Request req) {
//          LogRequest(req);
//          Response resp = decorated_(std::move(req));
//          LogResponse(resp);
//          return resp;
//      }

// private:
//      SomeRequestHandler& decorated_;
// };

}  // namespace http_handler

