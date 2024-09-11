#include "urlencode.h"

std::stringstream& EncodeChar(std::stringstream& out, unsigned char c){
    if( c == ' '){
        out << '+';
    } else if( c <= '!' || c == '#' || c == '$' || 
        (c >= '&' && c <= ',') || c == '/' || 
        c == ':' || c == ';' || c == '=' || 
        c == '?' || c == '@' || c == '[' || 
        c == ']' || c >= 128 ){
            out << '%' << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(c);
    } else {
        out << c;
    }

    return out;
}


std::string UrlEncode(std::string_view str){
    std::stringstream ss;
    for(char c : str){
        EncodeChar(ss, c);
    }
    std::string result{ss.str()};
    return result;
}