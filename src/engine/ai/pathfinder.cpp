#include "pathfinder.h"

#include "core/interval.h"
#include "core/magic.h"
#include "core/units.h"
#include "core/vec.h"
#include "engine/script/reflection.h"
#include "engine/world/box.h"
#include "engine/world/world.h"
#include "serialization/box_ptr.h"
#include "serialization/deque.h"
#include "serialization/not_null.h"
#include "serialization/optional.h"
#include "serialization/ptr.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/unordered_map.h"
#include "serialization/vector.h"
#include "serialization/vector_element.h"
#include "util/helpers.h"

#include <algorithm>
#include <boost/assert.hpp>
#include <cstdint>
#include <functional>
#include <gsl-lite/gsl-lite.hpp>
#include <utility>

namespace engine::ai
{
namespace
{
template<typename T>
[[nodiscard]] constexpr const auto& uncheckedClamp(const T& value, const core::Interval<T>& interval)
{
  if(value < interval.min)
    return interval.min;
  else if(value > interval.max)
    return interval.max;
  else
    return value;
}

struct SteeringCalculator
{
  static constexpr auto Margin = 1_sectors / 2;

  static constexpr uint8_t CanMoveXNeg = 1u << 0u;
  static constexpr uint8_t CanMoveXPos = 1u << 1u;
  static constexpr uint8_t CanMoveZNeg = 1u << 2u;
  static constexpr uint8_t CanMoveZPos = 1u << 3u;
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  static constexpr uint8_t CanMoveAllDirs = CanMoveXNeg | CanMoveXPos | CanMoveZNeg | CanMoveZPos;

  explicit SteeringCalculator(const core::TRVec& origin)
      : m_origin{origin}
      , m_target{origin}
  {
  }

  core::TRVec m_origin;       // The current position of the entity
  core::TRVec m_target;       // The calculated steering target
  bool m_isDiverging = false; // True if the shortcut path is restricted by box boundaries
  uint8_t m_directionMask = CanMoveAllDirs;
  core::Interval<core::Length> m_allowedXRange{0_len, 0_len};
  core::Interval<core::Length> m_allowedZRange{0_len, 0_len};

  bool calculate(
    const gsl_lite::not_null<const world::Box*>& startBox,
    const bool isFlying,
    const world::Box* goalBox,
    const core::TRVec& goalPos,
    const std::function<const world::Box*(const gsl_lite::not_null<const world::Box*>& box)>& getNextBoxInPath,
    const std::function<bool(const world::Box& box)>& isBoxVisitable)
  {
    auto box = startBox;
    BOOST_ASSERT(startBox->xInterval.contains(m_origin.X));
    BOOST_ASSERT(startBox->zInterval.contains(m_origin.Z));
    m_allowedXRange = {0_len, 0_len};
    m_allowedZRange = {0_len, 0_len};

    m_target = m_origin;
    m_isDiverging = false;
    m_directionMask = CanMoveAllDirs;
    while(true)
    {
      if(isFlying)
      {
        m_target.Y = std::min(m_target.Y, box->floor - 1_sectors);
      }
      else
      {
        m_target.Y = std::min(m_target.Y, box->floor);
      }

      if(box->xInterval.contains(m_origin.X) && box->zInterval.contains(m_origin.Z))
      {
        // Scenario 1: We are still within the boundaries of the current box.
        // The allowed range for shortcutting is the box itself.
        m_allowedXRange = box->xInterval;
        m_allowedZRange = box->zInterval;
      }
      else if(tryMove(*box))
      {
        // Scenario 2: We are trying to shortcut through a box that doesn't contain the origin.
        // If tryMove returns true, it means we've reached a point where we must change direction
        // or stop shortcutting because the next box is not in a straight line.
        return true;
      }

      if(box == goalBox)
      {
        calculateFinalMove(*box, goalPos);

        return true;
      }

      const auto nextBox = getNextBoxInPath(box);
      if(nextBox == nullptr || !isBoxVisitable(*nextBox))
        break;

      box = gsl_lite::not_null{nextBox};
    }

    calculateFinalIncompleteMove(*box, isFlying);

    return false;
  }

  bool tryMove(const world::Box& box)
  {
    if(m_origin.Z < box.zInterval.min)
    {
      if(tryMoveZPos(box))
        return true;
    }
    else if(m_origin.Z > box.zInterval.max)
    {
      if(tryMoveZNeg(box))
        return true;
    }

    if(m_origin.X < box.xInterval.min)
    {
      if(tryMoveXPos(box))
        return true;
    }
    else if(m_origin.X > box.xInterval.max)
    {
      if(tryMoveXNeg(box))
        return true;
    }

    return false;
  }

  bool finishInvalid()
  {
    if(!m_isDiverging && m_directionMask == CanMoveAllDirs)
    {
      // We haven't established a primary movement direction yet.
      m_isDiverging = true;
      return false;
    }
    else
    {
      // We cannot shortcut in the primary direction anymore.
      return true;
    }
  }

  bool tryMoveZPos(const world::Box& box)
  {
    // Try to move to +Z, as we're outside the box.
    if((m_directionMask & CanMoveZPos) && box.xInterval.contains(m_origin.X))
    {
      // Scenario 1: We can move to +Z, *and* the current X position is within the new box's X range.
      // This allows us to extend the current straight-line shortcut.

      // The "max" is to ensure that it won't go to -Z, as the box's Z range may contain the m_target Z.
      m_target.Z = std::max(m_target.Z, box.zInterval.max - Margin);
      if(m_isDiverging)
      {
        return true;
      }

      // We narrow down the valid X values, as the new box may form a narrower passage.
      m_allowedXRange = m_allowedXRange.intersect(box.xInterval);

      // Now remember our "primary" movement direction, i.e. the initial direction we moved to.
      m_directionMask = CanMoveZPos;
    }
    else
    {
      // Scenario 2: We cannot move straight to +Z anymore.
      if(m_directionMask == CanMoveZPos)
      {
        // If this was our primary direction, keep going for now (e.g. following a curved corridor).
        return false;
      }

      // Move to the edge of our currently allowed area.
      m_target.Z = m_allowedZRange.max - Margin;

      return finishInvalid();
    }

    return false;
  }

  bool tryMoveZNeg(const world::Box& box)
  {
    if((m_directionMask & CanMoveZNeg) && box.xInterval.contains(m_origin.X))
    {
      m_target.Z = std::min(m_target.Z, box.zInterval.min + Margin);
      if(m_isDiverging)
      {
        return true;
      }

      m_allowedXRange = m_allowedXRange.intersect(box.xInterval);

      m_directionMask = CanMoveZNeg;
    }
    else
    {
      if(m_directionMask == CanMoveZNeg)
      {
        return false;
      }

      m_target.Z = m_allowedZRange.min + Margin;

      return finishInvalid();
    }

    return false;
  }

  bool tryMoveXPos(const world::Box& box)
  {
    if((m_directionMask & CanMoveXPos) && box.zInterval.contains(m_origin.Z))
    {
      m_target.X = std::max(m_target.X, box.xInterval.max - Margin);
      if(m_isDiverging)
      {
        return true;
      }

      m_allowedZRange = m_allowedZRange.intersect(box.zInterval);

      m_directionMask = CanMoveXPos;
    }
    else
    {
      if(m_directionMask == CanMoveXPos)
      {
        return false;
      }

      m_target.X = m_allowedXRange.max - Margin;

      return finishInvalid();
    }

    return false;
  }

  bool tryMoveXNeg(const world::Box& box)
  {
    if((m_directionMask & CanMoveXNeg) && box.zInterval.contains(m_origin.Z))
    {
      m_target.X = std::min(m_target.X, box.xInterval.min + Margin);
      if(m_isDiverging)
      {
        return true;
      }

      m_allowedZRange = m_allowedZRange.intersect(box.zInterval);

      m_directionMask = CanMoveXNeg;
    }
    else
    {
      if(m_directionMask == CanMoveXNeg)
      {
        return false;
      }

      m_target.X = m_allowedXRange.min + Margin;

      return finishInvalid();
    }

    return false;
  }

  /**
   * Calculate the final movement if we can go straight to the target.
   */
  void calculateFinalMove(const world::Box& box, const core::TRVec& target)
  {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(m_directionMask & (CanMoveZNeg | CanMoveZPos))
    {
      m_target.Z = target.Z;
    }
    else if(!m_isDiverging)
    {
      m_target.Z = uncheckedClamp(m_target.Z, box.zInterval.narrowed(Margin));
    }
    gsl_Assert(box.zInterval.contains(m_target.Z));

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(m_directionMask & (CanMoveXNeg | CanMoveXPos))
    {
      m_target.X = target.X;
    }
    else if(!m_isDiverging)
    {
      m_target.X = uncheckedClamp(m_target.X, box.xInterval.narrowed(Margin));
    }
    gsl_Assert(box.xInterval.contains(m_target.X));

    m_target.Y = target.Y;
  }

  /**
   * Calculate the final movement if we cannot directly reach the target.
   * Depending on the current state, will randomise the target position within the current box.
   */
  void calculateFinalIncompleteMove(const world::Box& box, const bool isFlying)
  {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(m_directionMask & (CanMoveZNeg | CanMoveZPos))
    {
      const auto range = box.zInterval.size() - 2 * Margin;
      m_target.Z = util::rand15(range) + box.zInterval.min + Margin;
    }
    else if(!m_isDiverging)
    {
      m_target.Z = uncheckedClamp(m_target.Z, box.zInterval.narrowed(Margin));
    }
    gsl_Assert(box.zInterval.contains(m_target.Z));

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(m_directionMask & (CanMoveXNeg | CanMoveXPos))
    {
      const auto range = box.xInterval.size() - 2 * Margin;
      m_target.X = util::rand15(range) + box.xInterval.min + Margin;
    }
    else if(!m_isDiverging)
    {
      m_target.X = uncheckedClamp(m_target.X, box.xInterval.narrowed(Margin));
    }
    gsl_Assert(box.xInterval.contains(m_target.X));

    if(isFlying)
      m_target.Y = box.floor - 384_len;
    else
      m_target.Y = box.floor;
  }
};
} // namespace

bool PathFinder::calculateTarget(const world::World& world,
                                 core::TRVec& moveTarget,
                                 const core::TRVec& startPos,
                                 const gsl_lite::not_null<const world::Box*>& startBox)
{
  gsl_Expects(m_targetBox != nullptr);
  gsl_Expects(m_targetBox->xInterval.contains(m_target.X));
  gsl_Expects(m_targetBox->zInterval.contains(m_target.Z));
  gsl_Expects(startBox->xInterval.contains(startPos.X));
  gsl_Expects(startBox->zInterval.contains(startPos.Z));
  expandPathGraph(world);

  SteeringCalculator calc{startPos};
  const auto result = calc.calculate(
    startBox,
    isFlying(),
    m_targetBox,
    m_target,
    [this](auto box)
    {
      return getNextPathBox(box);
    },
    [this](auto box)
    {
      return canVisit(box);
    });
  moveTarget = calc.m_target;
  return result;
}

void PathFinder::expandPathGraph(const world::World& world)
{
  const auto zoneRef = world::Box::getZoneRef(world.roomsAreSwapped(), isFlying(), m_step);

  static constexpr uint8_t MaxExpansionsPerTick = 50;

  for(uint8_t i = 0; i < MaxExpansionsPerTick && !m_expansions.empty(); ++i)
  {
    // This does a backwards search from the target (usually Lara) towards the source (the AI entity).
    // The m_expansions queue is initialized with the target box.

    const auto current = m_expansions.front();
    m_expansions.pop_front();
    const auto searchZone = current.get()->*zoneRef;

    for(const auto& neighbor : current->overlaps)
    {
      if(neighbor == current)
        continue;

      if(searchZone != neighbor.get()->*zoneRef)
      {
        setReachable(neighbor, false);
        continue;
      }

      if(const auto dy = current->floor - neighbor->floor; dy < -m_step || dy > -m_drop)
      {
        setReachable(neighbor, false);
        continue;
      }

      // Update reachability and distance of the neighbor
      if(updateEdge(current, neighbor))
        continue;

      const auto reachable = canVisit(*neighbor);
      if(reachable)
      {
        // Success! Connect both boxes.
        BOOST_ASSERT_MSG(!m_edges.contains(neighbor), "cycle in pathfinder graph detected");
        m_edges.emplace(neighbor, current);
        m_distances[neighbor] = m_distances[current] + 1;
      }

      setReachable(neighbor, reachable);
    }
  }
}

void PathFinder::setReachable(const gsl_lite::not_null<const world::Box*>& box, const bool reachable)
{
  m_reachable[box] |= reachable;
  if(reachable && std::ranges::find(m_expansions, box) == m_expansions.end())
    m_expansions.emplace_back(box);
}

void PathFinder::updateDistance(const gsl_lite::not_null<const world::Box*>& current,
                                const gsl_lite::not_null<const world::Box*>& neighbor)
{
  BOOST_ASSERT(m_distances.contains(neighbor));
  BOOST_ASSERT(m_distances.contains(current));

  auto& neighborDistance = m_distances[neighbor];
  const auto newDistance = m_distances[current] + 1;
  if(neighborDistance <= newDistance)
    return;

  neighborDistance = newDistance;
  m_edges.erase(current);
  m_edges.emplace(current, neighbor);
  m_expansions.emplace_back(neighbor);
}

bool PathFinder::updateEdge(const gsl_lite::not_null<const world::Box*>& current,
                            const gsl_lite::not_null<const world::Box*>& neighbor)
{
  if(!m_reachable.at(current))
  {
    // Propagate "unreachable" to neighbors
    setReachable(neighbor, false);
    return true;
  }

  if(const auto it = m_reachable.find(neighbor); it != m_reachable.end() && it->second)
  {
    // Neighbor was already reachable, but we might have found a shorter path.
    updateDistance(current, neighbor);
    return true;
  }

  return false;
}

void PathFinder::serialize(const serialization::Serializer<world::World>& ser) const
{
  ser(S_NV("edges", m_edges),
      S_NV("boxes", m_boxes),
      S_NV("expansions", m_expansions),
      S_NV("distances", m_distances),
      S_NV("reachable", m_reachable),
      S_NV("cannotVisitBlockable", m_cannotVisitBlockable),
      S_NV("cannotVisitBlocked", m_cannotVisitBlocked),
      S_NV("step", m_step),
      S_NV("drop", m_drop),
      S_NV("fly", m_fly),
      S_NV_VECTOR_ELEMENT("targetBox", std::cref(ser.context->getBoxes()), std::cref(m_targetBox)),
      S_NV("target", m_target));
}

void PathFinder::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ser(S_NV("edges", m_edges),
      S_NV("boxes", m_boxes),
      S_NV("expansions", m_expansions),
      S_NV("distances", m_distances),
      S_NV("reachable", m_reachable),
      S_NV("cannotVisitBlockable", m_cannotVisitBlockable),
      S_NV("cannotVisitBlocked", m_cannotVisitBlocked),
      S_NV("step", m_step),
      S_NV("drop", m_drop),
      S_NV("fly", m_fly),
      S_NV_VECTOR_ELEMENT("targetBox", std::cref(ser.context->getBoxes()), std::ref(m_targetBox)),
      S_NV("target", m_target));
}

void PathFinder::init(const world::World& world,
                      const gsl_lite::not_null<const world::Box*>& box,
                      const script::ObjectInfo& objectInfo)
{
  m_cannotVisitBlockable = objectInfo.cannot_visit_blockable;
  m_cannotVisitBlocked = objectInfo.cannot_visit_blocked;

  resetBoxes(world, box);
  setLimits(world,
            box,
            core::Length{objectInfo.step_limit},
            core::Length{objectInfo.drop_limit},
            core::Length{objectInfo.fly_limit});
}

bool PathFinder::canVisit(const world::Box& box) const noexcept
{
  return canVisit(box, false, false);
}

bool PathFinder::canVisit(const world::Box& box, const bool ignoreBlocked, const bool ignoreBlockable) const noexcept
{
  if(m_cannotVisitBlocked && box.blocked)
    return ignoreBlocked;

  if(m_cannotVisitBlockable && box.blockable)
    return ignoreBlockable;

  return true;
}

void PathFinder::setRandomSearchTarget(const gsl_lite::not_null<const world::Box*>& box)
{
  const auto xSize = box->xInterval.size() - 2 * SteeringCalculator::Margin;
  m_target.X = util::rand15(xSize) + box->xInterval.min + SteeringCalculator::Margin;
  const auto zSize = box->zInterval.size() - 2 * SteeringCalculator::Margin;
  m_target.Z = util::rand15(zSize) + box->zInterval.min + SteeringCalculator::Margin;
  if(isFlying())
  {
    m_target.Y = box->floor - 384_len;
  }
  else
  {
    m_target.Y = box->floor;
  }
}

void PathFinder::setTargetBox(const gsl_lite::not_null<const world::Box*>& box)
{
  if(box == m_targetBox)
    return;

  m_targetBox = box;
  setRandomSearchTarget(box);

  m_expansions.clear();
  m_expansions.emplace_back(m_targetBox);
  m_distances.clear();
  m_distances[box] = 0;
  m_reachable.clear();
  m_reachable[box] = true;
  m_edges.clear();
}

const gsl_lite::not_null<const world::Box*>& PathFinder::getRandomBox() const
{
  gsl_Expects(!m_boxes.empty());
  return m_boxes[util::rand15(m_boxes.size())];
}

void PathFinder::resetBoxes(const world::World& world, const gsl_lite::not_null<const world::Box*>& box)
{
  const auto zoneRef1 = world::Box::getZoneRef(false, isFlying(), m_step);
  const auto zoneRef2 = world::Box::getZoneRef(true, isFlying(), m_step);
  const auto zoneData1 = box.get()->*zoneRef1;
  const auto zoneData2 = box.get()->*zoneRef2;
  m_boxes.clear();
  for(const auto& levelBox : world.getBoxes())
  {
    if(levelBox.*zoneRef1 == zoneData1 || levelBox.*zoneRef2 == zoneData2)
    {
      m_boxes.emplace_back(&levelBox);
    }
  }
}

void PathFinder::setLimits(const world::World& world,
                           const world::Box* box,
                           const core::Length& step,
                           const core::Length& drop,
                           const core::Length& fly)
{
  gsl_Expects(step >= 0_len);
  gsl_Expects(drop <= 0_len);
  gsl_Expects(fly >= 0_len);
  if(box == nullptr)
    return;

  // NOLINTNEXTLINE(hicpp-signed-bitwise, bitwise-instead-of-logical)
  if((std::exchange(m_step, step) != step) | (std::exchange(m_drop, drop) != drop) | (std::exchange(m_fly, fly) != fly))
  {
    resetBoxes(world, gsl_lite::not_null{box});
  }
  if(m_targetBox != box)
  {
    setTargetBox(gsl_lite::not_null{box});
    BOOST_ASSERT(std::ranges::count(m_boxes, box) != 0);
    setRandomSearchTarget(gsl_lite::not_null{box});
  }
}
} // namespace engine::ai