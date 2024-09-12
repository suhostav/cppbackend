#include "ApiHandler.h"
#include <iostream>


namespace http_handler {

    std::string ApiHandler::MapsListResponse() const{
        auto maps = game_app_.GetMaps();
        boost::json::array jbody;
        for(const model::Map& map : maps){
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
            jbody["lootTypes"] = boost::json::parse(map_ptr->GetLootTypes());
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

    boost::json::array ApiHandler::GetRoadsArray(const model::Map& map) {
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

    StringResponse ApiHandler::HandleApiRequest(const StringRequest& req) const{
        StringResponse response;
        try{
            response = DoHandleApiRequest(req);
            response.set("Cache-Control", "no-cache");
        } catch(const model::BadMapIdException& ex){
            response = handlers_statics::MakeStringResponse(
                http::status::not_found,
                handlers_statics::BadRequest("mapNotFound", ex.what()),
                req.version(),
                req.keep_alive(),
                ContentType::APPLICATION_JSON
            );
            response.set("Cache-Control", "no-cache");
        } catch(const BadRequestException& ex){
            response = handlers_statics::MakeStringResponse(
                http::status::bad_request,
                handlers_statics::BadRequest(ex.GetCode(), ex.what()),
                req.version(),
                req.keep_alive(),
                ContentType::APPLICATION_JSON
            );
            response.set("Cache-Control", "no-cache");
        } catch(const BadMethodException& ex) {
            response = handlers_statics::BadMethodResponse(req.version(), req.keep_alive(), 
                    ex.GetCode(), ex.what());
        } catch(const UnknownTokenExeption& ex){
            response = handlers_statics::UnknownTokenResponse(req.version(), req.keep_alive());
        } catch( const InvalidTokenExeption& ){
            response = handlers_statics::InvalidTokenResponse(req.version(), req.keep_alive());
        }
        return response;
    }


    StringResponse ApiHandler::DoHandleApiRequest(const StringRequest& req) const{
        std::string target{req.target()};
        target = DecodeUri(target);
        StringResponse response{};
        std::string body;
        const auto text_response = [&req, this](http::status status, std::string_view text, std::string_view content_type) {
            return handlers_statics::MakeStringResponse(status, text, req.version(), req.keep_alive(), content_type);
        };
        if(target == api_requests::maps_list_request) {
            CheckGetOrHead(req);
            body = IsGetRequest(req) ? MapsListResponse() : "";
            response =  text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
        } else if(target.starts_with(api_requests::map_request) && target.size() > api_requests::map_request.size()){
            CheckGetOrHead(req);
            std::string map_id{target.substr(api_requests::map_request.size())};
            auto result = MapResponse(map_id);
             body = IsGetRequest(req) ? result.second : "";
            if(result.first == true){
                response =  text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
            } else {
                response = text_response(http::status::not_found, body, ContentType::APPLICATION_JSON);
            }
        } else if(target == api_requests::join_game_request){
            CheckPost(req);
            body = JoinGameResponse(req);
            response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
        } else if(target == api_requests::players_list_requesst){
            CheckGetOrHead(req);
            body = IsGetRequest(req) ? PlayersResponse(GetSession(req)) : "";
            response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
        } else if(target == api_requests::state_requesst){
            CheckGetOrHead(req);
            body = IsGetRequest(req) ? StateResponse(GetSession(req)) : "";
            response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
        } else if(target == api_requests::action_request){
            CheckPost(req);
            body = ActionResponse(req);
            response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
        } else if(target == api_requests::tick_request){
            CheckPost(req);
            body = TickResponse(req);
            response = text_response(http::status::ok, body, ContentType::APPLICATION_JSON);
        } else {
            response = text_response(http::status::bad_request, BadRequest(), ContentType::APPLICATION_JSON);
        }
        return response;
    }


    std::string ApiHandler::BadRequest() const{
        boost::json::object jbody;
        jbody["code"] = "badRequest";
        jbody["message"] = "Bad request";
        return boost::json::serialize(jbody);
    }

    std::string ApiHandler::JoinGameResponse(const StringRequest& req) const{
        boost::json::object req_body{boost::json::parse(req.body()).as_object()};
        std::string_view dog_name{req_body.at("userName").as_string()};
        if(dog_name.empty()){
            throw BadRequestException("invalidArgument", "Invalid name");
        }
        std::string_view map_id{req_body.at("mapId").as_string()};
        return JoinGameResponse(dog_name, map_id);
    }

    std::string ApiHandler::JoinGameResponse(std::string_view dog_name, std::string_view map_id) const{
        app::JoinInfo join_info = game_app_.JoinGame(dog_name, map_id);
        boost::json::object jbody;
        jbody["playerId"] = join_info.player->GetDog()->GetId();
        jbody["authToken"] = *(join_info.token);
        return boost::json::serialize(jbody);
    }

std::optional<app::Token> ApiHandler::TryExtractToken(const StringRequest& req) const {
    std::string_view auth_str = req["Authorization"];
    if(auth_str.size() == 0){
        return {};
    }
    if(!auth_str.starts_with(handlers_statics::BEARER_STR)){
        return {};
    }
    std::string_view token_sv = auth_str.substr(handlers_statics::BEARER_STR.size());
    std::string token_str;
    if(token_sv.size() != handlers_statics::token_length){
        return {};
    }
    for(auto tc : token_sv){
        char c = (char)tolower(tc);
        if(isdigit(c) || (c >= 'a' && c <= 'f')){
            token_str += c;
        } else {
            return {};
        }
    }
    return app::Token(token_str);
}

const model::GameSession* ApiHandler::GetSession(const StringRequest& req) const{
    std::optional<app::Token> token = TryExtractToken(req);
    if(!token){
        throw InvalidTokenExeption("");
    }
    auto session = GetSessionByToken(req["Authorization"]);
    if(session == nullptr){
        throw UnknownTokenExeption("");
    }
    return session;
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

std::string ApiHandler::StateResponse(const model::GameSession* session) const{
    boost::json::object jbody;
    boost::json::object jplayers;
    const std::unordered_map<std::uint64_t, model::Dog*>& dogs = session->GetSessionDogs();
    for(auto& dog : dogs){
        boost::json::array point{dog.second->GetPoint().x, dog.second->GetPoint().y};
        boost::json::array speed{dog.second->GetSpeed().hs, dog.second->GetSpeed().vs};
        std::string dir_str{static_cast<char>(dog.second->GetDir())};
        boost::json::string dir{dir_str};
        boost::json::object jplayer;
        jplayer["pos"] = point;
        jplayer["speed"] = speed;
        jplayer["dir"] = dir;
        jplayers[std::to_string(dog.first)] = jplayer;
    }
    jbody["players"] = jplayers;
    boost::json::object jloots;
    int i = 0;
    for(const auto& loot : session->GetLoots()){
        boost::json::array jpoint{loot.second.x, loot.second.y};
        boost::json::object jloot;
        jloot["type"] = loot.first;
        jloot["pos"] = jpoint;
        jloots[std::to_string(i++)] = jloot;
    }
    jbody["lostObjects"] = jloots;
    return boost::json::serialize(jbody);
}

std::string ApiHandler::ActionResponse(const StringRequest& req) const{
    auto session = const_cast<model::GameSession*>(GetSession(req));
    auto token = TryExtractToken(req);
    boost::json::object req_body;
    req_body = boost::json::parse(req.body()).as_object();
    char dir = (char)toupper(req_body["move"].as_string()[0]);
    bool valid_dir = (dir == 'U' || dir == 'D' || dir == 'L' || dir == 'R');
    if(!valid_dir){
        throw BadRequestException("InvalidDirection","Invalid Direction");
    }
    game_app_.SetPlayerSpeed(*token, session, dir);
    return "{}"s;    
}

std::string ApiHandler::TickResponse(const StringRequest& req) const{
    if(!test_mode_){
        throw BadRequestException("badRequest", "Invalid endpoint");
    }
    boost::json::object req_body;
    try{
        req_body = boost::json::parse(req.body()).as_object();
    } catch(const std::exception& ){
        throw BadRequestException("invalidArgument", "Failed to parse tick request JSON");
    }
    if(!req_body.contains("timeDelta") || !req_body["timeDelta"].is_int64()){
        throw BadRequestException("invalidArgument", "Failed to parse tick request JSON");
    }
    std::int64_t time_period = req_body["timeDelta"].as_int64();
    game_app_.Move(time_period * 1ms);
    return "{}"s;    
}

}