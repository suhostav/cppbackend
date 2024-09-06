#include "urlencode.h"

std::stringstream& EncodeChar(std::stringstream& out, char c){
    if( c == ' '){
        out << '+';
    } else if( c <= '!' || c == '#' || c == '$' || 
        (c >= '&' && c <= ',') || c == '/' || 
        c == ':' || c == ';' || c == '=' || 
        c == '?' || c == '@' || c == '[' || 
        c == ']' || c >= 128 ){
            out << '%' << std::hex << static_cast<int>(c);
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

    return ss.str();
}