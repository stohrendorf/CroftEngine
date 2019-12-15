#define BOOST_TEST_MODULE qs_test

#include "qs.h"

#include <boost/test/included/unit_test.hpp>

#define TPL(...) __VA_ARGS__
#define STR(X) #X

#define TEST_SAME(L, R) BOOST_CHECK_MESSAGE(TPL(std::is_same_v<L, R>), STR(TPL(L)) " == " STR(TPL(R)))

using namespace qs::detail;
using namespace qs;

BOOST_AUTO_TEST_SUITE(qs_type_tests)

BOOST_AUTO_TEST_CASE(test_drop_if_same_t)
{
  TEST_SAME(TPL(drop_if_same_t<int, int>), std::tuple<>);
  TEST_SAME(TPL(drop_if_same_t<int, short>), std::tuple<short>);
}

BOOST_AUTO_TEST_CASE(test_drop_one_t)
{
  TEST_SAME(TPL(drop_one_t<int, std::tuple<>>), std::tuple<>);
  TEST_SAME(TPL(drop_one_t<int, std::tuple<int>>), std::tuple<>);
  TEST_SAME(TPL(drop_one_t<int, std::tuple<int, int>>), std::tuple<int>);
  TEST_SAME(TPL(drop_one_t<int, std::tuple<short>>), std::tuple<short>);
  TEST_SAME(TPL(drop_one_t<int, std::tuple<short, int>>), std::tuple<short>);
}

BOOST_AUTO_TEST_CASE(test_first_t)
{
  TEST_SAME(TPL(first_t<std::tuple<int, short, char>>), int);
  TEST_SAME(TPL(first_t<std::tuple<short>>), short);
  TEST_SAME(TPL(first_t<std::tuple<short, int>>), short);
}

BOOST_AUTO_TEST_CASE(test_except_first_t)
{
  TEST_SAME(TPL(except_first_t<std::tuple<int, short, char>>), TPL(std::tuple<short, char>));
  TEST_SAME(except_first_t<std::tuple<short>>, std::tuple<>);
  TEST_SAME(TPL(except_first_t<std::tuple<short, int>>), std::tuple<int>);
}

BOOST_AUTO_TEST_CASE(test_drop_all_once_t)
{
  TEST_SAME(TPL(drop_all_once_t<std::tuple<int, short>, std::tuple<char>>), std::tuple<char>);
  TEST_SAME(TPL(drop_all_once_t<std::tuple<short>, std::tuple<short, int>>), std::tuple<int>);
  TEST_SAME(TPL(drop_all_once_t<std::tuple<short>, std::tuple<short, short, int>>), TPL(std::tuple<short, int>));
  TEST_SAME(TPL(drop_all_once_t<std::tuple<short>, std::tuple<>>), std::tuple<>);
}

BOOST_AUTO_TEST_CASE(test_symmetric_difference)
{
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<int, short>, std::tuple<char>>::reduced_l),
            TPL(std::tuple<int, short>));
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<int, short>, std::tuple<char>>::reduced_r), std::tuple<char>);
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<short>, std::tuple<short, int>>::reduced_l), std::tuple<>);
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<short>, std::tuple<short, int>>::reduced_r), std::tuple<int>);
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<short, short, int>, std::tuple<short, int>>::reduced_l),
            std::tuple<short>);
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<short, short, int>, std::tuple<short, int>>::reduced_r),
            std::tuple<>);
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<short>, std::tuple<>>::reduced_l), std::tuple<short>);
  TEST_SAME(TPL(typename symmetric_difference<std::tuple<short>, std::tuple<>>::reduced_r), std::tuple<>);
}

BOOST_AUTO_TEST_CASE(test_fraction_unit_t)
{
  TEST_SAME(TPL(fraction_unit_t<std::tuple<int, int, int, int>, std::tuple<short, int>>),
            TPL(fraction_unit<std::tuple<int, int, int>, std::tuple<short>>));

  TEST_SAME(TPL(fraction_unit_t<std::tuple<int, char>, std::tuple<char>>), int);

  TEST_SAME(TPL(fraction_unit_t<std::tuple<int, int, int>, std::tuple<int>>), TPL(product_unit<int, int>));

  TEST_SAME(TPL(fraction_unit_t<std::tuple<int, int>, std::tuple<int>>), int);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(qs_quantitative_tests)

BOOST_AUTO_TEST_CASE(test_comparison)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(Foo{2}, Foo{2});
  BOOST_CHECK_NE(Foo{2}, Foo{3});
  BOOST_CHECK_LE(Foo{2}, Foo{3});
  BOOST_CHECK_LE(Foo{3}, Foo{3});
  BOOST_CHECK_LT(Foo{2}, Foo{3});
  BOOST_CHECK_GE(Foo{3}, Foo{2});
  BOOST_CHECK_GE(Foo{3}, Foo{3});
  BOOST_CHECK_GT(Foo{3}, Foo{2});
}

BOOST_AUTO_TEST_CASE(test_mul)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(Foo{2} * 5, Foo{10});
  BOOST_CHECK_EQUAL(5 * Foo{2}, Foo{10});
}

BOOST_AUTO_TEST_CASE(test_div)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(Foo{10} / 2, Foo{5});
  BOOST_CHECK_EQUAL(Foo{10} / Foo{2}, 5);
}

BOOST_AUTO_TEST_CASE(test_mod)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(Foo{10} % Foo{2}, Foo{0});
  BOOST_CHECK_EQUAL(Foo{10} % Foo{3}, Foo{1});
}

BOOST_AUTO_TEST_CASE(test_add)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(Foo{10} + Foo{2}, Foo{12});
}

BOOST_AUTO_TEST_CASE(test_sub)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(Foo{10} - Foo{2}, Foo{8});
}

BOOST_AUTO_TEST_CASE(test_unary_plus)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(+Foo{2}, Foo{+2});
}

BOOST_AUTO_TEST_CASE(test_unary_minus)
{
  QS_DECLARE_QUANTITY(Foo, int, "foo");

  BOOST_CHECK_EQUAL(-Foo{2}, Foo{-2});
}

BOOST_AUTO_TEST_SUITE_END()
