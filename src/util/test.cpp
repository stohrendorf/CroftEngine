#define BOOST_TEST_MODULE util

#include "smallcollections.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(util_tests)

BOOST_AUTO_TEST_CASE(test_insert_unique)
{
  std::vector<int> values;
  util::insertUnique(values, 123);
  BOOST_REQUIRE_EQUAL(values.size(), 1);
  BOOST_CHECK_EQUAL(values[0], 123);

  util::insertUnique(values, 123);
  BOOST_REQUIRE_EQUAL(values.size(), 1);
  BOOST_CHECK_EQUAL(values[0], 123);

  util::insertUnique(values, 456);
  BOOST_REQUIRE_EQUAL(values.size(), 2);
  BOOST_CHECK_EQUAL(values[0], 123);
  BOOST_CHECK_EQUAL(values[1], 456);
}

BOOST_AUTO_TEST_CASE(test_erase_unique)
{
  std::vector<int> values;
  util::eraseUnique(values, 123);
  BOOST_REQUIRE(values.empty());

  values = {123};
  util::eraseUnique(values, 123);
  BOOST_REQUIRE(values.empty());

  values = {123};
  util::eraseUnique(values, 999);
  BOOST_REQUIRE_EQUAL(values.size(), 1);
  BOOST_CHECK_EQUAL(values[0], 123);

  values = {123, 456, 789};
  util::eraseUnique(values, 456);
  BOOST_REQUIRE_EQUAL(values.size(), 2);
  BOOST_CHECK_EQUAL(values[0], 123);
  BOOST_CHECK_EQUAL(values[1], 789);
}

BOOST_AUTO_TEST_CASE(test_contains_unique)
{
  std::vector<int> values;
  BOOST_CHECK(!util::containsUnique(values, 123));

  values = {123};
  BOOST_CHECK(util::containsUnique(values, 123));
  BOOST_CHECK(!util::containsUnique(values, 456));
}

BOOST_AUTO_TEST_CASE(test_get_or_create)
{
  std::vector<std::pair<int, bool>> values;
  BOOST_CHECK_EQUAL(util::getOrCreate(values, 123), false);
  BOOST_REQUIRE_EQUAL(values.size(), 1);
  BOOST_REQUIRE_EQUAL(values[0].first, 123);
  BOOST_REQUIRE_EQUAL(values[0].second, false);

  util::getOrCreate(values, 123) = true;
  BOOST_REQUIRE_EQUAL(values.size(), 1);
  BOOST_REQUIRE_EQUAL(values[0].first, 123);
  BOOST_REQUIRE_EQUAL(values[0].second, true);

  BOOST_CHECK_EQUAL(util::getOrCreate(values, 456), false);
  BOOST_REQUIRE_EQUAL(values.size(), 2);
  BOOST_REQUIRE_EQUAL(values[0].first, 123);
  BOOST_REQUIRE_EQUAL(values[0].second, true);
  BOOST_REQUIRE_EQUAL(values[1].first, 456);
  BOOST_REQUIRE_EQUAL(values[1].second, false);

  util::getOrCreate(values, 456) = true;
  BOOST_REQUIRE_EQUAL(values.size(), 2);
  BOOST_REQUIRE_EQUAL(values[0].first, 123);
  BOOST_REQUIRE_EQUAL(values[0].second, true);
  BOOST_REQUIRE_EQUAL(values[1].first, 456);
  BOOST_REQUIRE_EQUAL(values[1].second, true);

  util::getOrCreate(values, 123) = false;
  BOOST_REQUIRE_EQUAL(values.size(), 2);
  BOOST_REQUIRE_EQUAL(values[0].first, 123);
  BOOST_REQUIRE_EQUAL(values[0].second, false);
  BOOST_REQUIRE_EQUAL(values[1].first, 456);
  BOOST_REQUIRE_EQUAL(values[1].second, true);
}

BOOST_AUTO_TEST_CASE(test_try_get)
{
  std::vector<std::pair<int, bool>> values;
  BOOST_CHECK(!util::tryGet(values, 123).has_value());
  BOOST_REQUIRE(values.empty());

  values = {{456, true}};
  BOOST_CHECK(!util::tryGet(values, 123).has_value());
  BOOST_CHECK(util::tryGet(values, 456).has_value());
  BOOST_CHECK_EQUAL(*util::tryGet(values, 456), true);
  BOOST_REQUIRE_EQUAL(values.size(), 1);
}

BOOST_AUTO_TEST_CASE(test_contains)
{
  std::vector<std::pair<int, bool>> values;
  BOOST_CHECK(!util::contains(values, 123));
  BOOST_REQUIRE(values.empty());

  values = {{456, true}};
  BOOST_CHECK(!util::contains(values, 123));
  BOOST_CHECK(util::contains(values, 456));
  BOOST_REQUIRE_EQUAL(values.size(), 1);
}
BOOST_AUTO_TEST_SUITE_END()
