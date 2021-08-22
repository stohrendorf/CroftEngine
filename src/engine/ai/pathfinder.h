#pragma once

#include "engine/world/box.h"
#include "loader/file/datatypes.h"
#include "serialization/serialization_fwd.h"
#include "util/helpers.h"

#include <deque>
#include <unordered_map>
#include <unordered_set>

namespace engine::world
{
class World;
}

namespace engine::ai
{
struct PathFinder
{
  bool cannotVisitBlocked = true;
  bool cannotVisitBlockable = false;

  [[nodiscard]] bool canVisit(const world::Box& box) const noexcept
  {
    if(cannotVisitBlocked && box.blocked)
      return false;
    if(cannotVisitBlockable && box.blockable)
      return false;
    return true;
  }

  //! @brief Movement limits
  //! @{
  core::Length step = core::QuarterSectorSize;
  core::Length drop = -core::QuarterSectorSize;
  core::Length fly = 0_len;
  //! @}

  core::TRVec target;

  void setRandomSearchTarget(const gsl::not_null<const world::Box*>& box)
  {
    const auto zSize = box->zmax - box->zmin - core::SectorSize;
    target.Z = util::rand15(zSize) + box->zmin + core::SectorSize / 2;
    const auto xSize = box->xmax - box->xmin - core::SectorSize;
    target.X = util::rand15(xSize) + box->xmin + core::SectorSize / 2;
    if(fly != 0_len)
    {
      target.Y = box->floor - 384_len;
    }
    else
    {
      target.Y = box->floor;
    }
  }

  bool calculateTarget(const world::World& world,
                       core::TRVec& moveTarget,
                       const core::TRVec& startPos,
                       const gsl::not_null<const world::Box*>& startBox);

  void setTargetBox(const gsl::not_null<const world::Box*>& box)
  {
    if(box == m_targetBox)
      return;

    m_targetBox = box;

    m_expansions.clear();
    m_expansions.emplace_back(m_targetBox);
    m_reachable.clear();
    m_reachable[m_targetBox] = true;
  }

  void serialize(const serialization::Serializer<world::World>& ser);

  void collectBoxes(const world::World& world, const gsl::not_null<const world::Box*>& box);

  // returns true if and only if the box is visited and marked unreachable
  [[nodiscard]] bool isUnreachable(const gsl::not_null<const world::Box*>& box) const
  {
    const auto it = m_reachable.find(box);
    return it != m_reachable.end() && !it->second;
  }

  [[nodiscard]] const auto& getRandomBox() const
  {
    Expects(!m_boxes.empty());
    return m_boxes[util::rand15(m_boxes.size())];
  }

  [[nodiscard]] const world::Box* getNextPathBox(const gsl::not_null<const world::Box*>& box) const
  {
    const auto it = m_edges.find(box);
    return it == m_edges.end() ? nullptr : it->second.get();
  }

  [[nodiscard]] const auto& getTargetBox() const
  {
    return m_targetBox;
  }

  [[nodiscard]] bool isFlying() const
  {
    return fly != 0_len;
  }

private:
  void searchPath(const world::World& world);

  std::vector<gsl::not_null<const world::Box*>> m_boxes;
  std::deque<gsl::not_null<const world::Box*>> m_expansions;
  std::unordered_map<gsl::not_null<const world::Box*>, bool> m_reachable;
  std::unordered_map<gsl::not_null<const world::Box*>, gsl::not_null<const world::Box*>> m_edges;
  //! @brief The target box we need to reach
  const world::Box* m_targetBox = nullptr;
};
} // namespace engine::ai
