#pragma once

#include "engine/engine.h"
#include "engine/items/itemnode.h"

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

    uint16_t search_revision = 0;
    bool blocked = false;

    const loader::file::Box* next_expansion = nullptr;

    YAML::Node save(const Engine& engine) const;

    void load(const YAML::Node& n, const Engine& engine);
};

struct LotInfo
{
    std::unordered_map<const loader::file::Box*, SearchNode> nodes;

    std::vector<gsl::not_null<const loader::file::Box*>> boxes;

    const loader::file::Box* head = nullptr;

    const loader::file::Box* tail = nullptr;

    uint16_t m_searchVersion = 0;

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

    bool calculateTarget(const engine::Engine& engine, core::TRVec& target, const items::ItemState& item);

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
    void updatePath(const engine::Engine& engine, const uint8_t maxDepth)
    {
        if(required_box != nullptr && required_box != target_box)
        {
            target_box = required_box;

            const auto targetNode = &nodes[target_box];
            if(targetNode->next_expansion == nullptr && tail != target_box)
            {
                targetNode->next_expansion = head;

                if(std::exchange(head, target_box) == nullptr)
                    tail = target_box;
            }

            targetNode->search_revision = ++m_searchVersion;
            targetNode->exit_box = nullptr;
        }

        Expects(target_box != nullptr);
        searchPath(engine, maxDepth);
    }

    void searchPath(const engine::Engine& engine, const uint8_t maxDepth)
    {
        if(head == nullptr)
        {
            return;
        }

        const auto zoneRef = loader::file::Box::getZoneRef(engine.roomsAreSwapped(), fly, step);
        const auto searchZone = head->*zoneRef;

        for(uint8_t i = 0; i < maxDepth; ++i)
        {
            if(head == nullptr)
            {
                return;
            }

            const auto headNode = &nodes[head];

            for(const auto overlapBoxIdx : getOverlaps(engine, head->overlap_index))
            {
                const auto* overlapBox = &engine.getBoxes().at(overlapBoxIdx & 0x7FFFu);

                if(searchZone != overlapBox->*zoneRef)
                    continue; // cannot switch zones

                const auto boxHeightDiff = overlapBox->floor - head->floor;
                if(boxHeightDiff > step || boxHeightDiff < drop)
                    continue; // can't reach from this box, but still maybe from another one

                auto overlapNode = &nodes[overlapBox];

                if(headNode->search_revision < overlapNode->search_revision)
                    continue; // not yet checked if we can reach this box

                if(headNode->blocked)
                {
                    if(headNode->search_revision == overlapNode->search_revision)
                        continue; // already visited

                    // mark as visited and blocked
                    overlapNode->search_revision = headNode->search_revision;
                    overlapNode->blocked = true;
                }
                else
                {
                    if(headNode->search_revision == overlapNode->search_revision && !overlapNode->blocked)
                        continue; // already visited and marked reachable

                    // mark as visited, and check if reachable
                    overlapNode->search_revision = headNode->search_revision;
                    overlapNode->blocked = false;
                    if(!canVisit(*overlapBox))
                        overlapNode->blocked = true; // can't reach this box
                    else
                        overlapNode->exit_box = head; // success! connect both boxes
                }

                if(overlapNode->next_expansion == nullptr && overlapBox != tail)
                    tail = nodes[tail].next_expansion = overlapBox; // enqueue for expansion
            }

            head = std::exchange(headNode->next_expansion, nullptr);
        }
    }

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

    CreatureInfo(const engine::Engine& engine, const core::TypeId type);

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
