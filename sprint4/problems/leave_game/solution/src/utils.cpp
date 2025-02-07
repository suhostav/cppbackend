#include "utils.h"


namespace util {
using std::locale, std::cout, std::endl;


string_view Trim(string_view str) {
    const auto start = str.find_first_not_of(' ');
    if (start == str.npos) {
        return {};
    }
    return str.substr(start, str.find_last_not_of(' ') + 1 - start);
}

/**
 * Разбивает строку string на n строк, с помощью указанного символа-разделителя delim
 */
vector<string_view> Split(string_view str, char delim, bool remove_empty) {
    vector<string_view> result;

    size_t pos = 0;
    while ((pos = str.find_first_not_of(' ', pos)) < str.length()) {
        auto delim_pos = str.find(delim, pos);
        if (delim_pos == str.npos) {
            delim_pos = str.size();
        }
        auto substr = Trim(str.substr(pos, delim_pos - pos));
        if ( !(remove_empty && substr.empty())) {
            result.push_back(substr);
        }
        pos = delim_pos + 1;
    }

    return result;
}

string Join(vector<std::string_view> elems, char delim){
    if(elems.size() == 0){
        return {};
    }
    if(elems.size() == 1){
        return string(elems[0]);
    }
    string joined{elems[0]};
    for(size_t i = 1; i < elems.size(); ++i){
        joined += delim;
        joined += elems[i];
    }

    return joined;
}

std::string Join(vector<std::string> elems, std::string delim){
    if(elems.size() == 0){
        return {};
    }
    if(elems.size() == 1){
        return elems[0];
    }
    string joined{elems[0]};
    for(size_t i = 1; i < elems.size(); ++i){
        joined += delim;
        joined += elems[i];
    }

    return joined;
}

std::vector<std::string> PrepareTags(std::string_view tags_data){
    std::set<std::string> tags;
    std::vector<std::string_view> elems = util::Split(tags_data, ',', true);
    for(auto elem : elems){
        std::vector<std::string_view> words = util::Split(elem, ' ', true);
        tags.emplace(Join(words, ' '));
    }

    return {tags.begin(), tags.end()};
}

locale SetWindowsLocale() {
    locale::global(locale(""));
    auto loc = locale();
    cout.imbue(loc);
    return loc;
}

}