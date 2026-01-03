#include "serialization/array.h"
#include "serialization/deque.h"
#include "serialization/map.h"
#include "serialization/serialization.h"
#include "serialization/unordered_map.h"
#include "serialization/unordered_set.h"
#include "serialization/vector.h"
#include "serialization/yamldocument.h"

#include <array>
#include <boost/test/unit_test.hpp>
#include <deque>
#include <gsl-lite/gsl-lite.hpp>
#include <map>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace serialization::tests
{
struct ContainerContext
{
};

BOOST_AUTO_TEST_SUITE(container_serialization_tests)

BOOST_AUTO_TEST_CASE(test_vector_roundtrip)
{
  std::vector data = {1, 2, 3};
  const std::string yaml = "config:\n  data: !<vector> [1, 2, 3]";
  YAMLDocument<true> doc{yaml};
  std::vector<int> result;
  ContainerContext ctx;
  struct Cfg
  {
    std::vector<int>& ref;
    void deserialize(const Deserializer<ContainerContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL_COLLECTIONS(result.begin(), result.end(), data.begin(), data.end());
}

BOOST_AUTO_TEST_CASE(test_deque_roundtrip)
{
  std::deque<std::string> data = {"a", "b"};
  const std::string yaml = "config:\n  data: [a, b]";
  YAMLDocument<true> doc{yaml};
  std::deque<std::string> result;
  ContainerContext ctx;
  struct Cfg
  {
    std::deque<std::string>& ref;
    void deserialize(const Deserializer<ContainerContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.size(), 2);
  BOOST_CHECK_EQUAL(result[0], "a");
}

BOOST_AUTO_TEST_CASE(test_array_roundtrip)
{
  const std::string yaml = "config:\n  data: [10, 20]";
  YAMLDocument<true> doc{yaml};
  std::array<int, 2> result;
  ContainerContext ctx;
  struct Cfg
  {
    std::array<int, 2>& ref;
    void deserialize(const Deserializer<ContainerContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result[0], 10);
  BOOST_CHECK_EQUAL(result[1], 20);
}

BOOST_AUTO_TEST_CASE(test_map_roundtrip)
{
  const std::string yaml = "config:\n  data: !<map> [{key: key, value: 42}]";
  YAMLDocument<true> doc{yaml};
  std::map<std::string, int> result;
  ContainerContext ctx;
  struct Cfg
  {
    std::map<std::string, int>& ref;
    void deserialize(const Deserializer<ContainerContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result["key"], 42);
}

BOOST_AUTO_TEST_CASE(test_unordered_map_roundtrip)
{
  std::unordered_map<int, std::string> data = {{1, "one"}};
  const std::string yaml = "config:\n  data: !<unordered_map> [{key: 1, value: one}]";
  YAMLDocument<true> doc{yaml};
  std::unordered_map<int, std::string> result;
  ContainerContext ctx;
  struct Cfg
  {
    std::unordered_map<int, std::string>& ref;
    void deserialize(const Deserializer<ContainerContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result[1], "one");
}

BOOST_AUTO_TEST_CASE(test_unordered_set_roundtrip)
{
  std::unordered_set data = {1, 2};
  const std::string yaml = "config:\n  data: [1, 2]";
  YAMLDocument<true> doc{yaml};
  std::unordered_set<int> result;
  ContainerContext ctx;
  struct Cfg
  {
    std::unordered_set<int>& ref;
    void deserialize(const Deserializer<ContainerContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.size(), 2);
  BOOST_CHECK(result.count(1));
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace serialization::tests
