#pragma once

#define BOOST_BEAST_USE_STD_STRING_VIEW
#include <functional>
#include <optional>
#include "handlers_tools.h"
#include "Dog.h"
#include "GameApp.h"

namespace http_handler {

using StringResponse = http::response<http::string_body>;
using StringRequest = http::request<http::string_body>;

class ApiHandler {
public:
    ApiHandler(app::GameApp& game_app) : game_app_(game_app){
    }

    // bool IsApiRequest(const StringRequest& request) const;
    bool IsGetRequest(const StringRequest& req) const{
        return req.method_string() == handlers_statics::GET_STR;
    }
    bool IsHeadRequest(const StringRequest& req) const{
        return req.method_string() == handlers_statics::HEAD_STR;
    }
    bool IsGetOrHeadRequest(const StringRequest& req) const{
        return IsGetRequest(req) || IsHeadRequest(req);
    }
    void CheckGetOrHead(const StringRequest& req) const{
        if(!IsGetOrHeadRequest(req)){
            throw BadMethodException("GET, HEAD"s, "Invalid method"s);
        }
    }
    void CheckPost(const StringRequest& req) const{
        if(!IsPostRequest(req)){
            throw BadMethodException("POSTD"s, "Invalid method"s);
        }
    }

    bool IsPostRequest(const StringRequest& req) const{
        return req.method_string() == handlers_statics::POST_STR;
    }
    StringResponse HandleApiRequest(const StringRequest& request) const;
    StringResponse DoHandleApiRequest(const StringRequest& request) const;


private:
    boost::json::array GetRoadsArray(const model::Map& map) const;
    boost::json::array GetBuildingssArray(const model::Map& map) const;
    boost::json::array GetOfficesArray(const model::Map& map) const;

    std::string MapsListResponse() const;
    std::pair<bool, std::string> MapResponse(const std::string& map_id) const;
    std::string BadRequest() const;
    std::string JoinGameResponse(const StringRequest& req) const;
    std::string JoinGameResponse(std::string_view dog_name, std::string_view map_id) const;
    std::optional<app::Token> TryExtractToken(const StringRequest& req) const;
    const model::GameSession* GetSession(const StringRequest& req) const;
    // std::string AutorizedResponse(const std::string& req_path, std::string_view token_str) const;
    const model::GameSession* GetSessionByToken(std::string_view token_str) const;
    std::string PlayersResponse(const model::GameSession* session) const;
    std::string StateResponse(const model::GameSession* session) const;
    std::string ActionResponse(const StringRequest& req) const;
    std::string TickResponse(const StringRequest& req) const;

    app::GameApp& game_app_;
};

} //namespace http_handler