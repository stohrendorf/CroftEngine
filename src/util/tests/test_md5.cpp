#include "util/md5.h"

#include <boost/test/unit_test.hpp>
#include <string>
#include <vector>

namespace util::tests
{
BOOST_AUTO_TEST_SUITE(md5_tests)

BOOST_AUTO_TEST_CASE(test_md5_empty_string)
{
  const std::string empty;
  const auto hash = md5(empty.data(), empty.size());
  BOOST_CHECK_EQUAL(hash, "D41D8CD98F00B204E9800998ECF8427E");
}

BOOST_AUTO_TEST_CASE(test_md5_single_char)
{
  const std::string input = "a";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "0CC175B9C0F1B6A831C399E269772661");
}

BOOST_AUTO_TEST_CASE(test_md5_abc)
{
  const std::string input = "abc";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "900150983CD24FB0D6963F7D28E17F72");
}

BOOST_AUTO_TEST_CASE(test_md5_message_digest)
{
  const std::string input = "message digest";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "F96B697D7CB7938D525A2F31AAF161D0");
}

BOOST_AUTO_TEST_CASE(test_md5_alphabet_lowercase)
{
  const std::string input = "abcdefghijklmnopqrstuvwxyz";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "C3FCD3D76192E4007DFB496CCA67E13B");
}

BOOST_AUTO_TEST_CASE(test_md5_alphanumeric)
{
  const std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "D174AB98D277D9F5A5611C2C9F419D9F");
}

BOOST_AUTO_TEST_CASE(test_md5_long_sequence)
{
  const std::string input = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "57EDF4A22BE3C955AC49DA2E2107B67A");
}

BOOST_AUTO_TEST_CASE(test_md5_binary_data)
{
  std::vector<uint8_t> zeros(64, 0);
  const auto hash = md5(zeros.data(), zeros.size());
  BOOST_CHECK_EQUAL(hash, "3B5D3C7D207E37DCEEEDD301E35E2E58");
}

BOOST_AUTO_TEST_CASE(test_md5_single_byte_values)
{
  const uint8_t data[] = {0xFF};
  const auto hash = md5(data, 1);
  BOOST_CHECK_EQUAL(hash, "00594FD4F42BA43FC1CA0427A0576295");
}

BOOST_AUTO_TEST_CASE(test_md5_deterministic)
{
  const std::string input = "test deterministic behavior";

  const auto hash1 = md5(input.data(), input.size());
  const auto hash2 = md5(input.data(), input.size());
  const auto hash3 = md5(input.data(), input.size());

  BOOST_CHECK_EQUAL(hash1, hash2);
  BOOST_CHECK_EQUAL(hash2, hash3);
}

BOOST_AUTO_TEST_CASE(test_md5_sensitivity)
{
  const std::string input1 = "hello world";
  const std::string input2 = "hello worle"; // Changed last char

  const auto hash1 = md5(input1.data(), input1.size());
  const auto hash2 = md5(input2.data(), input2.size());

  BOOST_CHECK_NE(hash1, hash2);

  BOOST_CHECK_EQUAL(hash1, "5EB63BBBE01EEED093CB22BB8F5ACDC3");
}

BOOST_AUTO_TEST_CASE(test_md5_quick_brown_fox)
{
  const std::string input = "The quick brown fox jumps over the lazy dog";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "9E107D9D372BB6826BD81D3542A419D6");
}

BOOST_AUTO_TEST_CASE(test_md5_quick_brown_fox_period)
{
  const std::string input = "The quick brown fox jumps over the lazy dog.";
  const auto hash = md5(input.data(), input.size());
  BOOST_CHECK_EQUAL(hash, "E4D909C290D0FB1CA068FFADDF22CBD0");
}

BOOST_AUTO_TEST_CASE(test_md5_varying_lengths)
{
  std::string input55(55, 'a');
  std::string input56(56, 'a');
  std::string input63(63, 'a');
  std::string input64(64, 'a');
  std::string input65(65, 'a');
  std::string input127(127, 'a');
  std::string input128(128, 'a');
  std::string input129(129, 'a');

  const auto hash55 = md5(input55.data(), input55.size());
  const auto hash56 = md5(input56.data(), input56.size());
  const auto hash63 = md5(input63.data(), input63.size());
  const auto hash64 = md5(input64.data(), input64.size());
  const auto hash65 = md5(input65.data(), input65.size());
  const auto hash127 = md5(input127.data(), input127.size());
  const auto hash128 = md5(input128.data(), input128.size());
  const auto hash129 = md5(input129.data(), input129.size());

  BOOST_CHECK_NE(hash55, hash56);
  BOOST_CHECK_NE(hash56, hash63);
  BOOST_CHECK_NE(hash63, hash64);
  BOOST_CHECK_NE(hash64, hash65);
  BOOST_CHECK_NE(hash127, hash128);
  BOOST_CHECK_NE(hash128, hash129);

  BOOST_CHECK_NE(hash55, hash127);
}

BOOST_AUTO_TEST_CASE(test_md5_uint8_overload)
{
  const uint8_t data[] = {0x61, 0x62, 0x63}; // "abc" in ASCII
  const auto hash = md5(data, 3);
  BOOST_CHECK_EQUAL(hash, "900150983CD24FB0D6963F7D28E17F72");
}

BOOST_AUTO_TEST_CASE(test_md5_char_overload)
{
  const char data[] = "abc";
  const auto hash = md5(data, 3);
  BOOST_CHECK_EQUAL(hash, "900150983CD24FB0D6963F7D28E17F72");
}

BOOST_AUTO_TEST_CASE(test_md5_collision_resistance)
{
  std::vector<std::string> inputs = {"test1", "test2", "test3", "different", "another", "unique"};

  std::vector<std::string> hashes;
  for(const auto& input : inputs)
  {
    hashes.push_back(md5(input.data(), input.size()));
  }

  for(size_t i = 0; i < hashes.size(); ++i)
  {
    for(size_t j = i + 1; j < hashes.size(); ++j)
    {
      BOOST_CHECK_NE(hashes[i], hashes[j]);
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace util::tests
