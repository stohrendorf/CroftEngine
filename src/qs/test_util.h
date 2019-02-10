#pragma once

#define BOOST_TEST_MODULE qs_test

#include <boost/test/included/unit_test.hpp>

#define TPL(...) __VA_ARGS__

#define TEST_SAME(L, R) \
    BOOST_CHECK_MESSAGE( (std::is_same<L, R>::value), #L " == " #R )

#define BEGIN_TEST \
    BOOST_AUTO_TEST_SUITE( tests ) \
    BOOST_AUTO_TEST_CASE( test ) \
    {

#define END_TEST \
    } \
    BOOST_AUTO_TEST_SUITE_END()
