#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>
#include <filesystem>
#include <optional>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
namespace fs = std::filesystem;
using namespace std::literals;

using StringResponse = http::response<http::string_body>;
using FileResponse = http::response<http::file_body>;

bool IsSubPath(fs::path path, fs::path base);

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send, const std::string& base_dir) {
        // Обработать запрос request и отправить ответ, используя send
        const std::string_view method_str{req.method_string()};
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
                if(target == maps_list_request){
                    body = MapsListResponse();
                    response =  text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
                } else if(target.starts_with(map_request) && target.size() > map_request.size()){
                    std::string map_id{target.substr(map_request.size())};
                    auto result = MapResponse(map_id);
                    body = result.second;
                    if(result.first == true){
                        response =  text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
                    } else {
                        response = text_response(http::status::not_found, body, ContentType::APPLICATION_JSON);
                    }
                } else if(target.starts_with(bad_api_request)){
                    body = BadRequest();
                    response = text_response(http::status::bad_request, body, ContentType::APPLICATION_JSON);
                } else if(IsSubPath(base_dir + target, base_dir)){
                    try{
                        send(MakeFileResponse(base_dir + target, req.version(), req.keep_alive()));
                        return;
                    } catch(...){
                        send(NotFound(req.version(), req.keep_alive()));
                        return;
                    }
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
    std::string MapsListResponse();
    std::pair<bool, std::string> MapResponse(const std::string& map_id) const;

    boost::json::array GetRoadsArray(const model::Map& map) const;
    boost::json::array GetBuildingssArray(const model::Map& map) const;
    boost::json::array GetOfficesArray(const model::Map& map) const;

    std::string DecodeUri(const std::string& uri_str);
    char DecodeChar(std::istream& in);

    std::string BadRequest();
    StringResponse NotFound(unsigned http_version, bool keep_alive);

    model::Game& game_;
    static std::map<std::string, std::string_view> extensions_ ;
};



}  // namespace http_handler

