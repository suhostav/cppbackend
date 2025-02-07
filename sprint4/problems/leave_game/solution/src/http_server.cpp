
#include "http_server.h"

#include <boost/asio/dispatch.hpp>
#include <iostream>

namespace http_server {

    BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
    BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

    void ReportError(beast::error_code ec, std::string_view what) {

        boost::json::value error_data{{"code"s, 1},
            {"text"s, ec.message()},
            {"what", what}};
        BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, error_data) << "error"sv;
}

void SessionBase::Run(){
    net::dispatch(stream_.get_executor(),
        beast::bind_front_handler(&SessionBase::Read, GetSharedThis()));
}

void SessionBase::Read(){
    request_ = {};
    stream_.expires_after(30s);
    http::async_read(stream_, buffer_, request_,
        beast::bind_front_handler(&SessionBase::OnRead, GetSharedThis()));
}

void SessionBase::OnRead(beast::error_code ec, [[maybe_unused]] std::size_t bytes_read){
    using namespace std::literals;
    if(ec == http::error::end_of_stream){
        return Close();
    }
    if(ec){
        return ReportError(ec, "read"sv);
    }
    boost::json::value req_data{
        {"ip"s, stream_.socket().remote_endpoint().address().to_string()},
        {"URI"s, request_.target()},
        {"method"s, request_.method_string()}};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, req_data) << "request received"sv;
    //устанавливаем время начала обработки запроса
    start_request_handling_ = std::chrono::system_clock::now();
    HandleRequest(std::move(request_));
}

void SessionBase::Close(){
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
    if(ec){
        std::cerr << "Error while session closing: " << ec.what() << std::endl;
    }
}

void SessionBase::OnWrite(bool close, beast::error_code ec, [[maybe_unused]] std::size_t bytes_written){
    if(ec){
        return ReportError(ec, "write"sv);
    }
    if(close){
        return Close();
    }
    Read();
}

void SessionBase::WriteResponseLog(int response_time, int response_code, const std::string& content_type){
    boost::json::value res_data{
        {"response_time"s, response_time},
        {"code"s, response_code},
        {"content_type"s, content_type}};
    BOOST_LOG_TRIVIAL(info) << logging::add_value(additional_data, res_data) << "response sent"sv;
}

}  // namespace http_server

