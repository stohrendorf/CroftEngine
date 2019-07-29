#include "pathfinder.h"

#include "engine/engine.h"
#include "engine/items/itemnode.h"

namespace engine
{
namespace ai
{
PathFinder::PathFinder(const engine::Engine& engine)
{
    for(const auto& box : engine.getBoxes())
        nodes.insert(std::make_pair(&box, PathFinderNode{}));
}

namespace
{
gsl::span<const uint16_t> getOverlaps(const Engine& engine, const uint16_t idx)
{
    const auto first = &engine.getOverlaps().at(idx);
    auto last = first;
    const auto endOfUniverse = &engine.getOverlaps().back() + 1;

    while(last < endOfUniverse && (*last & 0x8000u) == 0)
    {
        ++last;
    }

    return gsl::make_span(first, last + 1);
}
} // namespace

bool PathFinder::calculateTarget(const Engine& engine, core::TRVec& moveTarget, const items::ItemState& item)
{
    updatePath(engine);

    moveTarget = item.position.position;

    auto here = item.box;
    if(here == nullptr)
        return false;

    core::Length minZ = 0_len, maxZ = 0_len, minX = 0_len, maxX = 0_len;

    const auto clampX = [&minX, &maxX, &here]() {
        minX = std::max(minX, here->xmin);
        maxX = std::min(maxX, here->xmax);
    };

    const auto clampZ = [&minZ, &maxZ, &here]() {
        minZ = std::max(minZ, here->zmin);
        maxZ = std::min(maxZ, here->zmax);
    };

    constexpr uint8_t CanMoveXPos = 0x01u;
    constexpr uint8_t CanMoveXNeg = 0x02u;
    constexpr uint8_t CanMoveZPos = 0x04u;
    constexpr uint8_t CanMoveZNeg = 0x08u;
    constexpr uint8_t CanMoveAllDirs = CanMoveXPos | CanMoveXNeg | CanMoveZPos | CanMoveZNeg;
    bool detour = false;

    uint8_t moveDirs = CanMoveAllDirs;
    while(true)
    {
        if(fly != 0_len)
        {
            if(here->floor - core::SectorSize < moveTarget.Y)
                moveTarget.Y = here->floor - core::SectorSize;
        }
        else
        {
            if(here->floor < moveTarget.Y)
                moveTarget.Y = here->floor;
        }

        if(here->contains(item.position.position.X, item.position.position.Z))
        {
            minZ = here->zmin;
            maxZ = here->zmax;
            minX = here->xmin;
            maxX = here->xmax;
        }
        else
        {
            if(item.position.position.Z < here->zmin)
            {
                // try to move to -Z
                if((moveDirs & CanMoveZNeg) && here->containsX(item.position.position.X))
                {
                    // can move straight to -Z while not leaving the X limits of the current box
                    moveTarget.Z = std::max(moveTarget.Z, here->zmin + core::SectorSize / 2);

                    if(detour)
                        return true;

                    // narrow X to the current box limits, ensure we can only move to -Z from now on
                    clampX();
                    moveDirs = CanMoveZNeg;
                }
                else if(detour || moveDirs != CanMoveZNeg)
                {
                    moveTarget.Z = maxZ - core::SectorSize / 2;
                    if(detour || moveDirs != CanMoveAllDirs)
                        return true;

                    detour = true;
                }
            }
            else if(item.position.position.Z > here->zmax)
            {
                if((moveDirs & CanMoveZPos) && here->containsX(item.position.position.X))
                {
                    moveTarget.Z = std::min(moveTarget.Z, here->zmax - core::SectorSize / 2);

                    if(detour)
                        return true;

                    clampX();

                    moveDirs = CanMoveZPos;
                }
                else if(detour || moveDirs != CanMoveZPos)
                {
                    moveTarget.Z = minZ + core::SectorSize / 2;
                    if(detour || moveDirs != CanMoveAllDirs)
                        return true;

                    detour = true;
                }
            }

            if(item.position.position.X < here->xmin)
            {
                if((moveDirs & CanMoveXNeg) && here->containsZ(item.position.position.Z))
                {
                    moveTarget.X = std::max(moveTarget.X, here->xmin + core::SectorSize / 2);

                    if(detour)
                        return true;

                    clampZ();

                    moveDirs = CanMoveXNeg;
                }
                else if(detour || moveDirs != CanMoveXNeg)
                {
                    moveTarget.X = maxX - core::SectorSize / 2;
                    if(detour || moveDirs != CanMoveAllDirs)
                        return true;

                    detour = true;
                }
            }
            else if(item.position.position.X > here->xmax)
            {
                if((moveDirs & CanMoveXPos) && here->containsZ(item.position.position.Z))
                {
                    moveTarget.X = std::min(moveTarget.X, here->xmax - core::SectorSize / 2);

                    if(detour)
                        return true;

                    clampZ();

                    moveDirs = CanMoveXPos;
                }
                else if(detour || moveDirs != CanMoveXPos)
                {
                    moveTarget.X = minX + core::SectorSize / 2;
                    if(detour || moveDirs != CanMoveAllDirs)
                        return true;

                    detour = true;
                }
            }
        }

        if(here == target_box)
        {
            if(moveDirs & (CanMoveZPos | CanMoveZNeg))
            {
                moveTarget.Z = target.Z;
            }
            else if(!detour)
            {
                moveTarget.Z
                    = util::clamp(moveTarget.Z, here->zmin + core::SectorSize / 2, here->zmax - core::SectorSize / 2);
            }

            if(moveDirs & (CanMoveXPos | CanMoveXNeg))
            {
                moveTarget.X = target.X;
            }
            else if(!detour)
            {
                moveTarget.X
                    = util::clamp(moveTarget.X, here->xmin + core::SectorSize / 2, here->xmax - core::SectorSize / 2);
            }

            moveTarget.Y = target.Y;

            return true;
        }

        const auto nextBox = nodes[here].exit_box;
        if(nextBox == nullptr || !canVisit(*nextBox))
            break;

        here = nextBox;
    }

    BOOST_ASSERT(here != nullptr);
    if(moveDirs & (CanMoveZPos | CanMoveZNeg))
    {
        const auto center = here->zmax - here->zmin - core::SectorSize;
        moveTarget.Z = util::rand15(center) + here->zmin + core::SectorSize / 2;
    }
    else if(!detour)
    {
        moveTarget.Z = util::clamp(moveTarget.Z, here->zmin + core::SectorSize / 2, here->zmax - core::SectorSize / 2);
    }

    if(moveDirs & (CanMoveXPos | CanMoveXNeg))
    {
        const auto center = here->xmax - here->xmin - core::SectorSize;
        moveTarget.X = util::rand15(center) + here->xmin + core::SectorSize / 2;
    }
    else if(!detour)
    {
        moveTarget.X = util::clamp(moveTarget.X, here->xmin + core::SectorSize / 2, here->xmax - core::SectorSize / 2);
    }

    if(fly != 0_len)
        moveTarget.Y = here->floor - 384_len;
    else
        moveTarget.Y = here->floor;

    return false;
}

YAML::Node PathFinder::save(const Engine& engine) const
{
    YAML::Node node;
    for(const auto& entry : nodes)
        node["nodes"][std::distance(&engine.getBoxes()[0], entry.first)] = entry.second.save(engine);
    for(const auto& box : boxes)
        node["boxes"].push_back(std::distance(&engine.getBoxes()[0], box.get()));
    for(const auto& box : expansions)
        node["expansions"].push_back(std::distance(&engine.getBoxes()[0], box));
    for(const auto& box : visited)
        node["visited"].push_back(std::distance(&engine.getBoxes()[0], box));
    node["cannotVisitBlockable"] = cannotVisitBlockable;
    node["cannotVisitBlocked"] = cannotVisitBlocked;
    node["step"] = step;
    node["drop"] = drop;
    node["fly"] = fly;
    if(target_box != nullptr)
        node["targetBox"] = std::distance(&engine.getBoxes()[0], target_box);
    if(required_box != nullptr)
        node["requiredBox"] = std::distance(&engine.getBoxes()[0], required_box);
    node["target"] = target.save();

    return node;
}

void PathFinder::load(const YAML::Node& n, const Engine& engine)
{
    nodes.clear();
    for(const auto& entry : n["nodes"])
        nodes[&engine.getBoxes().at(entry.first.as<size_t>())].load(entry.second, engine);
    boxes.clear();
    for(const auto& entry : n["boxes"])
        boxes.emplace_back(&engine.getBoxes().at(entry.as<size_t>()));
    expansions.clear();
    for(const auto& e : n["expansions"])
        expansions.emplace_back(&engine.getBoxes().at(e.as<size_t>()));
    visited.clear();
    for(const auto& e : n["visited"])
        visited.emplace(&engine.getBoxes().at(e.as<size_t>()));
    cannotVisitBlockable = n["cannotVisitBlockable"].as<bool>();
    cannotVisitBlocked = n["cannotVisitBlocked"].as<bool>();
    step = n["step"].as<core::Length>();
    drop = n["drop"].as<core::Length>();
    fly = n["fly"].as<core::Length>();
    if(!n["targetBox"].IsDefined())
        target_box = nullptr;
    else
        target_box = &engine.getBoxes().at(n["targetBox"].as<size_t>());
    if(!n["requiredBox"].IsDefined())
        required_box = nullptr;
    else
        required_box = &engine.getBoxes().at(n["requiredBox"].as<size_t>());
    target.load(n["target"]);
}

void PathFinder::updatePath(const engine::Engine& engine)
{
    if(required_box != nullptr && required_box != target_box)
    {
        target_box = required_box;

        nodes[target_box].exit_box = nullptr;
        nodes[target_box].traversable = true;
        expansions.clear();
        expansions.emplace_back(target_box);
        visited.clear();
        visited.emplace(target_box);
    }

    Expects(target_box != nullptr);
    searchPath(engine);
}

void PathFinder::searchPath(const engine::Engine& engine)
{
    const auto zoneRef = loader::file::Box::getZoneRef(engine.roomsAreSwapped(), fly, step);

    static constexpr const uint8_t MaxExpansions = 5;

    for(uint8_t i = 0; i < MaxExpansions && !expansions.empty(); ++i)
    {
        const auto current = expansions.front();
        expansions.pop_front();
        const auto& currentNode = nodes[current];
        const auto searchZone = current->*zoneRef;

        for(const auto overlapBoxIdx : getOverlaps(engine, current->overlap_index))
        {
            const auto* successorBox = &engine.getBoxes().at(overlapBoxIdx & 0x7FFFu);

            if(successorBox == current)
                continue;

            if(searchZone != successorBox->*zoneRef)
                continue; // cannot switch zones

            const auto boxHeightDiff = successorBox->floor - current->floor;
            if(boxHeightDiff > step || boxHeightDiff < drop)
                continue; // can't reach from this box, but still maybe from another one

            auto& successorNode = nodes[successorBox];

            if(!currentNode.traversable)
            {
                if(visited.emplace(successorBox).second)
                    successorNode.traversable = false;
            }
            else
            {
                if(successorNode.traversable && visited.count(successorBox) != 0)
                    continue; // already visited and marked reachable

                // mark as visited and check if traversable (may switch traversable to true)
                visited.emplace(successorBox);
                successorNode.traversable = canVisit(*successorBox);
                if(successorNode.traversable)
                    successorNode.exit_box = current; // success! connect both boxes

                if(std::find(expansions.begin(), expansions.end(), successorBox) == expansions.end())
                    expansions.emplace_back(successorBox);
            }
        }
    }
}

YAML::Node PathFinderNode::save(const Engine& engine) const
{
    YAML::Node node;
    node["traversable"] = traversable;
    if(exit_box != nullptr)
        node["exitBox"] = std::distance(&engine.getBoxes()[0], exit_box);
    return node;
}

void PathFinderNode::load(const YAML::Node& n, const Engine& engine)
{
    traversable = n["traversable"].as<bool>();
    if(!n["exitBox"].IsDefined())
        exit_box = nullptr;
    else
        exit_box = &engine.getBoxes().at(n["exitBox"].as<size_t>());
}
} // namespace ai
}