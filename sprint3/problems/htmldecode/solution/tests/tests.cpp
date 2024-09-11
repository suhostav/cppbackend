#include <catch2/catch_test_macros.hpp>

#include "../src/htmldecode.h"

using namespace std::literals;

TEST_CASE("Text without mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode(""sv) == ""s);
    CHECK(HtmlDecode("hello"sv) == "hello"s);
}

TEST_CASE("Text with mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&lthello&gt") == "<hello>"s);
    CHECK(HtmlDecode("&LT&amp&APOShello&quot&GT") == "<&'hello\">"s);
    CHECK(HtmlDecode("&LT&amphe&APOSllo&quot&GT") == "<&he'llo\">"s);
    CHECK(HtmlDecode("&LT;&amphe&APOSllo&quot&GT;") == "<&he'llo\">"s);
}

TEST_CASE("Text with bad mnemonics", "[HtmlDecode]") {
    CHECK(HtmlDecode("&lThello&Gt") == "&lThello&Gt"s);
    CHECK(HtmlDecode("&apohello&qu") == "&apohello&qu"s);
}

