#include "serialization/glm.h"
#include "serialization/serialization.h"
#include "serialization/yamldocument.h"

#include <boost/test/unit_test.hpp>
#include <glm/glm.hpp>
#include <gsl-lite/gsl-lite.hpp>

namespace serialization::tests
{
struct MathContext
{
};

BOOST_AUTO_TEST_SUITE(math_serialization_tests)

BOOST_AUTO_TEST_CASE(test_glm_vec3_roundtrip)
{
  const std::string yaml = "config:\n  data: !<vec> [1.0, 2.0, 3.0]";
  YAMLDocument<true> doc{yaml};
  glm::vec3 result;
  MathContext ctx;
  struct Cfg
  {
    glm::vec3& ref;
    void deserialize(const Deserializer<MathContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result.x, 1.0f);
  BOOST_CHECK_EQUAL(result.y, 2.0f);
  BOOST_CHECK_EQUAL(result.z, 3.0f);
}

BOOST_AUTO_TEST_CASE(test_glm_mat4_roundtrip)
{
  auto data = glm::mat4(1.0f);
  data[0][3] = 42.0f;

  // mat4 is serialized as a flat sequence of 16 floats: m[0][0], m[0][1], m[0][2], m[0][3], m[1][0], ...
  const std::string yaml = "config:\n  data: !<mat4> [1,0,0,42, 0,1,0,0, 0,0,1,0, 0,0,0,1]";

  YAMLDocument<true> doc{yaml};
  glm::mat4 result;
  MathContext ctx;
  struct Cfg
  {
    glm::mat4& ref;
    void deserialize(const Deserializer<MathContext>& ser)
    {
      ser("data", ref);
    }
  } cfg{result};
  doc.deserialize("config", gsl_lite::not_null{&ctx}, cfg);
  BOOST_CHECK_EQUAL(result[0][3], 42.0f);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace serialization::tests
