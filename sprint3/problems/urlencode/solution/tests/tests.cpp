#include <gtest/gtest.h>

#include "../src/urlencode.h"

using namespace std::literals;

std::string makeStringWithHiChars(){
    std::stringstream ss;
    ss << "Hello\t World!" << static_cast<unsigned char>(129);
    return ss.str();
}


TEST(UrlEncodeTestSuite, OrdinaryCharsAreNotEncoded) {
    EXPECT_EQ(UrlEncode(""sv), ""s);
    EXPECT_EQ(UrlEncode("hello"sv), "hello"s);
}

TEST(UrlEncodeTestSuite, SpesialCharsAreEncoded) {
    EXPECT_EQ(UrlEncode("HelloWorld!"sv), "HelloWorld%21"s);
    EXPECT_EQ(UrlEncode("Hello World!"sv), "Hello+World%21"s);
    EXPECT_EQ(UrlEncode("Hello\t World!"sv), "Hello%09+World%21"s);
    EXPECT_EQ(UrlEncode(makeStringWithHiChars()), "Hello%09+World%21%81"s);
}
/* Напишите остальные тесты самостоятельно */
