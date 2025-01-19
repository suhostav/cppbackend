#pragma once

#include <iostream>
#include <locale>
#include <map>
#include <set>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace util {
using std::string, std::string_view, std::vector;
string_view Trim(string_view str);
vector<string_view> Split(string_view string, char delim, bool remove_empty = false);
string Join(vector<std::string_view> elems, char delim);
string Join(vector<std::string> elems, std::string delim);
std::vector<std::string> PrepareTags(std::string_view tags_data);
std::locale SetWindowsLocale();

}