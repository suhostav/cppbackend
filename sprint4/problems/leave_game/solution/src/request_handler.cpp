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

    std::string RequestHandler::BadRequest() const{
        boost::json::object jbody;
        jbody["code"] = "badRequest";
        jbody["message"] = "Bad request";
        return boost::json::serialize(jbody);
    }

    StringResponse RequestHandler::ReportServerError(unsigned version, bool keep_alive) const{
        return handlers_statics::MakeStringResponse(http::status::not_found, "Server Error."s, version, keep_alive, ContentType::TEXT_PLAIN);

    }

    StringResponse RequestHandler::NotFound(unsigned http_version, bool keep_alive) const{
        return handlers_statics::MakeStringResponse(http::status::not_found, "File not found."s, http_version, keep_alive, ContentType::TEXT_PLAIN);
    }

    FileResponse RequestHandler::MakeFileResponse(const std::string file_name, unsigned http_version, bool keep_alive) const{
        FileResponse response;
        fs::path file_path{file_name};
        file_path = fs::weakly_canonical(file_path);
        response.version(http_version);
        std::string file_ext{file_path.extension()};
        std::for_each(file_ext.begin(), file_ext.end(), [](char& c){ c = static_cast<char>(tolower(c));});
        http::file_body::value_type file;
        if (beast::error_code ec; file.open(file_path.c_str(), beast::file_mode::read, ec), ec) {
            throw std::runtime_error("Failed to open file "s + file_name);
        }
        response.body() = std::move(file);
        if(handlers_statics::extensions_.contains(file_ext)){
            response.set(http::field::content_type, handlers_statics::extensions_[file_ext]);
        } else{
            response.set(http::field::content_type, ContentType::OCTET_STREAM);
        }
        response.prepare_payload();
        return response;
    }

    RequestHandler::FileRequestResult RequestHandler::HandleFileRequest(const StringRequest& req) const{
        FileRequestResult result;
        std::string target = DecodeUri(std::string{req.target()});
        if(IsSubPath(root_.string() + target, root_)){
            try{
                if(target == "" || target == "/"){
                    target = "/index.html"s;
                }
                result = MakeFileResponse(root_.string() + target, req.version(), req.keep_alive());
            } catch(...){
                result = NotFound(req.version(), req.keep_alive());
            }

        }
        return result;
    }

    bool RequestHandler::IsApiRequest(const StringRequest& request) const{
        std::string target = DecodeUri(std::string{request.target()});
        return target.starts_with("/api/");
    }

}  // namespace http_handler

