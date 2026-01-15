#include "core/angle.h"
#include "core/units.h"
#include "core/vec.h"
#include "util/helpers.h"

#include <boost/test/unit_test.hpp>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace util::tests
{
BOOST_AUTO_TEST_SUITE(helpers_tests)

BOOST_AUTO_TEST_CASE(test_square)
{
  BOOST_CHECK_EQUAL(square(0), 0);
  BOOST_CHECK_EQUAL(square(1), 1);
  BOOST_CHECK_EQUAL(square(2), 4);
  BOOST_CHECK_EQUAL(square(-3), 9);
  BOOST_CHECK_EQUAL(square(5), 25);

  // Test with floats
  BOOST_CHECK_CLOSE(square(2.5f), 6.25f, 0.001f);
  BOOST_CHECK_CLOSE(square(-1.5f), 2.25f, 0.001f);

  // Test with custom types (Length)
  const auto len = 10_len;
  const auto area = square(len);
  BOOST_CHECK_EQUAL(area.get(), 100);
}

BOOST_AUTO_TEST_CASE(test_bits_extraction)
{
  const uint8_t value = 0b11010110;

  BOOST_CHECK_EQUAL(bits(value, 0, 2), 0b10);

  BOOST_CHECK_EQUAL(bits(value, 2, 2), 0b01);

  BOOST_CHECK_EQUAL(bits(value, 4, 2), 0b01);

  BOOST_CHECK_EQUAL(bits(value, 6, 2), 0b11);

  BOOST_CHECK_EQUAL(bits(value, 0, 8), value);

  const uint16_t value16 = 0b1101011010101010;
  BOOST_CHECK_EQUAL(bits(value16, 0, 4), 0b1010);
  BOOST_CHECK_EQUAL(bits(value16, 8, 4), 0b0110);
  BOOST_CHECK_EQUAL(bits(value16, 12, 4), 0b1101);

  const uint32_t value32 = 0xDEADBEEF;
  BOOST_CHECK_EQUAL(bits(value32, 0, 8), 0xEF);
  BOOST_CHECK_EQUAL(bits(value32, 8, 8), 0xBE);
  BOOST_CHECK_EQUAL(bits(value32, 16, 8), 0xAD);
  BOOST_CHECK_EQUAL(bits(value32, 24, 8), 0xDE);
}

BOOST_AUTO_TEST_CASE(test_trigonometry_length)
{
  const auto len = 1000_len;

  BOOST_CHECK_EQUAL(sin(len, 0_deg), 0_len);

  const auto sin90 = sin(len, 90_deg);
  BOOST_CHECK_CLOSE(sin90.get<float>(), len.get<float>(), 0.1f);

  const auto sin180 = sin(len, 180_deg);
  BOOST_CHECK_SMALL(sin180.get<float>(), 1.0f);

  const auto sinNeg90 = sin(len, -90_deg);
  BOOST_CHECK_CLOSE(sinNeg90.get<float>(), -len.get<float>(), 0.1f);

  const auto cos0 = cos(len, 0_deg);
  BOOST_CHECK_CLOSE(cos0.get<float>(), len.get<float>(), 0.1f);

  const auto cos90 = cos(len, 90_deg);
  BOOST_CHECK_SMALL(cos90.get<float>(), 1.0f);

  const auto cos180 = cos(len, 180_deg);
  BOOST_CHECK_CLOSE(cos180.get<float>(), -len.get<float>(), 0.1f);
}

BOOST_AUTO_TEST_CASE(test_pitch_from_length)
{
  const auto len = 1000_len;

  const auto pitch0 = pitch(len, 0_deg);
  BOOST_CHECK_SMALL(pitch0.X.get<float>(), 1.0f);
  BOOST_CHECK_EQUAL(pitch0.Y, 0_len);
  BOOST_CHECK_CLOSE(pitch0.Z.get<float>(), len.get<float>(), 0.1f);

  const auto pitch90 = pitch(len, 90_deg);
  BOOST_CHECK_CLOSE(pitch90.X.get<float>(), len.get<float>(), 0.1f);
  BOOST_CHECK_EQUAL(pitch90.Y, 0_len);
  BOOST_CHECK_SMALL(pitch90.Z.get<float>(), 1.0f);

  const auto pitchNeg90 = pitch(len, -90_deg);
  BOOST_CHECK_CLOSE(pitchNeg90.X.get<float>(), -len.get<float>(), 0.1f);
  BOOST_CHECK_EQUAL(pitchNeg90.Y, 0_len);
  BOOST_CHECK_SMALL(pitchNeg90.Z.get<float>(), 1.0f);

  const auto dy = 500_len;
  const auto pitchWithDy = pitch(len, 45_deg, dy);
  BOOST_CHECK_EQUAL(pitchWithDy.Y, dy);
}

BOOST_AUTO_TEST_CASE(test_pitch_from_vector)
{
  const core::TRVec vec{0_len, 0_len, 1000_len};

  const auto rotated0 = pitch(vec, 0_deg);
  BOOST_CHECK_SMALL(rotated0.X.get<float>(), 1.0f);
  BOOST_CHECK_EQUAL(rotated0.Y, 0_len);
  BOOST_CHECK_CLOSE(rotated0.Z.get<float>(), vec.Z.get<float>(), 0.1f);

  const auto rotated90 = pitch(vec, 90_deg);
  BOOST_CHECK_CLOSE(rotated90.X.get<float>(), vec.Z.get<float>(), 0.1f);
  BOOST_CHECK_EQUAL(rotated90.Y, 0_len);
  BOOST_CHECK_SMALL(rotated90.Z.get<float>(), 1.0f);

  const core::TRVec vec2{500_len, 100_len, 866_len};
  const auto rotated45 = pitch(vec2, 45_deg);
  BOOST_CHECK_EQUAL(rotated45.Y, vec2.Y);

  const auto originalLen
    = std::sqrt(vec2.X.get<float>() * vec2.X.get<float>() + vec2.Z.get<float>() * vec2.Z.get<float>());
  const auto rotatedLen = std::sqrt(rotated45.X.get<float>() * rotated45.X.get<float>()
                                    + rotated45.Z.get<float>() * rotated45.Z.get<float>());
  BOOST_CHECK_CLOSE(originalLen, rotatedLen, 1.0f); // Allow 1% tolerance for integer math
}

BOOST_AUTO_TEST_CASE(test_yaw_pitch)
{
  const auto len = 1000_len;

  const core::TRRotation rot0{0_deg, 0_deg, 0_deg};
  const auto result0 = yawPitch(len, rot0);
  BOOST_CHECK_SMALL(result0.X.get<float>(), 1.0f);
  BOOST_CHECK_SMALL(result0.Y.get<float>(), 1.0f);
  BOOST_CHECK_CLOSE(result0.Z.get<float>(), len.get<float>(), 0.1f);

  const core::TRRotation rot90Y{0_deg, 90_deg, 0_deg};
  const auto result90Y = yawPitch(len, rot90Y);
  BOOST_CHECK_CLOSE(result90Y.X.get<float>(), len.get<float>(), 0.1f);
  BOOST_CHECK_SMALL(result90Y.Y.get<float>(), 1.0f);
  BOOST_CHECK_SMALL(result90Y.Z.get<float>(), 1.0f);

  const core::TRRotation rot90X{90_deg, 0_deg, 0_deg};
  const auto result90X = yawPitch(len, rot90X);
  BOOST_CHECK_SMALL(result90X.X.get<float>(), 1.0f);
  BOOST_CHECK_CLOSE(result90X.Y.get<float>(), -len.get<float>(), 0.1f);
  BOOST_CHECK_SMALL(result90X.Z.get<float>(), 1.0f);
}

BOOST_AUTO_TEST_CASE(test_matrix_lerp)
{
  const glm::mat4 a{1.0f};
  const glm::mat4 b{1.0f};

  auto result = lerp(a, b, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      BOOST_CHECK_EQUAL(result[i][j], (i == j ? 1.0f : 0.0f));

  result = lerp(a, b, 0.5f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      BOOST_CHECK_EQUAL(result[i][j], (i == j ? 1.0f : 0.0f));

  result = lerp(a, b, 1.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      BOOST_CHECK_EQUAL(result[i][j], (i == j ? 1.0f : 0.0f));

  glm::mat4 c{1.0f};
  c[0][0] = 2.0f;
  c[1][1] = 3.0f;
  c[2][2] = 4.0f;
  c[3][3] = 5.0f;

  glm::mat4 d{1.0f};
  d[0][0] = 4.0f;
  d[1][1] = 6.0f;
  d[2][2] = 8.0f;
  d[3][3] = 10.0f;

  result = lerp(c, d, 0.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      BOOST_CHECK_CLOSE(result[i][j], c[i][j], 0.001f);

  result = lerp(c, d, 1.0f);
  for(int i = 0; i < 4; ++i)
    for(int j = 0; j < 4; ++j)
      BOOST_CHECK_CLOSE(result[i][j], d[i][j], 0.001f);

  result = lerp(c, d, 0.5f);
  BOOST_CHECK_CLOSE(result[0][0], 3.0f, 0.001f);
  BOOST_CHECK_CLOSE(result[1][1], 4.5f, 0.001f);
  BOOST_CHECK_CLOSE(result[2][2], 6.0f, 0.001f);
  BOOST_CHECK_CLOSE(result[3][3], 7.5f, 0.001f);
}

BOOST_AUTO_TEST_CASE(test_rand15_bounds)
{
  for(int i = 0; i < 100; ++i)
  {
    const auto value = rand15();
    BOOST_CHECK_GE(value, 0);
    BOOST_CHECK_LT(value, Rand15Max);
  }
}

BOOST_AUTO_TEST_CASE(test_rand15s_bounds)
{
  for(int i = 0; i < 100; ++i)
  {
    const auto value = rand15s();
    BOOST_CHECK_GE(value, -Rand15Max / 2);
    BOOST_CHECK_LT(value, Rand15Max / 2);
  }
}

BOOST_AUTO_TEST_CASE(test_rand15_with_max)
{
  const int max = 100;
  for(int i = 0; i < 100; ++i)
  {
    const auto value = rand15(max);
    BOOST_CHECK_GE(value, 0);
    BOOST_CHECK_LT(value, max);
  }

  const float maxFloat = 50.5f;
  for(int i = 0; i < 100; ++i)
  {
    const auto value = rand15(maxFloat);
    BOOST_CHECK_GE(value, 0.0f);
    BOOST_CHECK_LT(value, maxFloat);
  }
}

BOOST_AUTO_TEST_CASE(test_rand15s_with_max)
{
  const int max = 100;
  for(int i = 0; i < 100; ++i)
  {
    const auto value = rand15s(max);
    BOOST_CHECK_GE(value, -max / 2);
    BOOST_CHECK_LE(value, max / 2);
  }
}

BOOST_AUTO_TEST_CASE(test_escape_unescape_roundtrip)
{
  const std::string ascii = "Hello World 123";
  BOOST_CHECK_EQUAL(unescape(escape(ascii)), ascii);

  BOOST_CHECK_EQUAL(unescape(escape("ä")), "ä");
  BOOST_CHECK_EQUAL(unescape(escape("ö")), "ö");
  BOOST_CHECK_EQUAL(unescape(escape("ü")), "ü");
  BOOST_CHECK_EQUAL(unescape(escape("Ä")), "Ä");
  BOOST_CHECK_EQUAL(unescape(escape("Ö")), "Ö");
  BOOST_CHECK_EQUAL(unescape(escape("Ü")), "Ü");
  BOOST_CHECK_EQUAL(unescape(escape("ß")), "ß");

  const std::string german = "Schön Überraschung Ärger größer Müßiggang";
  BOOST_CHECK_EQUAL(unescape(escape(german)), german);

  const std::string mixed = "Hello Wörld! This is a täst. Größe matters.";
  BOOST_CHECK_EQUAL(unescape(escape(mixed)), mixed);
}

BOOST_AUTO_TEST_CASE(test_escape_latin1)
{
  std::string latin1_a_umlaut;
  latin1_a_umlaut += static_cast<char>(0xE4);

  const auto escaped = escape(latin1_a_umlaut);
  BOOST_CHECK_NE(escaped, latin1_a_umlaut);
  BOOST_CHECK_EQUAL(escaped.size(), 2u);

  const auto unescaped = unescape(escaped);
  BOOST_CHECK_EQUAL(unescaped, "ä");
}

BOOST_AUTO_TEST_CASE(test_time_str_formatting)
{
  using core::Seconds;

  BOOST_CHECK_EQUAL(toTimeStr(Seconds{0}), "00:00");
  BOOST_CHECK_EQUAL(toTimeStr(Seconds{30}), "00:30");
  BOOST_CHECK_EQUAL(toTimeStr(Seconds{59}), "00:59");

  BOOST_CHECK_EQUAL(toTimeStr(Seconds{60}), "01:00");

  BOOST_CHECK_EQUAL(toTimeStr(Seconds{90}), "01:30");
  BOOST_CHECK_EQUAL(toTimeStr(Seconds{125}), "02:05");
  BOOST_CHECK_EQUAL(toTimeStr(Seconds{3599}), "59:59");

  BOOST_CHECK_EQUAL(toTimeStr(Seconds{3600}), "1:00:00");

  BOOST_CHECK_EQUAL(toTimeStr(Seconds{3661}), "1:01:01");
  BOOST_CHECK_EQUAL(toTimeStr(Seconds{7325}), "2:02:05");
  BOOST_CHECK_EQUAL(toTimeStr(Seconds{36000}), "10:00:00");
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace util::tests
