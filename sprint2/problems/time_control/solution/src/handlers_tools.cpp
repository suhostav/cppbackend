#include "handlers_tools.h"

namespace http_handler {

    std::string DecodeUri(const std::string& uri_str){
        std::string result;
        std::istringstream ss{uri_str};
        // ss << uri_str;
        while(ss){
            char c{0};
            ss >> c;
            if(c == '%'){
                result += DecodeChar(ss);
            } else if(c == '+'){
                result += ' ';
            } else if(c != '\0'){
                result += c;
            } else {
                break;
            }
        }

        return result;
    }

    char DecodeChar(std::istream& in){
        auto hex_toint = [](char c){
            if(isdigit(c)) return c - '0';
            if(c >= 'a' && c <= 'f') return c - 'a' + 10;
            if(c >= 'A' && c <= 'F') return c - 'A' + 10;
            throw std::invalid_argument("Invalid hex digit");
        };

        char c1, c2;
        in >> c1;
        in >> c2;
        return 16 * hex_toint(c1) + hex_toint(c2);
    }


    std::map<std::string, std::string_view> handlers_statics::extensions_ = {
        {".htm"s, ContentType::TEXT_HTML},
        {".html"s, ContentType::TEXT_HTML},
        {".css"s, ContentType::TEXT_HTML},
        {".txt"s, ContentType::TEXT_PLAIN},
        {".js"s, ContentType::TEXT_JAVASCRIPT},
        {".json"s, ContentType::APPLICATION_JSON},
        {".xml"s, ContentType::APPLICATION_XML},
        {".png"s, ContentType::IMAGE_PNG},
        {".jpg"s, ContentType::IMAGE_JPEG},
        {".jpe"s, ContentType::IMAGE_JPEG},
        {".jpeg"s, ContentType::IMAGE_JPEG},
        {".gif"s, ContentType::IMAGE_GIF},
        {".bmp"s, ContentType::IMAGE_BMP},
        {".ico"s, ContentType::IMAGE_ICO},
        {".tiff"s, ContentType::IMAGE_TIFF},
        {".tif"s, ContentType::IMAGE_TIFF},
        {".svg"s, ContentType::IMAGE_SVG},
        {".svgz"s, ContentType::IMAGE_SVG},
        {".mp3"s, ContentType::AUDIO_MP3}
    };

    StringResponse handlers_statics::MakeStringResponse(http::status status,
                                std::string_view body,
                                unsigned http_version,
                                bool keep_alive,
                                std::string_view content_type,
                                bool noCache
                                ) {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        if(noCache){
            response.set(http::field::cache_control, "no-cache"sv);
        }
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    std::string handlers_statics::BadRequest(const std::string& code, const std::string& message){
        boost::json::object jbody;
        jbody["code"] = code;
        jbody["message"] = message;
        return boost::json::serialize(jbody);
    }

    StringResponse handlers_statics::BadMethodResponse(unsigned http_version, bool keep_alive, const std::string& method, const std::string& message){
        StringResponse response = 
            handlers_statics::MakeStringResponse(
                http::status::method_not_allowed, 
                handlers_statics::BadRequest("invalidMethod"s, message),
                http_version,
                keep_alive,
                ContentType::APPLICATION_JSON
            );
        response.set("Allow", method);

        return response;
    }

    StringResponse handlers_statics::InvalidTokenResponse(unsigned http_version, bool keep_alive){
        StringResponse response = 
            handlers_statics::MakeStringResponse(
                http::status::unauthorized, 
                handlers_statics::BadRequest("invalidToken"s, "Authorization header is missing"s),
                http_version,
                keep_alive,
                ContentType::APPLICATION_JSON
            );
        response.set("Cache-Control", "no-cache");

        return response;
    }

    StringResponse handlers_statics::UnknownTokenResponse(unsigned http_version, bool keep_alive){
        StringResponse response = 
            handlers_statics::MakeStringResponse(
                http::status::unauthorized, 
                handlers_statics::BadRequest("unknownToken"s, "Player token has not been found"s),
                http_version,
                keep_alive,
                ContentType::APPLICATION_JSON
            );
        response.set("Cache-Control", "no-cache");

        return response;
    }
}