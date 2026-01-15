#include "serialization/default.h"
#include "serialization/optional_value.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"

#include <boost/test/unit_test.hpp>
#include <gsl-lite/gsl-lite.hpp>
#include <string>

namespace serialization::tests
{
struct TestContext
{
};

struct BasicConfig
{
  std::string socket;
  int port = 0;
  bool online = false;
  enum class Type
  {
    A,
    B
  } type = Type::A;

  void serialize(const Serializer<TestContext>& ser) const
  {
    ser("socket", socket, "port", port, "online", online, "type", type);
  }

  void deserialize(const Deserializer<TestContext>& ser)
  {
    ser("socket", socket, "port", port, "online", online, "type", type);
  }
};

BOOST_AUTO_TEST_SUITE(basic_serialization_tests)

BOOST_AUTO_TEST_CASE(test_basic_roundtrip)
{
  BasicConfig cfg{.socket = "localhost", .port = 8080, .online = true, .type = BasicConfig::Type::B};
  const std::string yaml = "config:\n  socket: localhost\n  port: 8080\n  online: true\n  type: 1";
  YAMLDocument<true> doc{yaml};
  BasicConfig result;
  TestContext ctx;
  doc.deserialize("config", gsl_lite::not_null{&ctx}, result);

  BOOST_CHECK_EQUAL(result.socket, "localhost");
  BOOST_CHECK_EQUAL(result.port, 8080);
  BOOST_CHECK_EQUAL(result.online, true);
  BOOST_CHECK(result.type == BasicConfig::Type::B);
}

BOOST_AUTO_TEST_CASE(test_string_empty_null_missing)
{
  TestContext ctx;
  auto test_deserialization = [&](const std::string& yaml, const std::string& expected)
  {
    YAMLDocument<true> doc{yaml};
    std::string val = "pre-filled";
    struct SingleConfig
    {
      std::string& ref;
      void deserialize(const Deserializer<TestContext>& ser)
      {
        ser("val", ref);
      }
    } cfg{val};
    doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
    BOOST_CHECK_EQUAL(val, expected);
  };

  test_deserialization("config:\n  val: ''", "");
  test_deserialization("config:\n  val: ~", "~");
  test_deserialization("config:\n  val: null", "null");
  test_deserialization("config:\n  val: !!null ~", "");
}

BOOST_AUTO_TEST_CASE(test_is_null_logic)
{
  TestContext ctx;
  auto check_is_null = [&](const std::string& yaml, const bool expected)
  {
    const std::string full_yaml = "config:\n  val: " + yaml;
    YAMLDocument<true> doc{full_yaml};
    bool result = false;
    struct Probe
    {
      bool& res;
      void deserialize(const Deserializer<TestContext>& ser)
      {
        res = ser["val"].isNull();
      }
    } probe{result};
    doc.deserialize("config", gsl_lite::not_null{&ctx}, probe);
    BOOST_CHECK_MESSAGE(result == expected,
                        "YAML val: \"" << yaml << "\", Expected isNull: " << expected << ", Got: " << result);
  };

  check_is_null("~", false);
  check_is_null("null", false);
  check_is_null("!!null ~", true);
  check_is_null("", false);
  check_is_null("''", false);
  check_is_null("\"\"", false);
  check_is_null("{}", false);
  check_is_null("[]", false);
  check_is_null("foo", false);
}

BOOST_AUTO_TEST_CASE(test_is_null_missing_key)
{
  TestContext ctx;
  YAMLDocument<true> doc{std::string{"config: {}"}};
  bool result = true;
  struct Probe
  {
    bool& res;
    void deserialize(const Deserializer<TestContext>& ser)
    {
      res = ser["val"].isNull();
    }
  } probe{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, probe);
  BOOST_CHECK_EQUAL(result, false); // Missing key is NOT null
}

BOOST_AUTO_TEST_CASE(test_optional_value_missing)
{
  TestContext ctx;
  const std::string yaml = "config: {}";
  YAMLDocument<true> doc{yaml};

  bool online = true;
  struct OptConfig
  {
    bool& ref;
    void deserialize(const Deserializer<TestContext>& ser)
    {
      ser(S_NVO("online", std::ref(ref)));
    }
  } cfg{online};

  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(online, true); // Should remain true as it's missing in YAML
}

BOOST_AUTO_TEST_CASE(test_default_value)
{
  TestContext ctx;
  const std::string yaml = "config: {}";
  YAMLDocument<true> doc{yaml};

  int port = 123;
  struct DefConfig
  {
    int& ref;
    void deserialize(const Deserializer<TestContext>& ser)
    {
      ser(S_NVD("port", std::ref(ref), 8080));
    }
  } cfg{port};

  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(port, 8080); // Should be set to default
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace serialization::tests
