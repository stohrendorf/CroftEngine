#include "pathfinder.h"

#include "core/interval.h"
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
#include <exception>
#include <functional>

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

struct MovementCalculator
{
  static constexpr auto Margin = 1_sectors / 2;

  static constexpr uint8_t CanMoveXNeg = 1u << 0u;
  static constexpr uint8_t CanMoveXPos = 1u << 1u;
  static constexpr uint8_t CanMoveZNeg = 1u << 2u;
  static constexpr uint8_t CanMoveZPos = 1u << 3u;
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  static constexpr uint8_t CanMoveAllDirs = CanMoveXNeg | CanMoveXPos | CanMoveZNeg | CanMoveZPos;

  explicit MovementCalculator(const core::TRVec& startPos)
      : startPos{startPos}
      , moveTarget{startPos}
  {
  }

  core::TRVec startPos;
  core::TRVec moveTarget;
  bool frozenRanges = false;
  uint8_t moveDirs = CanMoveAllDirs;
  core::Interval<core::Length> xRange{0_len, 0_len};
  core::Interval<core::Length> zRange{0_len, 0_len};

  bool calculate(const gsl::not_null<const world::Box*>& startBox,
                 bool isFlying,
                 const world::Box* goalBox,
                 const core::TRVec& goalPos,
                 const std::function<const world::Box*(const gsl::not_null<const world::Box*>& box)>& getNextPathBox,
                 const std::function<bool(const world::Box& box)>& canVisit)
  {
    auto box = startBox;
    BOOST_ASSERT(startBox->xInterval.contains(startPos.X));
    BOOST_ASSERT(startBox->zInterval.contains(startPos.Z));
    xRange = {0_len, 0_len};
    zRange = {0_len, 0_len};

    moveTarget = startPos;
    frozenRanges = false;
    moveDirs = CanMoveAllDirs;
    while(true)
    {
      if(isFlying)
      {
        moveTarget.Y = std::min(moveTarget.Y, box->floor - 1_sectors);
      }
      else
      {
        moveTarget.Y = std::min(moveTarget.Y, box->floor);
      }

      if(box->xInterval.contains(startPos.X) && box->zInterval.contains(startPos.Z))
      {
        xRange = box->xInterval;
        zRange = box->zInterval;
      }
      else
      {
        if(startPos.Z < box->zInterval.min)
        {
          if(tryMoveZPos(*box))
            return true;
        }
        else if(startPos.Z > box->zInterval.max)
        {
          if(tryMoveZNeg(*box))
            return true;
        }

        if(startPos.X < box->xInterval.min)
        {
          if(tryMoveXPos(*box))
            return true;
        }
        else if(startPos.X > box->xInterval.max)
        {
          if(tryMoveXNeg(*box))
            return true;
        }
      }

      if(box == goalBox)
      {
        calculateFinalMove(*box, goalPos);

        return true;
      }

      const auto nextBox = getNextPathBox(box);
      if(nextBox == nullptr || !canVisit(*nextBox))
        break;

      box = gsl::not_null{nextBox};
    }

    calculateFinalIncompleteMove(*box, isFlying);

    return false;
  }

  bool tryMoveZPos(const world::Box& box)
  {
    // Only refine the moveTarget in the movement area.
    if((moveDirs & CanMoveZPos) && box.xInterval.contains(startPos.X))
    {
      // The "max" is to ensure that it won't go to -Z, as the box's Z range may contain the moveTarget Z.
      moveTarget.Z = std::max(moveTarget.Z, box.zInterval.max - Margin);
    }
    else
    {
      // Move to the virtual wall of our currently allowed movement area. This can happen, for example, when we're
      // in the middle of a room with stairs in a corner.
      moveTarget.Z = zRange.max - Margin;
    }

    if(frozenRanges)
    {
      return true;
    }

    // Try to move to +Z, as we're outside of the box.
    if((moveDirs & CanMoveZPos) && box.xInterval.contains(startPos.X))
    {
      // Scenario 1: We can move to +Z, *and* the new position will have a valid X value. This means we move as little
      // as we can into the new box.

      // We narrow down the valid X values, as the new box may form a narrower passage.
      xRange = xRange.intersect(box.xInterval);

      // Now remember our "primary" movement direction, i.e. the initial direction we moved to.
      moveDirs = CanMoveZPos;
    }
    else
    {
      // Scenario 2: We cannot move to +Z, *or* the new position won't have a valid X value. This means we're not
      // moving in the primary direction anymore, or our X value will become invalid, or both.

      if(moveDirs == CanMoveZPos)
      {
        // We followed our primary direction, so we try to go further in that direction. This usually happens on stairs
        // or slopes that are made of a line of linearly laid out boxes.
        return false;
      }
      else if(moveDirs == CanMoveAllDirs)
      {
        // We reached the maximum possible +Z, and we have no primary direction.
        frozenRanges = true;
      }
      else
      {
        // We didn't go in primary direction. We can call it a day.
        return true;
      }
    }

    return false;
  }

  bool tryMoveZNeg(const world::Box& box)
  {
    if((moveDirs & CanMoveZNeg) && box.xInterval.contains(startPos.X))
    {
      moveTarget.Z = std::min(moveTarget.Z, box.zInterval.min + Margin);
    }
    else
    {
      moveTarget.Z = zRange.min + Margin;
    }

    if(frozenRanges)
    {
      return true;
    }

    if((moveDirs & CanMoveZNeg) && box.xInterval.contains(startPos.X))
    {
      xRange = xRange.intersect(box.xInterval);

      moveDirs = CanMoveZNeg;
    }
    else
    {
      if(moveDirs == CanMoveZNeg)
      {
        return false;
      }
      else if(moveDirs == CanMoveAllDirs)
      {
        frozenRanges = true;
      }
      else
      {
        return true;
      }
    }

    return false;
  }

  bool tryMoveXPos(const world::Box& box)
  {
    if((moveDirs & CanMoveXPos) && box.zInterval.contains(startPos.Z))
    {
      moveTarget.X = std::max(moveTarget.X, box.xInterval.max - Margin);
    }
    else
    {
      moveTarget.X = xRange.max - Margin;
    }

    if(frozenRanges)
    {
      return true;
    }

    if((moveDirs & CanMoveXPos) && box.zInterval.contains(startPos.Z))
    {
      zRange = zRange.intersect(box.zInterval);

      moveDirs = CanMoveXPos;
    }
    else
    {
      if(moveDirs == CanMoveXPos)
      {
        return false;
      }
      else if(moveDirs == CanMoveAllDirs)
      {
        frozenRanges = true;
      }
      else
      {
        return true;
      }
    }

    return false;
  }

  bool tryMoveXNeg(const world::Box& box)
  {
    if((moveDirs & CanMoveXNeg) && box.zInterval.contains(startPos.Z))
    {
      moveTarget.X = std::min(moveTarget.X, box.xInterval.min + Margin);
    }
    else
    {
      moveTarget.X = xRange.min + Margin;
    }

    if(frozenRanges)
    {
      return true;
    }

    if((moveDirs & CanMoveXNeg) && box.zInterval.contains(startPos.Z))
    {
      zRange = zRange.intersect(box.zInterval);

      moveDirs = CanMoveXNeg;
    }
    else
    {
      if(moveDirs == CanMoveXNeg)
      {
        return false;
      }
      else if(moveDirs == CanMoveAllDirs)
      {
        frozenRanges = true;
      }
      else
      {
        return true;
      }
    }

    return false;
  }

  /**
   * Calculate the final movement if we can go straight to the target.
   */
  void calculateFinalMove(const world::Box& box, const core::TRVec& target)
  {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(moveDirs & (CanMoveZNeg | CanMoveZPos))
    {
      moveTarget.Z = target.Z;
    }
    else if(!frozenRanges)
    {
      moveTarget.Z = uncheckedClamp(moveTarget.Z, box.zInterval.narrowed(Margin));
    }
    gsl_Assert(box.zInterval.contains(moveTarget.Z));

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(moveDirs & (CanMoveXNeg | CanMoveXPos))
    {
      moveTarget.X = target.X;
    }
    else if(!frozenRanges)
    {
      moveTarget.X = uncheckedClamp(moveTarget.X, box.xInterval.narrowed(Margin));
    }
    gsl_Assert(box.xInterval.contains(moveTarget.X));

    moveTarget.Y = target.Y;
  }

  /**
   * Calculate the final movement if we cannot directly reach the target. Depending on the current state, will
   * randomise the #moveTarget.
   */
  void calculateFinalIncompleteMove(const world::Box& box, bool isFlying)
  {
    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(moveDirs & (CanMoveZNeg | CanMoveZPos))
    {
      const auto range = box.zInterval.size() - 2 * Margin;
      moveTarget.Z = util::rand15(range) + box.zInterval.min + Margin;
    }
    else if(!frozenRanges)
    {
      moveTarget.Z = uncheckedClamp(moveTarget.Z, box.zInterval.narrowed(Margin));
    }
    gsl_Assert(box.zInterval.contains(moveTarget.Z));

    // NOLINTNEXTLINE(hicpp-signed-bitwise)
    if(moveDirs & (CanMoveXNeg | CanMoveXPos))
    {
      const auto range = box.xInterval.size() - 2 * Margin;
      moveTarget.X = util::rand15(range) + box.xInterval.min + Margin;
    }
    else if(!frozenRanges)
    {
      moveTarget.X = uncheckedClamp(moveTarget.X, box.xInterval.narrowed(Margin));
    }
    gsl_Assert(box.xInterval.contains(moveTarget.X));

    if(isFlying)
      moveTarget.Y = box.floor - 384_len;
    else
      moveTarget.Y = box.floor;
  }
};
} // namespace

bool PathFinder::calculateTarget(const world::World& world,
                                 core::TRVec& moveTarget,
                                 const core::TRVec& startPos,
                                 const gsl::not_null<const world::Box*>& startBox)
{
  gsl_Expects(m_targetBox != nullptr);
  gsl_Expects(m_targetBox->xInterval.contains(m_target.X));
  gsl_Expects(m_targetBox->zInterval.contains(m_target.Z));
  gsl_Expects(startBox->xInterval.contains(startPos.X));
  gsl_Expects(startBox->zInterval.contains(startPos.Z));
  expandNodes(world);

  MovementCalculator calc{startPos};
  auto result = calc.calculate(
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
  moveTarget = calc.moveTarget;
  return result;
}

void PathFinder::expandNodes(const world::World& world)
{
  const auto zoneRef = world::Box::getZoneRef(world.roomsAreSwapped(), isFlying(), m_step);

  static constexpr uint8_t MaxExpansions = 50;

  auto setReachable = [this](const gsl::not_null<const world::Box*>& box, bool reachable)
  {
    m_reachable[box] = reachable;
    if(std::find(m_expansions.begin(), m_expansions.end(), box) == m_expansions.end())
      m_expansions.emplace_back(box);
  };

  // this does a backwards search from the target (usually Lara) to the source (usually a baddie)
  for(uint8_t i = 0; i < MaxExpansions && !m_expansions.empty(); ++i)
  {
    const auto currentBox = m_expansions.front();
    m_expansions.pop_front();
    const auto searchZone = currentBox.get()->*zoneRef;

    for(const auto& successorBox : currentBox->overlaps)
    {
      if(successorBox == currentBox)
        continue;

      if(searchZone != successorBox.get()->*zoneRef)
        continue;

      if(const auto boxHeightDiff = successorBox->floor - currentBox->floor;
         boxHeightDiff < -m_step || boxHeightDiff > -m_drop)
        continue;

      const auto it = m_reachable.find(successorBox);
      const bool successorInitialized = it != m_reachable.end();

      if(!m_reachable.at(currentBox))
      {
        // propagate "unreachable" to all connected boxes if their reachability hasn't been determined yet
        if(!successorInitialized)
        {
          setReachable(successorBox, false);
        }
        continue;
      }

      // propagate "reachable" to all connected boxes if their reachability hasn't been determined yet
      // OR they were previously determined to be unreachable
      if(successorInitialized && it->second)
      {
        // already visited and marked reachable, but path might be shorter
        auto& successorDistance = m_distances[successorBox];
        auto currentDistance = m_distances[currentBox] + 1;
        if(successorDistance > currentDistance)
        {
          successorDistance = currentDistance;
          m_edges.erase(currentBox);
          m_edges.emplace(currentBox, successorBox);
          m_expansions.emplace_back(successorBox);
        }
        continue;
      }

      const auto reachable = canVisit(*successorBox);
      if(reachable)
      {
        BOOST_ASSERT_MSG(m_edges.count(successorBox) == 0, "cycle in pathfinder graph detected");
        m_edges.emplace(successorBox, currentBox); // success! connect both boxes
        m_distances[successorBox] = m_distances[currentBox] + 1;
      }

      setReachable(successorBox, reachable);
    }
  }
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
                      const gsl::not_null<const world::Box*>& box,
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

bool PathFinder::canVisit(const world::Box& box, bool ignoreBlocked, bool ignoreBlockable) const noexcept
{
  if(m_cannotVisitBlocked && box.blocked)
    return ignoreBlocked;
  if(m_cannotVisitBlockable && box.blockable)
    return ignoreBlockable;
  return true;
}

void PathFinder::setRandomSearchTarget(const gsl::not_null<const world::Box*>& box)
{
  const auto xSize = box->xInterval.size() - 2 * MovementCalculator::Margin;
  m_target.X = util::rand15(xSize) + box->xInterval.min + MovementCalculator::Margin;
  const auto zSize = box->zInterval.size() - 2 * MovementCalculator::Margin;
  m_target.Z = util::rand15(zSize) + box->zInterval.min + MovementCalculator::Margin;
  if(isFlying())
  {
    m_target.Y = box->floor - 384_len;
  }
  else
  {
    m_target.Y = box->floor;
  }
}

void PathFinder::setTargetBox(const gsl::not_null<const world::Box*>& box)
{
  if(box == m_targetBox)
    return;

  m_targetBox = box;
  setRandomSearchTarget(box);

  BOOST_LOG_TRIVIAL(debug) << "search reset, found target = "
                           << std::any_of(m_edges.begin(),
                                          m_edges.end(),
                                          [this](const auto& edge)
                                          {
                                            return edge.second == m_targetBox;
                                          })
                           << ", reachable = " << m_reachable[gsl::not_null{m_targetBox}]
                           << ", distance = " << m_distances[gsl::not_null{m_targetBox}];
  m_expansions.clear();
  m_expansions.emplace_back(m_targetBox);
  m_distances.clear();
  m_distances[box] = 0;
  m_reachable.clear();
  m_reachable[box] = true;
  m_edges.clear();
}

const gsl::not_null<const world::Box*>& PathFinder::getRandomBox() const
{
  gsl_Expects(!m_boxes.empty());
  return m_boxes[util::rand15(m_boxes.size())];
}

void PathFinder::resetBoxes(const world::World& world, const gsl::not_null<const world::Box*>& box)
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
                           const gsl::not_null<const world::Box*>& box,
                           const core::Length& step,
                           const core::Length& drop,
                           const core::Length& fly)
{
  gsl_Expects(step >= 0_len);
  gsl_Expects(drop <= 0_len);
  gsl_Expects(fly >= 0_len);
  if(std::exchange(m_step, step) != step && std::exchange(m_drop, drop) != drop && std::exchange(m_fly, fly) != fly)
  {
    resetBoxes(world, box);
  }
  if(m_targetBox != box)
  {
    setTargetBox(box);
    BOOST_ASSERT(std::count(m_boxes.begin(), m_boxes.end(), box) != 0);
    setRandomSearchTarget(box);
  }
}
} // namespace engine::ai
