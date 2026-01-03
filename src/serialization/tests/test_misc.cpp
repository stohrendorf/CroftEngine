#include "serialization/bitset.h"
#include "serialization/chrono.h"
#include "serialization/named_enum.h"
#include "serialization/not_null.h"
#include "serialization/optional.h"
#include "serialization/pair.h"
#include "serialization/path.h"
#include "serialization/quantity.h"
#include "serialization/ratio.h"
#include "serialization/serialization.h"
#include "serialization/variant.h"
#include "serialization/vector_element.h"
#include "serialization/yamldocument.h"

#include <bitset>
#include <boost/test/unit_test.hpp>
#include <chrono>
#include <filesystem>
#include <gsl-lite/gsl-lite.hpp>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace serialization::tests
{
struct MiscContext
{
};

BOOST_AUTO_TEST_SUITE(misc_serialization_tests)

BOOST_AUTO_TEST_CASE(test_bitset_roundtrip)
{
  std::bitset<4> data;
  data.set(1);
  data.set(3);
  const std::string yaml = "config:\n  data: !<bitset> '0101'";
  YAMLDocument<true> doc{yaml};
  std::bitset<4> result;
  MiscContext ctx;
  struct Cfg
  {
    std::bitset<4>& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.test(1), true);
  BOOST_CHECK_EQUAL(result.test(3), true);
  BOOST_CHECK_EQUAL(result.test(0), false);
  BOOST_CHECK_EQUAL(result.test(2), false);
}

BOOST_AUTO_TEST_CASE(test_optional_roundtrip)
{
  const std::string yaml = "config:\n  data: 42";
  YAMLDocument<true> doc{yaml};
  std::optional<int> result;
  MiscContext ctx;
  struct Cfg
  {
    std::optional<int>& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_REQUIRE(result.has_value());
  BOOST_CHECK_EQUAL(*result, 42);
}

struct TestUnit
{
  static std::string suffix()
  {
    return "m";
  }
};

BOOST_AUTO_TEST_CASE(test_quantity_roundtrip)
{
  const std::string yaml = "config:\n  data: !<m> 100";
  YAMLDocument<true> doc{yaml};
  qs::quantity<TestUnit, int> result{0};
  MiscContext ctx;
  struct Cfg
  {
    qs::quantity<TestUnit, int>& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.get(), 100);
}

BOOST_AUTO_TEST_CASE(test_chrono_roundtrip)
{
  const std::string yaml = "config:\n  data: !<duration>\n    count: 60\n    period:\n      n: 1\n      d: 1";
  YAMLDocument<true> doc{yaml};
  std::chrono::seconds result{0};
  MiscContext ctx;
  struct Cfg
  {
    std::chrono::seconds& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.count(), 60);
}

BOOST_AUTO_TEST_CASE(test_pair_roundtrip)
{
  std::pair<int, std::string> data = {1, "two"};
  const std::string yaml = "config:\n  data: !<pair>\n    first: 1\n    second: two";
  YAMLDocument<true> doc{yaml};
  std::pair<int, std::string> result;
  MiscContext ctx;
  struct Cfg
  {
    std::pair<int, std::string>& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.first, 1);
  BOOST_CHECK_EQUAL(result.second, "two");
}

BOOST_AUTO_TEST_CASE(test_path_roundtrip)
{
  std::filesystem::path data = "foo/bar";
  const std::string yaml = "config:\n  data: !<path> foo/bar";
  YAMLDocument<true> doc{yaml};
  std::filesystem::path result;
  MiscContext ctx;
  struct Cfg
  {
    std::filesystem::path& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.string(), "foo/bar");
}

BOOST_AUTO_TEST_CASE(test_variant_roundtrip)
{
  std::variant<int, std::string> result;
  MiscContext ctx;

  auto test = [&](const std::string& yaml)
  {
    YAMLDocument<true> doc{yaml};
    struct Cfg
    {
      std::variant<int, std::string>& ref;
      void deserialize(const Deserializer<MiscContext>& ser)
      {
        ser("data", ref);
      }
    } cfg{result};
    doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  };

  test("config:\n  data: 42");
  BOOST_CHECK(std::holds_alternative<int>(result));
  BOOST_CHECK_EQUAL(std::get<int>(result), 42);

  test("config:\n  data: hello");
  BOOST_CHECK(std::holds_alternative<std::string>(result));
  BOOST_CHECK_EQUAL(std::get<std::string>(result), "hello");
}

enum class MyEnum
{
  A,
  B
};
inline std::string toString(const MyEnum e)
{
  return e == MyEnum::A ? "A" : "B";
}

struct MyEnumConverter
{
  static std::string name()
  {
    return "my_enum";
  }
  static MyEnum fromString(const std::string& s)
  {
    if(s == "A")
      return MyEnum::A;
    if(s == "B")
      return MyEnum::B;
    throw std::domain_error("invalid");
  }
};

BOOST_AUTO_TEST_CASE(test_named_enum_roundtrip)
{
  const std::string yaml = "config:\n  data: !<my_enum> A";
  YAMLDocument<true> doc{yaml};
  NamedEnum<MyEnum, MyEnumConverter> result;
  MiscContext ctx;
  struct Cfg
  {
    NamedEnum<MyEnum, MyEnumConverter>& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK(result.value == MyEnum::A);
}

BOOST_AUTO_TEST_CASE(test_vector_element_roundtrip)
{
  std::vector vec = {10, 20, 30};
  const int* element = &vec[1];

  MiscContext ctx;

  // Test serialization
  {
    YAMLDocument<false> doc{"unused.yaml"};
    struct Cfg
    {
      const std::vector<int>& vec;
      const int*& element;
      void serialize(const Serializer<MiscContext>& ser) const
      {
        ser("el", VectorElement{std::cref(vec), std::ref(element)});
      }
    } cfg{vec, element};
    doc.serialize("config", gsl_lite::not_null{&ctx}, cfg);
  }

  // Test deserialization
  {
    std::string yaml = "config:\n  el: !<element> 2";
    YAMLDocument<true> doc{yaml};
    const int* result = nullptr;
    struct Cfg
    {
      const std::vector<int>& vec;
      const int*& ref;
      void deserialize(const Deserializer<MiscContext>& ser)
      {
        ser("el", VectorElement{std::cref(vec), std::ref(ref)});
      }
    } cfg{vec, result};
    doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
    BOOST_CHECK_EQUAL(result, &vec[2]);
    BOOST_CHECK_EQUAL(*result, 30);
  }
}

BOOST_AUTO_TEST_CASE(test_ratio_roundtrip)
{
  const std::string yaml = "config:\n  data: !<ratio>\n    n: 1\n    d: 1000";
  YAMLDocument<true> doc{yaml};
  std::ratio<1, 1000> result;
  MiscContext ctx;
  struct Cfg
  {
    std::ratio<1, 1000>& ref;
    void deserialize(const Deserializer<MiscContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace serialization::tests
