#pragma once
#include "http_server.h"
#include "model.h"
#include <boost/json.hpp>
#include <optional>

namespace http_handler {
namespace beast = boost::beast;
namespace http = beast::http;
using namespace std::literals;

using StringResponse = http::response<http::string_body>;

class RequestHandler {
public:
    explicit RequestHandler(model::Game& game)
        : game_{game} {
    }

    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    template <typename Body, typename Allocator, typename Send>
    void operator()(http::request<Body, http::basic_fields<Allocator>>&& req, Send&& send) {
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
        constexpr static std::string_view TEXT_HTML =   "text/html"sv;
        constexpr static std::string_view APPLICATION_JSON =   "application/json"sv;
    };
    StringResponse MakeStringResponse(http::status status,
                                        std::string_view body,
                                        unsigned http_version,
                                        bool keep_alive,
                                        std::string_view content_type = ContentType::TEXT_HTML
                                        ){
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    std::string MapsListResponse(){
        auto maps = game_.GetMaps();
        boost::json::array jbody;
        for(model::Map map : maps){
            boost::json::object jmap;
            jmap["id"] = *map.GetId();
            jmap["name"] = map.GetName();
            jbody.emplace_back(jmap);
        }
        return boost::json::serialize(jbody);
    }

    std::pair<bool, std::string> MapResponse(const std::string& map_id){
        std::string body;
        model::Map::Id id{map_id};
        const model::Map* map_ptr = game_.FindMap(id);
        if(map_ptr) {
            // body = map_ptr->ToJson();
            boost::json::object jbody;
            jbody["id"] = *map_ptr->GetId();
            jbody["name"] = map_ptr->GetName();
            boost::json::array jroads;
            const auto roads = map_ptr->GetRoads();
            for(const auto& road : roads){
                boost::json::object jroad;
                jroad["x0"] = road.GetStart().x;
                jroad["y0"] = road.GetStart().y;
                if(road.IsHorizontal()){
                    jroad["x1"] = road.GetEnd().x;
                } else {
                    jroad["y1"] = road.GetEnd().y;
                }
                jroads.emplace_back(jroad);
            }
            jbody["roads"] = jroads;
            boost::json::array jbuildings;
            const auto buildings = map_ptr->GetBuildings();
            for(const auto& building : buildings){
                boost::json::object jbuilding;
                jbuilding["x"] = building.GetBounds().position.x;
                jbuilding["y"] = building.GetBounds().position.y;
                jbuilding["w"] = building.GetBounds().size.width;
                jbuilding["h"] = building.GetBounds().size.height;
                jbuildings.emplace_back(jbuilding);              
            }
            jbody["buildings"] = jbuildings;
            boost::json::array joffices;
            const auto offices = map_ptr->GetOffices();
            for(const auto& office : offices){
                boost::json::object joffice;
                joffice["id"] = *office.GetId();
                joffice["x"] = office.GetPosition().x;
                joffice["y"] = office.GetPosition().y;
                joffice["offsetX"] = office.GetOffset().dx;
                joffice["offsetY"] = office.GetOffset().dy;
                joffices.emplace_back(joffice);
            }
            jbody["offices"] = joffices;
            body = boost::json::serialize(jbody);
            return {true, body};
        } else {
            boost::json::object jbody;
            jbody["code"] = "mapNotFound";
            jbody["message"] = "Map not found";
            body = boost::json::serialize(jbody);
            return {false, body};
        }
    }

    std::string BadRequest(){
        boost::json::object jbody;
        jbody["code"] = "badRequest";
        jbody["message"] = "Bad request";
        return boost::json::serialize(jbody);
    }
    model::Game& game_;
};


}  // namespace http_handler

