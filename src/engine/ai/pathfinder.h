#pragma once

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

namespace engine
{
namespace objects
{
struct ObjectState;
}

namespace ai
{
struct PathFinderNode
{
  /**
     * @brief The next box on the path.
     */
  const loader::file::TypedBox* exit_box = nullptr;
  bool traversable = true;

  void serialize(const serialization::Serializer<world::World>& ser);
  static PathFinderNode create(const serialization::Serializer<world::World>& ser);
};

struct PathFinder
{
  std::unordered_map<const loader::file::TypedBox*, PathFinderNode> nodes;
  std::vector<gsl::not_null<const loader::file::TypedBox*>> boxes;
  std::deque<const loader::file::TypedBox*> expansions;
  //! Contains all boxes where the "traversable" state has been determined
  std::unordered_set<const loader::file::TypedBox*> visited;

  bool cannotVisitBlocked = true;
  bool cannotVisitBlockable = false;

  [[nodiscard]] bool canVisit(const loader::file::TypedBox& box) const noexcept
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

  //! @brief The target box we need to reach
  const loader::file::TypedBox* target_box = nullptr;
  const loader::file::TypedBox* required_box = nullptr;

  core::TRVec target;

  explicit PathFinder(const world::World& world);

  void setRandomSearchTarget(const gsl::not_null<const loader::file::TypedBox*>& box)
  {
    required_box = box;
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

  bool calculateTarget(const world::World& world, core::TRVec& moveTarget, const objects::ObjectState& objectState);

  /**
     * @brief Incrementally calculate all paths to a specific box.
     *
     * @details
     * The algorithm performs a greedy breadth-first search, searching for all paths that lead to
     * #required_box.  While searching, @arg maxDepth limits the number of nodes expanded, so it may take multiple
     * calls to actually calculate the full path.  Until a full path is found, the nodes partially retain the old
     * paths from a previous search.
     */
  void updatePath(const world::World& world);

  void searchPath(const world::World& world);

  void serialize(const serialization::Serializer<world::World>& ser);
};
} // namespace ai
} // namespace engine
