#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/json.hpp>
#include <exception>
#include <map>
#include <string>
#include <filesystem>

#include "model.h"

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
using StringRequest = http::request<http::string_body>;


class BadRequestException : public std::invalid_argument {
public:
    BadRequestException(const std::string& code, const std::string& msg)
        : code_(code)
        , std::invalid_argument(msg){

        }
    const std::string& GetCode() const { return code_;}
private:
    std::string code_;
};

class BadMethodException : public std::invalid_argument {
public:
    BadMethodException(const std::string& code, const std::string& msg)
        : code_(code)
        , std::invalid_argument(msg){

        }
    const std::string& GetCode() const { return code_;}
private:
    std::string code_;
};

class InvalidTokenExeption : public std::invalid_argument {
public:
    InvalidTokenExeption(const std::string msg):
        std::invalid_argument(msg){
    }
};

class UnknownTokenExeption : public std::invalid_argument {
public:
    UnknownTokenExeption(const std::string msg):
        std::invalid_argument(msg){
    }
};

std::string DecodeUri(const std::string& uri_str);
char DecodeChar(std::istream& in);

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

struct handlers_statics {
    static constexpr std::string_view HEAD_STR{"HEAD"};
    static constexpr std::string_view GET_STR{"GET"};
    static constexpr std::string_view POST_STR{"POST"};
    static constexpr std::string_view API_STR{"/api/"};
    static constexpr std::string_view BEARER_STR{"Bearer "};
    static constexpr size_t token_length = 32;
    static std::map<std::string, std::string_view> extensions_ ;
    static StringResponse MakeStringResponse(http::status status,
                                        std::string_view body,
                                        unsigned http_version,
                                        bool keep_alive,
                                        std::string_view content_type = ContentType::TEXT_HTML,
                                        bool noCache = true
                                        );
    static std::string BadRequest(const std::string& code, const std::string& message);
    static StringResponse BadMethodResponse(unsigned http_version, bool keep_alive, const std::string& method, const std::string& message);
    static StringResponse InvalidTokenResponse(unsigned http_version, bool keep_alive);
    static StringResponse UnknownTokenResponse(unsigned http_version, bool keep_alive);
};

class api_requests {
public:
    static constexpr std::string_view maps_list_request{"/api/v1/maps"};
    static constexpr std::string_view map_request{"/api/v1/maps/"};
    static constexpr std::string_view join_game_request{"/api/v1/game/join"};
    static constexpr std::string_view players_list_requesst{"/api/v1/game/players"};
    static constexpr std::string_view state_requesst{"/api/v1/game/state"};
    static constexpr std::string_view action_request{"/api/v1/game/player/action"};
    static constexpr std::string_view tick_request{"/api/v1/game/player/tick"};
};

}   //namespace http_handler