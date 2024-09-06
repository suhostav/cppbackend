#include <sstream>
#include "urldecode.h"

#include <charconv>
#include <stdexcept>

char DecodeChar(std::string_view in){
    if(in.size() < 2){
        throw std::invalid_argument("Invalid hex digit");
    }
    auto hex_toint = [](char c){
        if(isdigit(c)) return c - '0';
        if(c >= 'a' && c <= 'f') return c - 'a' + 10;
        if(c >= 'A' && c <= 'F') return c - 'A' + 10;
        throw std::invalid_argument("Invalid hex digit");
    };

    return 16 * hex_toint(in[0]) + hex_toint(in[1]);
}

std::string UrlDecode(std::string_view uri_str) {
        std::string result;
        for(size_t i = 0; i < uri_str.size(); ++i){
            char c = uri_str[i];
            if(c == '%'){
                result += DecodeChar(uri_str.substr(i+1));
                i += 2;
            } else if(c == '+'){
                result += ' ';
            } else if(c != '\0'){
                result += c;
            } else {
                break;
            }
        }

        return result;
    }

std::stringstream& EncodeChar(std::stringstream& out, char c){
    if( c <= '!' || c == '#' || c == '$' || 
        (c >= '&' && c <= ',') || c == '/' || 
        c == ':' || c == ';' || c == '=' || 
        c == '?' || c == '@' || c == '[' || c == ']'){
            out << std::hex << static_cast<int>(c);
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