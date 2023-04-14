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
      else if(tryMove(*box))
      {
        return true;
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

  bool tryMove(const world::Box& box)
  {
    if(startPos.Z < box.zInterval.min)
    {
      if(tryMoveZPos(box))
        return true;
    }
    else if(startPos.Z > box.zInterval.max)
    {
      if(tryMoveZNeg(box))
        return true;
    }

    if(startPos.X < box.xInterval.min)
    {
      if(tryMoveXPos(box))
        return true;
    }
    else if(startPos.X > box.xInterval.max)
    {
      if(tryMoveXNeg(box))
        return true;
    }

    return false;
  }

  bool finishInvalid()
  {
    if(!frozenRanges && moveDirs == CanMoveAllDirs)
    {
      // We reached the maximum possible +Z, and we have no primary direction.
      frozenRanges = true;
      return false;
    }
    else
    {
      // We didn't go in primary direction. We can call it a day.
      return true;
    }
  }

  bool tryMoveZPos(const world::Box& box)
  {
    // Try to move to +Z, as we're outside the box.
    if((moveDirs & CanMoveZPos) && box.xInterval.contains(startPos.X))
    {
      // Scenario 1: We can move to +Z, *and* the new position will have a valid X value. This means we move as little
      // as we can into the new box.

      // The "max" is to ensure that it won't go to -Z, as the box's Z range may contain the moveTarget Z.
      moveTarget.Z = std::max(moveTarget.Z, box.zInterval.max - Margin);
      if(frozenRanges)
      {
        return true;
      }

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

      // Move to the virtual wall of our currently allowed movement area.
      moveTarget.Z = zRange.max - Margin;

      return finishInvalid();
    }

    return false;
  }

  bool tryMoveZNeg(const world::Box& box)
  {
    if((moveDirs & CanMoveZNeg) && box.xInterval.contains(startPos.X))
    {
      moveTarget.Z = std::min(moveTarget.Z, box.zInterval.min + Margin);
      if(frozenRanges)
      {
        return true;
      }

      xRange = xRange.intersect(box.xInterval);

      moveDirs = CanMoveZNeg;
    }
    else
    {
      if(moveDirs == CanMoveZNeg)
      {
        return false;
      }

      moveTarget.Z = zRange.min + Margin;

      return finishInvalid();
    }

    return false;
  }

  bool tryMoveXPos(const world::Box& box)
  {
    if((moveDirs & CanMoveXPos) && box.zInterval.contains(startPos.Z))
    {
      moveTarget.X = std::max(moveTarget.X, box.xInterval.max - Margin);
      if(frozenRanges)
      {
        return true;
      }

      zRange = zRange.intersect(box.zInterval);

      moveDirs = CanMoveXPos;
    }
    else
    {
      if(moveDirs == CanMoveXPos)
      {
        return false;
      }

      moveTarget.X = xRange.max - Margin;

      return finishInvalid();
    }

    return false;
  }

  bool tryMoveXNeg(const world::Box& box)
  {
    if((moveDirs & CanMoveXNeg) && box.zInterval.contains(startPos.Z))
    {
      moveTarget.X = std::min(moveTarget.X, box.xInterval.min + Margin);
      if(frozenRanges)
      {
        return true;
      }

      zRange = zRange.intersect(box.zInterval);

      moveDirs = CanMoveXNeg;
    }
    else
    {
      if(moveDirs == CanMoveXNeg)
      {
        return false;
      }

      moveTarget.X = xRange.min + Margin;

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

  for(uint8_t i = 0; i < MaxExpansions && !m_expansions.empty(); ++i)
  {
    // this does a backwards search from the target (usually Lara) to the source (usually a baddie), as the expansions
    // are initialised with the target when reset.

    const auto currentBox = m_expansions.front();
    m_expansions.pop_front();
    const auto searchZone = currentBox.get()->*zoneRef;

    for(const auto& predecessorBox : currentBox->overlaps)
    {
      if(predecessorBox == currentBox)
        continue;

      if(searchZone != predecessorBox.get()->*zoneRef)
        continue;

      if(const auto boxHeightDiff = currentBox->floor - predecessorBox->floor;
         boxHeightDiff < -m_step || boxHeightDiff > -m_drop)
        continue;

      // update predecessor reachability and distance
      if(updateEdge(currentBox, predecessorBox))
        continue;

      const auto reachable = canVisit(*predecessorBox);
      if(reachable)
      {
        // success! connect both boxes
        BOOST_ASSERT_MSG(m_edges.find(predecessorBox) == m_edges.end(), "cycle in pathfinder graph detected");
        m_edges.emplace(predecessorBox, currentBox);
        m_distances[predecessorBox] = m_distances[currentBox] + 1;
      }

      setReachable(predecessorBox, reachable);
    }
  }
}

void PathFinder::setReachable(const gsl::not_null<const world::Box*>& box, bool reachable)
{
  m_reachable[box] = reachable;
  if(std::find(m_expansions.begin(), m_expansions.end(), box) == m_expansions.end())
    m_expansions.emplace_back(box);
}

void PathFinder::updateDistance(const gsl::not_null<const world::Box*>& currentBox,
                                const gsl::not_null<const world::Box*>& predecessorBox)
{
  BOOST_ASSERT(m_distances.find(predecessorBox) != m_distances.end());
  BOOST_ASSERT(m_distances.find(currentBox) != m_distances.end());

  auto& currentPredecessorDistance = m_distances[predecessorBox];
  auto newPredecessorDistance = m_distances[currentBox] + 1;
  if(currentPredecessorDistance <= newPredecessorDistance)
    return;

  currentPredecessorDistance = newPredecessorDistance;
  m_edges.erase(currentBox);
  m_edges.emplace(currentBox, predecessorBox);
  m_expansions.emplace_back(predecessorBox);
}

bool PathFinder::updateEdge(const gsl::not_null<const world::Box*>& currentBox,
                            const gsl::not_null<const world::Box*>& predecessorBox)
{
  const auto it = m_reachable.find(predecessorBox);
  const bool predecessorInitialized = it != m_reachable.end();

  if(!m_reachable.at(currentBox))
  {
    // propagate "unreachable" to all connected boxes if their reachability hasn't been determined yet
    if(!predecessorInitialized)
    {
      setReachable(predecessorBox, false);
    }
    return true;
  }

  if(predecessorInitialized && it->second)
  {
    // predecessor was already determined to be reachable, but path might be shorter
    updateDistance(currentBox, predecessorBox);
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
  // NOLINTNEXTLINE(hicpp-signed-bitwise)
  if((std::exchange(m_step, step) != step) | (std::exchange(m_drop, drop) != drop) | (std::exchange(m_fly, fly) != fly))
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
