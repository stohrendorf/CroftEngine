#pragma once

#include "engine/engine.h"
#include "engine/items/itemnode.h"

#include <boost/range/adaptor/map.hpp>
#include <unordered_map>

namespace engine
{
namespace items
{
class AIAgent;
}

namespace ai
{
enum class Mood
{
    Bored,
    Attack,
    Escape,
    Stalk
};

inline std::ostream& operator<<(std::ostream& str, const Mood mood)
{
    switch(mood)
    {
    case Mood::Bored: return str << "Bored";
    case Mood::Attack: return str << "Attack";
    case Mood::Escape: return str << "Escape";
    case Mood::Stalk: return str << "Stalk";
    default: BOOST_THROW_EXCEPTION(std::runtime_error("Invalid mood"));
    }
}

struct SearchNode
{
    /**
     * @brief The next box on the path.
     */
    const loader::file::Box* exit_box = nullptr;

    bool blocked = false;

    YAML::Node save(const Engine& engine) const;

    void load(const YAML::Node& n, const Engine& engine);
};

struct LotInfo
{
    std::unordered_map<const loader::file::Box*, SearchNode> nodes;

    std::vector<gsl::not_null<const loader::file::Box*>> boxes;

    std::deque<const loader::file::Box*> expansions;
    //! Contains all boxes where the "blocked" state has been determined
    std::unordered_set<const loader::file::Box*> visited;

    bool cannotVisitBlocked = true;
    bool cannotVisitBlockable = false;

    bool canVisit(const loader::file::Box& box) const noexcept
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
    const loader::file::Box* target_box = nullptr;

    const loader::file::Box* required_box = nullptr;

    core::TRVec target;

    explicit LotInfo(const engine::Engine& engine)
    {
        for(const auto& box : engine.getBoxes())
            nodes.insert(std::make_pair(&box, SearchNode{}));
    }

    static gsl::span<const uint16_t> getOverlaps(const engine::Engine& engine, uint16_t idx);

    void setRandomSearchTarget(const gsl::not_null<const loader::file::Box*>& box)
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

    bool calculateTarget(const engine::Engine& engine, core::TRVec& moveTarget, const items::ItemState& item);

    /**
     * @brief Incrementally calculate all paths to a specific box.
     * @param lvl The level for acquiring additional needed data.
     * @param maxDepth Maximum number of nodes of the search tree to expand at a time.
     *
     * @details
     * The algorithm performs a greedy breadth-first search, searching for all paths that lead to
     * #required_box.  While searching, @arg maxDepth limits the number of nodes expanded, so it may take multiple
     * calls to actually calculate the full path.  Until a full path is found, the nodes partially retain the old
     * paths from a previous search.
     */
    void updatePath(const engine::Engine& engine);

    void searchPath(const engine::Engine& engine);

    YAML::Node save(const Engine& engine) const;

    void load(const YAML::Node& n, const Engine& engine);
};

struct AiInfo
{
    loader::file::ZoneId zone_number;

    loader::file::ZoneId enemy_zone;
    bool enemy_unreachable = false;

    core::Area distance{0};

    bool ahead;

    bool bite;

    core::Angle angle;

    core::Angle enemy_facing;

    AiInfo(engine::Engine& engine, items::ItemState& item);

    bool canReachEnemyZone() const
    {
        return !enemy_unreachable && zone_number == enemy_zone;
    }
};

struct CreatureInfo
{
    core::Angle head_rotation = 0_deg;

    core::Angle neck_rotation = 0_deg;

    core::Angle maximum_turn = 1_deg;

    uint16_t flags = 0;

    Mood mood = Mood::Bored;

    LotInfo lot;

    core::TRVec target;

    CreatureInfo(const engine::Engine& engine, core::TypeId type);

    void rotateHead(const core::Angle& angle)
    {
        const auto delta = util::clamp(angle - head_rotation, -5_deg, +5_deg);
        head_rotation = util::clamp(delta + head_rotation, -90_deg, +90_deg);
    }

    static sol::usertype<CreatureInfo>& userType()
    {
        static auto type = sol::usertype<CreatureInfo>(sol::meta_function::construct,
                                                       sol::no_constructor,
                                                       "head_rotation",
                                                       &CreatureInfo::head_rotation,
                                                       "neck_rotation",
                                                       &CreatureInfo::neck_rotation,
                                                       "maximum_turn",
                                                       &CreatureInfo::maximum_turn,
                                                       "flags",
                                                       &CreatureInfo::flags,
                                                       "mood",
                                                       &CreatureInfo::mood,
                                                       "target",
                                                       &CreatureInfo::target);
        return type;
    }

    YAML::Node save(const Engine& engine) const;

    void load(const YAML::Node& n, const Engine& engine);
};

void updateMood(const engine::Engine& engine, const items::ItemState& item, const AiInfo& aiInfo, bool violent);
} // namespace ai
} // namespace engine
