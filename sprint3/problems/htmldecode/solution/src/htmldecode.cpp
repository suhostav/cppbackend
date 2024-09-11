#include "htmldecode.h"

string_view resize_str(size_t n, string_view str){
    str = str.substr(n);
    if(str.size() > 0 && str[0] == ';'){
        str = str.substr(1);
    }
    return str;
}

std::string HtmlDecode(std::string_view str) {
    string result;
    while(str.size() > 0){
        if(str.starts_with("&lt") || str.starts_with("&LT") ){
            result += '<';
            str = resize_str(3, str);
        } else if(str.starts_with("&gt") || str.starts_with("&GT") ){
            result += '>';
            str = resize_str(3, str);
        } else if(str.starts_with("&amp") || str.starts_with("&AMP") ){
            result += '&';
            str = resize_str(4, str);
        } else if(str.starts_with("&apos") || str.starts_with("&APOS") ){
            result += '\'';
            str = resize_str(5, str);
        } else if(str.starts_with("&quot") || str.starts_with("&QUOT") ){
            result += '\"';
            str = resize_str(5, str);
        } else {
            result += str[0];
            str = str.substr(1);
        }
    }
    return result;
}
