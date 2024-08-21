#include "request_handler.h"
#include <algorithm>

namespace http_handler {

    bool IsSubPath(fs::path path, fs::path base){
        // Приводим оба пути к каноничному виду (без . и ..)
        path = fs::weakly_canonical(path);
        base = fs::weakly_canonical(base);

        // Проверяем, что все компоненты base содержатся внутри path
        for (auto b = base.begin(), p = path.begin(); b != base.end(); ++b, ++p) {
            if (p == path.end() || *p != *b) {
                return false;
            }
        }
        return true;
    }

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

    StringResponse RequestHandler::NotFound(unsigned http_version, bool keep_alive){
        return MakeStringResponse(http::status::not_found, "File not found."s, http_version, keep_alive, ContentType::TEXT_PLAIN);
    }

    std::string RequestHandler::DecodeUri(const std::string& uri_str){
        std::string result;
        std::istringstream ss{uri_str};
        // ss << uri_str;
        while(ss){
            char c{0};
            ss >> c;
            if(c == '%'){
                result += DecodeChar(ss);
            } else if(c != '\0'){
                result += c;
            } else {
                break;
            }
        }

        return result;
    }

    char RequestHandler::DecodeChar(std::istream& in){
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

    FileResponse RequestHandler::MakeFileResponse(const std::string file_name, unsigned http_version, bool keep_alive) const{
        FileResponse response;
        fs::path file_path{file_name};
        file_path = fs::weakly_canonical(file_path);
        response.version(http_version);
        std::string file_ext{file_path.extension()};
        std::for_each(file_ext.begin(), file_ext.end(), [](char& c){ c = (char)tolower(c);});
        http::file_body::value_type file;
        // std::cout << "requested file: " << file_path.c_str() << ", file extension: " << file_ext << std::endl;
        if (beast::error_code ec; file.open(file_path.c_str(), beast::file_mode::read, ec), ec) {
            throw std::runtime_error("Failed to open file "s + file_name);
        }
        response.body() = std::move(file);
        if(extensions_.contains(file_ext)){
            response.set(http::field::content_type, extensions_[file_ext]);
        } else{
            response.set(http::field::content_type, ContentType::OCTET_STREAM);
        }
        response.prepare_payload();
        return response;
    }

    std::map<std::string, std::string_view> RequestHandler::extensions_ = {
        {".htm"s, RequestHandler::ContentType::TEXT_HTML},
        {".html"s, RequestHandler::ContentType::TEXT_HTML},
        {".css"s, RequestHandler::ContentType::TEXT_HTML},
        {".txt"s, RequestHandler::ContentType::TEXT_PLAIN},
        {".js"s, RequestHandler::ContentType::TEXT_JAVASCRIPT},
        {".json"s, RequestHandler::ContentType::APPLICATION_JSON},
        {".xml"s, RequestHandler::ContentType::APPLICATION_XML},
        {".png"s, RequestHandler::ContentType::IMAGE_PNG},
        {".jpg"s, RequestHandler::ContentType::IMAGE_JPEG},
        {".jpe"s, RequestHandler::ContentType::IMAGE_JPEG},
        {".jpeg"s, RequestHandler::ContentType::IMAGE_JPEG},
        {".gif"s, RequestHandler::ContentType::IMAGE_GIF},
        {".bmp"s, RequestHandler::ContentType::IMAGE_BMP},
        {".ico"s, RequestHandler::ContentType::IMAGE_ICO},
        {".tiff"s, RequestHandler::ContentType::IMAGE_TIFF},
        {".tif"s, RequestHandler::ContentType::IMAGE_TIFF},
        {".svg"s, RequestHandler::ContentType::IMAGE_SVG},
        {".svgz"s, RequestHandler::ContentType::IMAGE_SVG},
        {".mp3"s, RequestHandler::ContentType::AUDIO_MP3}
    };

}  // namespace http_handler

