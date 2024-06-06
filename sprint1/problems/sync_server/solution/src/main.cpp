#ifdef WIN32
#include <sdkddkver.h>
#endif
// boost.beast будет использовать std::string_view вместо boost::string_view
#define BOOST_BEAST_USE_STD_STRING_VIEW

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <iostream>
#include <thread>
#include <optional>
#include <thread>

namespace net = boost::asio;
using net::ip::tcp;
using namespace std::literals;
namespace beast = boost::beast;
namespace http = beast::http;
using std::cout, std::endl;
// Запрос, тело которого представлено в виде строки
using StringRequest = http::request<http::string_body>;
// Ответ, тело которого представлено в виде строки
using StringResponse = http::response<http::string_body>;

struct ContentType {
    ContentType() = delete;
    constexpr static std::string_view TEXT_HTML =   "text/html"sv;
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

std::optional<StringRequest> ReadRequest(tcp::socket& socket, beast::flat_buffer& buffer){
    beast::error_code ec;
    StringRequest req;
    http::read(socket, buffer, req, ec);
    if(ec == http::error::end_of_stream){
        return {};
    }
    if(ec){
        throw std::runtime_error("Failed to read request: "s.append(ec.what()));
    }
    return req;
}

StringResponse HandleRequest(StringRequest&& req) {
    const auto text_response = [&req](http::status status, std::string_view text) {
        return MakeStringResponse(status, text, req.version(), req.keep_alive());
    };

    // Здесь можно обработать запрос и сформировать ответ, но пока всегда отвечаем: Hello
    const std::string_view head_str{"HEAD"};
    const std::string_view get_str{"GET"};
    const std::string_view method_str{req.method_string()};
    if( method_str == head_str || method_str == get_str){
        std::string body;
        if( method_str == get_str){
            body = "Hello, ";
            body += req.target().substr(1);
        }
        return text_response(http::status::ok, body);
    } else {
        auto response = text_response(http::status::method_not_allowed, "Invalid method"sv);
        response.set("Allow"sv, "GET, HEAD"sv);
        response.set(http::field::content_type, "text/html"sv);
        return response;
    }
} 

void DumpRequest(const StringRequest& req){
    std::cout << req.method_string() << ' ' << req.target() << std::endl;
    for(const auto& header : req){
        std::cout << " "sv << header.name_string() << ": "sv << header.value() << std::endl;
    }
}

template <typename RequestHandler>
void HandleConnection(tcp::socket& socket, RequestHandler&& handler_request){
    try{
        beast::flat_buffer buffer;
        while(auto request = ReadRequest(socket, buffer)){
            DumpRequest(*request);

            StringResponse response = handler_request(*std::move(request));
            http::write(socket, response);
            if(response.need_eof()){
                break;
            }
        }
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

void HandleConnection(tcp::socket& socket){
    try{
        beast::flat_buffer buffer;
        while(auto request = ReadRequest(socket, buffer)){
            DumpRequest(*request);

            StringResponse response = HandleRequest(*std::move(request));
            http::write(socket, response);
            if(response.need_eof()){
                break;
            }
        }
    } catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
    }
    beast::error_code ec;
    socket.shutdown(tcp::socket::shutdown_send, ec);
}

int main() {
    net::io_context ioc;

    const auto address = net::ip::make_address("0.0.0.0");
    constexpr unsigned short port = 3333;

    tcp::acceptor acceptor(ioc, {address, port});
    for(int i = 0; i < 3; ++i){
        std::cout << "Start waiting request...\n";
        tcp::socket socket(ioc);
        acceptor.accept(socket);
        HandleConnection(socket, [](StringRequest&& req){ return HandleRequest(std::move(req));});
        // HandleConnection(socket);
    }
}