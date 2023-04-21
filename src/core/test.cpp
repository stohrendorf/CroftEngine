#define BOOST_TEST_MODULE core

#include "angle.h"
#include "boundingbox.h"

#include <boost/test/unit_test.hpp>

namespace core
{
inline std::ostream& operator<<(std::ostream& s, const Axis& axis)
{
  return s << "Axis(" << static_cast<int>(axis) << ")";
}

inline std::ostream& operator<<(std::ostream& s, const std::optional<Axis>& axis)
{
  if(!axis.has_value())
    return s << "Axis(nullopt)";
  return s << *axis;
}
} // namespace core

BOOST_AUTO_TEST_SUITE(core_tests)

BOOST_AUTO_TEST_CASE(test_angle_axis)
{
  BOOST_CHECK_EQUAL(core::axisFromAngle(0_deg), core::Axis::Deg0);
  BOOST_CHECK_EQUAL(core::axisFromAngle(40_deg), core::Axis::Deg0);
  BOOST_CHECK_EQUAL(core::axisFromAngle(50_deg), core::Axis::Right90);
  BOOST_CHECK_EQUAL(core::axisFromAngle(170_deg), core::Axis::Deg180);
  BOOST_CHECK_EQUAL(core::axisFromAngle(-170_deg), core::Axis::Deg180);
  BOOST_CHECK_EQUAL(core::axisFromAngle(-130_deg), core::Axis::Left90);
  BOOST_CHECK_EQUAL(core::axisFromAngle(-40_deg), core::Axis::Deg0);
}

BOOST_AUTO_TEST_CASE(test_angle_axis_with_margin)
{
  BOOST_CHECK_EQUAL(core::axisFromAngle(0_deg, 10_deg), core::Axis::Deg0);
  BOOST_CHECK(!core::axisFromAngle(40_deg, 10_deg).has_value());
  BOOST_CHECK(!core::axisFromAngle(50_deg, 10_deg).has_value());
  BOOST_CHECK(!core::axisFromAngle(170_deg, 10_deg).has_value());
  BOOST_CHECK_EQUAL(core::axisFromAngle(-175_deg, 10_deg), core::Axis::Deg180);
  BOOST_CHECK(!core::axisFromAngle(-130_deg, 10_deg).has_value());
  BOOST_CHECK(!core::axisFromAngle(-40_deg, 10_deg).has_value());
}

BOOST_AUTO_TEST_CASE(test_interval_construction)
{
  core::Interval<int> i1{};
  BOOST_CHECK_EQUAL(i1.min, 0);
  BOOST_CHECK_EQUAL(i1.max, 0);

  core::Interval<int> i2{3, 1};
  BOOST_CHECK(!i2.isValid());
  BOOST_CHECK_EQUAL(i2.min, 3);
  BOOST_CHECK_EQUAL(i2.max, 1);
}

BOOST_AUTO_TEST_CASE(test_interval_validity)
{
  core::Interval<int> i{3, 1};
  BOOST_CHECK(!i.isValid());
  BOOST_CHECK_EQUAL(i.min, 3);
  BOOST_CHECK_EQUAL(i.max, 1);

  auto i2 = i.sanitized();
  BOOST_CHECK(i2.isValid());
  BOOST_CHECK_EQUAL(i2.min, 1);
  BOOST_CHECK_EQUAL(i2.max, 3);
}

BOOST_AUTO_TEST_CASE(test_interval_checks)
{
  core::Interval<int> i{1, 3};
  BOOST_CHECK(!i.contains(0));
  BOOST_CHECK(i.contains(1));
  BOOST_CHECK(i.contains(2));
  BOOST_CHECK(i.contains(3));
  BOOST_CHECK(!i.contains(4));

  BOOST_CHECK(!i.containsExclusive(0));
  BOOST_CHECK(!i.containsExclusive(1));
  BOOST_CHECK(i.containsExclusive(2));
  BOOST_CHECK(!i.containsExclusive(3));
  BOOST_CHECK(!i.containsExclusive(4));

  BOOST_CHECK(i.intersects(i));
  BOOST_CHECK(i.intersectsExclusive(i));

  core::Interval<int> i2{2, 5};
  BOOST_CHECK(i.intersects(i2));
  BOOST_CHECK(i.intersectsExclusive(i2));

  core::Interval<int> i3{3, 5};
  BOOST_CHECK(i.intersects(i3));
  BOOST_CHECK(!i.intersectsExclusive(i3));
}

BOOST_AUTO_TEST_CASE(test_interval_queries)
{
  const core::Interval<int> i{0, 3};
  BOOST_CHECK_EQUAL(i.size(), 3);
  BOOST_CHECK_EQUAL(i.mid(), 1);
  BOOST_CHECK_EQUAL(i.clamp(-1), 0);
  BOOST_CHECK_EQUAL(i.clamp(1), 1);
  BOOST_CHECK_EQUAL(i.clamp(5), 3);

  auto i2 = i.narrowed(1);
  BOOST_CHECK_EQUAL(i2.min, 1);
  BOOST_CHECK_EQUAL(i2.max, 2);

  auto i3 = i.broadened(1);
  BOOST_CHECK_EQUAL(i3.min, -1);
  BOOST_CHECK_EQUAL(i3.max, 4);

  const core::Interval<int> i4{2, 5};
  auto i5 = i.intersect(i4);
  BOOST_CHECK_EQUAL(i5.min, 2);
  BOOST_CHECK_EQUAL(i5.max, 3);

  auto i6 = i4.intersect(i);
  BOOST_CHECK_EQUAL(i6.min, 2);
  BOOST_CHECK_EQUAL(i6.max, 3);
}

BOOST_AUTO_TEST_CASE(test_interval_ops)
{
  const core::Interval<int> i{0, 3};
  auto i2 = i + 5;
  BOOST_CHECK_EQUAL(i2.min, 5);
  BOOST_CHECK_EQUAL(i2.max, 8);

  auto i3 = i;
  i3 += 10;
  BOOST_CHECK_EQUAL(i3.min, 10);
  BOOST_CHECK_EQUAL(i3.max, 13);

  auto i4 = 20 + i;
  BOOST_CHECK_EQUAL(i4.min, 20);
  BOOST_CHECK_EQUAL(i4.max, 23);
}

BOOST_AUTO_TEST_CASE(test_bounding_box_validity)
{
  core::BoundingBox bbox{{1_len, 2_len, 3_len}, {-4_len, -5_len, 6_len}};
  BOOST_CHECK(!bbox.x.isValid());
  BOOST_CHECK(!bbox.y.isValid());
  BOOST_CHECK(bbox.z.isValid());
  bbox.sanitize();
  BOOST_CHECK_EQUAL(bbox.x.min, -4_len);
  BOOST_CHECK_EQUAL(bbox.x.max, 1_len);
  BOOST_CHECK_EQUAL(bbox.y.min, -5_len);
  BOOST_CHECK_EQUAL(bbox.y.max, 2_len);
  BOOST_CHECK_EQUAL(bbox.z.min, 3_len);
  BOOST_CHECK_EQUAL(bbox.z.max, 6_len);
}

BOOST_AUTO_TEST_CASE(test_bounding_box_contains)
{
  core::BoundingBox bbox{{1_len, 2_len, 3_len}, {4_len, 5_len, 6_len}};
  BOOST_CHECK(bbox.x.isValid());
  BOOST_CHECK(bbox.y.isValid());
  BOOST_CHECK(bbox.z.isValid());
  BOOST_CHECK((!bbox.contains({1_len, 2_len, 2_len})));
  BOOST_CHECK((bbox.contains({1_len, 2_len, 3_len})));
  BOOST_CHECK((bbox.contains({1_len, 2_len, 6_len})));
  BOOST_CHECK((!bbox.contains({1_len, 2_len, 7_len})));
}

BOOST_AUTO_TEST_CASE(test_bounding_box_intersects)
{
  core::BoundingBox a{{0_len, 1_len, 2_len}, {3_len, 4_len, 5_len}};
  core::BoundingBox b{{3_len, 10_len, 10_len}, {20_len, 20_len, 20_len}};
  BOOST_CHECK(!a.intersectsExclusive(b));
  core::BoundingBox c{{2_len, 10_len, 10_len}, {20_len, 20_len, 20_len}};
  BOOST_CHECK(!a.intersectsExclusive(c));
  core::BoundingBox d{{2_len, 1_len, 3_len}, {2_len, 1_len, 3_len}};
  BOOST_CHECK(!a.intersectsExclusive(d));
  core::BoundingBox e{{2_len, 1_len, 3_len}, {2_len, 2_len, 4_len}};
  BOOST_CHECK(a.intersectsExclusive(e));
  core::BoundingBox f{{1_len, 2_len, 3_len}, {1_len, 2_len, 3_len}};
  BOOST_CHECK(!f.intersectsExclusive(f));
}

BOOST_AUTO_TEST_CASE(test_atan)
{
  BOOST_CHECK_EQUAL(core::angleFromAtan(0, 0), 0_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(1, 1), 45_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(-1, 1), -45_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(1, 0), 90_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(-1, 0), -90_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(1, -1), 135_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(-1, -1), -135_deg);
  BOOST_CHECK_LE(abs(core::angleFromAtan(0, -1) - 180_deg), 1_au);
}

BOOST_AUTO_TEST_CASE(test_atan_len)
{
  BOOST_CHECK_EQUAL(core::angleFromAtan(0_len, 0_len), 0_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(1_len, 1_len), 45_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(-1_len, 1_len), -45_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(1_len, 0_len), 90_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(-1_len, 0_len), -90_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(1_len, -1_len), 135_deg);
  BOOST_CHECK_EQUAL(core::angleFromAtan(-1_len, -1_len), -135_deg);
  BOOST_CHECK_LE(abs(core::angleFromAtan(0_len, -1_len) - 180_deg), 1_au);
}

BOOST_AUTO_TEST_SUITE_END()
