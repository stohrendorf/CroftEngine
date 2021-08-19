#define BOOST_TEST_MODULE core

#include "angle.h"
#include "boundingbox.h"

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

BOOST_AUTO_TEST_CASE(test_bounding_box_validity)
{
  core::BoundingBox bbox{{1_len, 2_len, 3_len}, {-4_len, -5_len, 6_len}};
  BOOST_CHECK(!bbox.isValid());
  bbox.makeValid();
  BOOST_CHECK((bbox.min == core::TRVec{-4_len, -5_len, 3_len}));
  BOOST_CHECK((bbox.max == core::TRVec{1_len, 2_len, 6_len}));
}

BOOST_AUTO_TEST_CASE(test_bounding_box_contains)
{
  core::BoundingBox bbox{{1_len, 2_len, 3_len}, {4_len, 5_len, 6_len}};
  BOOST_CHECK(bbox.isValid());
  BOOST_CHECK((!bbox.contains({1_len, 2_len, 2_len})));
  BOOST_CHECK((bbox.contains({1_len, 2_len, 3_len})));
  BOOST_CHECK((bbox.contains({1_len, 2_len, 6_len})));
  BOOST_CHECK((!bbox.contains({1_len, 2_len, 7_len})));
}

BOOST_AUTO_TEST_CASE(test_bounding_box_intersects)
{
  core::BoundingBox a{{0_len, 1_len, 2_len}, {3_len, 4_len, 5_len}};
  core::BoundingBox b{{3_len, 10_len, 10_len}, {20_len, 20_len, 20_len}};
  BOOST_CHECK(!a.intersects(b));
  core::BoundingBox c{{2_len, 10_len, 10_len}, {20_len, 20_len, 20_len}};
  BOOST_CHECK(!a.intersects(c));
  core::BoundingBox d{{2_len, 1_len, 3_len}, {2_len, 1_len, 3_len}};
  BOOST_CHECK(!a.intersects(d));
  core::BoundingBox e{{2_len, 1_len, 3_len}, {2_len, 2_len, 4_len}};
  BOOST_CHECK(a.intersects(e));
  core::BoundingBox f{{1_len, 2_len, 3_len}, {1_len, 2_len, 3_len}};
  BOOST_CHECK(!f.intersects(f)); //-V678
}

BOOST_AUTO_TEST_SUITE_END()
