#define BOOST_TEST_MODULE core

#include "angle.h"

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(core_tests)

BOOST_AUTO_TEST_CASE(test_angle_axis)
{
  BOOST_CHECK(core::axisFromAngle(0_deg) == core::Axis::Deg0);
  BOOST_CHECK(core::axisFromAngle(40_deg) == core::Axis::Deg0);
  BOOST_CHECK(core::axisFromAngle(50_deg) == core::Axis::Right90);
  BOOST_CHECK(core::axisFromAngle(170_deg) == core::Axis::Deg180);
  BOOST_CHECK(core::axisFromAngle(-170_deg) == core::Axis::Deg180);
  BOOST_CHECK(core::axisFromAngle(-130_deg) == core::Axis::Left90);
  BOOST_CHECK(core::axisFromAngle(-40_deg) == core::Axis::Deg0);
}

BOOST_AUTO_TEST_CASE(test_angle_axis_with_margin)
{
  BOOST_CHECK(core::axisFromAngle(0_deg, 10_deg) == core::Axis::Deg0);
  BOOST_CHECK(core::axisFromAngle(40_deg, 10_deg) == std::nullopt);
  BOOST_CHECK(core::axisFromAngle(50_deg, 10_deg) == std::nullopt);
  BOOST_CHECK(core::axisFromAngle(170_deg, 10_deg) == std::nullopt);
  BOOST_CHECK(core::axisFromAngle(-175_deg, 10_deg) == core::Axis::Deg180);
  BOOST_CHECK(core::axisFromAngle(-130_deg, 10_deg) == std::nullopt);
  BOOST_CHECK(core::axisFromAngle(-40_deg, 10_deg) == std::nullopt);
}

BOOST_AUTO_TEST_SUITE_END()
