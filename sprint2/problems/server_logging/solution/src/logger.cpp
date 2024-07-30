#include "logger.h"

BOOST_LOG_ATTRIBUTE_KEYWORD(timestamp, "TimeStamp", boost::posix_time::ptime)
BOOST_LOG_ATTRIBUTE_KEYWORD(additional_data, "AdditionalData", boost::json::value)

void JsonFormatter(logging::record_view const& rec, logging::formatting_ostream& strm){

    boost::json::object jlog;
    auto ts = *rec[timestamp];
    jlog["timestamp"] = to_iso_extended_string(ts);
    jlog["data"] = *rec[additional_data];
    jlog["message"] = *rec[logging::expressions::smessage];
    strm << boost::json::serialize(jlog);
}

void InitLog(){
    logging::add_common_attributes();

    logging::add_console_log(
        std::clog,
        keywords::format = &JsonFormatter,
        keywords::auto_flush = true
    );
}