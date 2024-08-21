#include "request_handler.h"

namespace http_handler {

        StringResponse RequestHandler::MakeStringResponse(http::status status,
                                        std::string_view body,
                                        unsigned http_version,
                                        bool keep_alive,
                                        std::string_view content_type
                                        ) const {
        StringResponse response(status, http_version);
        response.set(http::field::content_type, content_type);
        response.body() = body;
        response.content_length(body.size());
        response.keep_alive(keep_alive);
        return response;
    }

    std::string RequestHandler::MapsListResponse(){
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

    std::pair<bool, std::string> RequestHandler::MapResponse(const std::string& map_id) const{
        std::string body;
        model::Map::Id id{map_id};
        const model::Map* map_ptr = game_.FindMap(id);
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

    boost::json::array RequestHandler::GetRoadsArray(const model::Map& map) const{
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

    boost::json::array RequestHandler::GetBuildingssArray(const model::Map& map) const{
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

    boost::json::array RequestHandler::GetOfficesArray(const model::Map& map) const{
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


    std::string RequestHandler::BadRequest(){
        boost::json::object jbody;
        jbody["code"] = "badRequest";
        jbody["message"] = "Bad request";
        return boost::json::serialize(jbody);
    }


}  // namespace http_handler

