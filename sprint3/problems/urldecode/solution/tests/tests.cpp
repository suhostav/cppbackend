#define BOOST_TEST_MODULE urlencode tests
#include <boost/test/unit_test.hpp>

#include "../src/urldecode.h"

BOOST_AUTO_TEST_CASE(UrlDecode_tests) {
    using namespace std::literals;

    BOOST_TEST(UrlDecode(""sv) == ""s);
    BOOST_TEST(UrlDecode("HelloWorld"sv) == "HelloWorld"sv);
    BOOST_TEST(UrlDecode("Hello%20World !"sv) == "Hello World !"sv);
    BOOST_TEST(UrlDecode("Hello%2bWorld !"sv) == "Hello+World !"sv);
    BOOST_TEST(UrlDecode("Hello%2BWorld !"sv) == "Hello+World !"sv);
    BOOST_TEST(UrlDecode("Hello+World !"sv) == "Hello World !"sv);
    BOOST_CHECK_THROW(UrlDecode("Hello%G0World !"sv), std::invalid_argument);
    BOOST_CHECK_THROW(UrlDecode("Hello%2"sv), std::invalid_argument);
}