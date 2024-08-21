#include "ApiHandler.h"
#include <iostream>


namespace http_handler {


    // StringResponse ApiHandler::MakeStringResponse(http::status status,
    //                                 std::string_view body,
    //                                 unsigned http_version,
    //                                 bool keep_alive,
    //                                 std::string_view content_type
    //                                 ) const {
    //     StringResponse response(status, http_version);
    //     response.set(http::field::content_type, content_type);
    //     response.body() = body;
    //     response.content_length(body.size());
    //     response.keep_alive(keep_alive);
    //     return response;
    // }

    std::string ApiHandler::MapsListResponse() const{
        auto maps = game_app_.GetMaps();
        boost::json::array jbody;
        for(model::Map map : maps){
            boost::json::object jmap;
            jmap["id"] = *map.GetId();
            jmap["name"] = map.GetName();
            jbody.emplace_back(jmap);
        }
        return boost::json::serialize(jbody);
    }

    std::pair<bool, std::string> ApiHandler::MapResponse(const std::string& map_id) const{
        std::string body;
        model::Map::Id id{map_id};
        const model::Map* map_ptr = game_app_.FindMap(id);
        if(map_ptr) {
            // body = map_ptr->ToJson();
            boost::json::object jbody;
            jbody["id"] = *map_ptr->GetId();
            jbody["name"] = map_ptr->GetName();
            jbody["roads"] = GetRoadsArray(*map_ptr);
            jbody["buildings"] = GetBuildingssArray(*map_ptr);
            jbody["offices"] = GetOfficesArray(*map_ptr);
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

    boost::json::array ApiHandler::GetRoadsArray(const model::Map& map) const{
        boost::json::array jroads;
        const auto roads = map.GetRoads();
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
        return jroads;
    }

    boost::json::array ApiHandler::GetBuildingssArray(const model::Map& map) const{
        boost::json::array jbuildings;
        const auto buildings = map.GetBuildings();
        for(const auto& building : buildings){
            boost::json::object jbuilding;
            jbuilding["x"] = building.GetBounds().position.x;
            jbuilding["y"] = building.GetBounds().position.y;
            jbuilding["w"] = building.GetBounds().size.width;
            jbuilding["h"] = building.GetBounds().size.height;
            jbuildings.emplace_back(jbuilding);              
        }
        return jbuildings;
    }

    boost::json::array ApiHandler::GetOfficesArray(const model::Map& map) const{
            boost::json::array joffices;
            const auto offices = map.GetOffices();
            for(const auto& office : offices){
                boost::json::object joffice;
                joffice["id"] = *office.GetId();
                joffice["x"] = office.GetPosition().x;
                joffice["y"] = office.GetPosition().y;
                joffice["offsetX"] = office.GetOffset().dx;
                joffice["offsetY"] = office.GetOffset().dy;
                joffices.emplace_back(joffice);
            }
        return joffices;
    }

    StringResponse ApiHandler::HandleApiRequest(const StringRequest& request) const{
        StringResponse response;
        try{
            response = DoHandleApiRequest(request);
            response.set("Cache-Control", "no-cache");
        } catch(const model::BadMapIdException& ex){
            response = handlers_statics::MakeStringResponse(
                http::status::not_found,
                handlers_statics::BadRequest("mapNotFound", ex.what()),
                request.version(),
                request.keep_alive(),
                ContentType::APPLICATION_JSON
            );
            response.set("Cache-Control", "no-cache");
        } catch(const BadRequestException& ex){
            response = handlers_statics::MakeStringResponse(
                http::status::bad_request,
                handlers_statics::BadRequest(ex.GetCode(), ex.what()),
                request.version(),
                request.keep_alive(),
                ContentType::APPLICATION_JSON
            );
            response.set("Cache-Control", "no-cache");
        }
        return response;
    }


    StringResponse ApiHandler::DoHandleApiRequest(const StringRequest& req) const{
        const static std::string maps_list_request{"/api/v1/maps"};
        const static std::string map_request{"/api/v1/maps/"};
        // const static std::string bad_api_request{"/api/"};
        const static std::string join_game_request{"/api/v1/game/join"};
        const static std::string players_list_requesst{"/api/v1/game/players"};
        std::string target{req.target()};
        target = DecodeUri(target);
        StringResponse response{};
        std::string body;
        const auto text_response = [&req, this](http::status status, std::string_view text, std::string_view content_type) {
            return handlers_statics::MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
        };
        if(target == maps_list_request) {
            if(IsGetOrHeadRequest(req)){
                if(IsGetRequest(req)) {
                    body = MapsListResponse();
                }
                response =  text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
            } else {
                response = handlers_statics::BadMethodResponse(req.version(), req.keep_alive(), 
                    "GET,HEAD", "Only GET or HEAD methods are expected");
            }
        } else if(target.starts_with(map_request) && target.size() > map_request.size()){
            if(IsGetOrHeadRequest(req)){
                std::string map_id{target.substr(map_request.size())};
                auto result = MapResponse(map_id);
                if(IsGetRequest(req)){
                    body = result.second;
                }
                if(result.first == true){
                    response =  text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
                } else {
                    response = text_response(http::status::not_found, body, ContentType::APPLICATION_JSON);
                }
            } else {
                response = handlers_statics::BadMethodResponse(req.version(), req.keep_alive(), 
                    "GET,HEAD", "Only GET or HEAD methods are expected");
            }
        } else if(target == join_game_request){
            if(IsPostRequest(req)){
                try{
                    // std::cout << "req body = " << req.body() << std::endl;
                    boost::json::object req_body{boost::json::parse(req.body()).as_object()};
                    std::string_view dog_name{req_body.at("userName").as_string()};
                    if(dog_name.empty()){
                        throw BadRequestException("invalidArgument", "Invalid name");
                    }
                    std::string_view map_id{req_body.at("mapId").as_string()};
                    // body = JoinGameResponse(std::forward<std::string>(dog_name), std::forward<std::string>(map_id));
                    body = JoinGameResponse(dog_name, map_id);
                    response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
                } catch(const BadRequestException& ex){
                    throw;
                } catch(const model::BadMapIdException& ex){
                    throw;
                } catch(...){
                    throw BadRequestException("invalidArgument", "Join game request parse error");
                }
            } else {
                response = handlers_statics::BadMethodResponse(req.version(), req.keep_alive(), "POST", "Only POST method is expected"s);
            }
        } else if(target == players_list_requesst){
            if(IsGetOrHeadRequest(req)){
                try{
                    if(req.count("Authorization") == 0 || !req.at("Authorization").starts_with("Bearer ")){
                        response = handlers_statics::InvalidTokenResponse(req.version(), req.keep_alive());
                    } else {
                        auto session = GetSessionByToken(req["Authorization"]);
                        if(session == nullptr){
                            response = handlers_statics::UnknownTokenResponse(req.version(), req.keep_alive());
                        } else {
                            if(IsGetRequest(req)) {
                                body = PlayersResponse(session);
                            }
                            response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
                        }
                    }
                } catch(const BadRequestException& ex){
                    throw;
                } catch(const model::BadMapIdException& ex){
                    throw;
                } catch(...){
                    throw BadRequestException("invalidArgument", "Join game request parse error");
                }
            } else {
                response = handlers_statics::BadMethodResponse(req.version(), req.keep_alive(), 
                    "GET,HEAD", "Only GET or HEAD methods are expected");
            }
        } else {
                body = BadRequest();
                response = text_response(http::status::bad_request, body, ContentType::APPLICATION_JSON);
            }
            return response;
    }

    std::string ApiHandler::BadRequest() const{
        boost::json::object jbody;
        jbody["code"] = "badRequest";
        jbody["message"] = "Bad request";
        return boost::json::serialize(jbody);
    }

    std::string ApiHandler::JoinGameResponse(std::string_view dog_name, std::string_view map_id) const{
        app::JoinInfo join_info = game_app_.JoinGame(dog_name, map_id);
        boost::json::object jbody;
        jbody["playerId"] = join_info.player->GetDog()->GetId();
        jbody["authToken"] = *(join_info.token);
        return boost::json::serialize(jbody);
    }

std::string ApiHandler::AutorizedResponse(const std::string& req_path, std::string_view token_str) const{
    std::string result;
    if(token_str.size() == 0){

    }
    app::Token token{std::string(token_str)};
    app::Player* player = game_app_.FindPlayerByToken(token);

    return result;
}

const model::GameSession* ApiHandler::GetSessionByToken(std::string_view token_str) const {
    app::Token token{std::string(token_str.substr(7))};
    return game_app_.GetPlayerSession(token);
}

std::string ApiHandler::PlayersResponse(const model::GameSession* session) const{
    boost::json::object jbody;
    const std::unordered_map<std::uint64_t, model::Dog*>& dogs = session->GetSessionDogs();
    for(auto& dog : dogs){
        jbody[std::to_string(dog.first)] = {"name", dog.second->GetName() };
    }

    return boost::json::serialize(jbody);
}

}