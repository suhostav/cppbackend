#include <catch2/catch_test_macros.hpp>
#include <string>

#include "../src/util/utils.h"
using namespace std::literals;

TEST_CASE("TAGs preparation") {
    constexpr std::string_view tags_data{"adventure, dog,   gold   rush  ,  dog,,dogs "};
    std::vector<std::string_view> tags = util::Split(tags_data, ',', true);
    std::vector<std::string_view> words = util::Split(tags[2], ' ', true);
    std::string joined_tag = util::Join(words, ' ');
    std::vector<std::string> prepared = util::PrepareTags(tags_data);
    CHECK(tags.size() == 5);
    CHECK(tags[0] == "adventure"sv);
    CHECK(words.size() == 2);
    CHECK(words[0] == "gold"sv);
    CHECK(words[1] == "rush"sv);
    CHECK(joined_tag == "gold rush"s);
    CHECK(prepared.size() == 4);
    CHECK(prepared[3] == "gold rush"s);
}