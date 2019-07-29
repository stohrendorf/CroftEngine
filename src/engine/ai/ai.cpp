#include "ai.h"

#include "engine/laranode.h"
#include "engine/script/reflection.h"

namespace engine
{
namespace ai
{
namespace
{
const char* toString(const Mood m)
{
    switch(m)
    {
    case Mood::Bored: return "Bored";
    case Mood::Attack: return "Attack";
    case Mood::Escape: return "Escape";
    case Mood::Stalk: return "Stalk";
    default: BOOST_THROW_EXCEPTION(std::domain_error("Invalid Mood"));
    }
}

Mood parseMood(const std::string& s)
{
    if(s == "Bored")
        return Mood::Bored;
    if(s == "Attack")
        return Mood::Attack;
    if(s == "Escape")
        return Mood::Escape;
    if(s == "Stalk")
        return Mood::Stalk;
    BOOST_THROW_EXCEPTION(std::domain_error("Invalid Mood"));
}
} // namespace

void updateMood(const Engine& engine, const items::ItemState& item, const AiInfo& aiInfo, const bool violent)
{
    if(item.creatureInfo == nullptr)
        return;

    CreatureInfo& creatureInfo = *item.creatureInfo;
    if(!creatureInfo.pathFinder.nodes[item.box].traversable && creatureInfo.pathFinder.visited.count(item.box) != 0)
    {
        creatureInfo.pathFinder.required_box = nullptr;
    }

    if(creatureInfo.mood != Mood::Attack && creatureInfo.pathFinder.required_box != nullptr
       && !item.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *creatureInfo.pathFinder.target_box))
    {
        if(aiInfo.canReachEnemyZone())
        {
            creatureInfo.mood = Mood::Bored;
        }
        creatureInfo.pathFinder.required_box = nullptr;
    }
    const auto originalMood = creatureInfo.mood;
    if(engine.getLara().m_state.health <= 0_hp)
    {
        creatureInfo.mood = Mood::Bored;
    }
    else if(violent)
    {
        switch(creatureInfo.mood)
        {
        case Mood::Bored:
        case Mood::Stalk:
            if(aiInfo.canReachEnemyZone())
            {
                creatureInfo.mood = Mood::Attack;
            }
            else if(item.is_hit)
            {
                creatureInfo.mood = Mood::Escape;
            }
            break;
        case Mood::Attack:
            if(!aiInfo.canReachEnemyZone())
            {
                creatureInfo.mood = Mood::Bored;
            }
            break;
        case Mood::Escape:
            if(aiInfo.canReachEnemyZone())
            {
                creatureInfo.mood = Mood::Attack;
            }
            break;
        }
    }
    else
    {
        switch(creatureInfo.mood)
        {
        case Mood::Bored:
        case Mood::Stalk:
            if(item.is_hit && (util::rand15() < 2048 || !aiInfo.canReachEnemyZone()))
            {
                creatureInfo.mood = Mood::Escape;
            }
            else if(aiInfo.canReachEnemyZone())
            {
                if(aiInfo.distance >= util::square(3 * core::SectorSize)
                   && (creatureInfo.mood != Mood::Stalk || creatureInfo.pathFinder.required_box != nullptr))
                {
                    creatureInfo.mood = Mood::Stalk;
                }
                else
                {
                    creatureInfo.mood = Mood::Attack;
                }
            }
            break;
        case Mood::Attack:
            if(item.is_hit && (util::rand15() < 2048 || !aiInfo.canReachEnemyZone()))
            {
                creatureInfo.mood = Mood::Escape;
            }
            else if(!aiInfo.canReachEnemyZone())
            {
                creatureInfo.mood = Mood::Bored;
            }
            break;
        case Mood::Escape:
            if(aiInfo.canReachEnemyZone() && util::rand15() < 256)
            {
                creatureInfo.mood = Mood::Stalk;
            }
            break;
        }
    }

    if(originalMood != creatureInfo.mood)
    {
        if(originalMood == Mood::Attack)
        {
            Expects(creatureInfo.pathFinder.target_box != nullptr);
            creatureInfo.pathFinder.setRandomSearchTarget(creatureInfo.pathFinder.target_box);
        }
        creatureInfo.pathFinder.required_box = nullptr;
    }

    switch(creatureInfo.mood)
    {
    case Mood::Attack:
        if(util::rand15()
           >= engine.getScriptEngine()["getObjectInfo"].call<script::ObjectInfo>(item.type.get()).target_update_chance)
            break;

        creatureInfo.pathFinder.target = engine.getLara().m_state.position.position;
        creatureInfo.pathFinder.required_box = engine.getLara().m_state.box;
        if(creatureInfo.pathFinder.fly != 0_len && engine.getLara().isOnLand())
            creatureInfo.pathFinder.target.Y += engine.getLara()
                                             .getSkeleton()
                                             ->getInterpolationInfo(engine.getLara().m_state)
                                             .getNearestFrame()
                                             ->bbox.toBBox()
                                             .minY;

        break;
    case Mood::Bored:
    {
        const auto box = creatureInfo.pathFinder.boxes[util::rand15(creatureInfo.pathFinder.boxes.size())];
        if(!item.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *box))
            break;

        if(item.stalkBox(engine, *box))
        {
            creatureInfo.pathFinder.setRandomSearchTarget(box);
            creatureInfo.mood = Mood::Stalk;
        }
        else if(creatureInfo.pathFinder.required_box == nullptr)
        {
            creatureInfo.pathFinder.setRandomSearchTarget(box);
        }
        break;
    }
    case Mood::Stalk:
    {
        if(creatureInfo.pathFinder.required_box != nullptr && item.stalkBox(engine, *creatureInfo.pathFinder.required_box))
            break;

        const auto box = creatureInfo.pathFinder.boxes[util::rand15(creatureInfo.pathFinder.boxes.size())];
        if(!item.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *box))
            break;

        if(item.stalkBox(engine, *box))
        {
            creatureInfo.pathFinder.setRandomSearchTarget(box);
        }
        else if(creatureInfo.pathFinder.required_box == nullptr)
        {
            creatureInfo.pathFinder.setRandomSearchTarget(box);
            if(!aiInfo.canReachEnemyZone())
            {
                creatureInfo.mood = Mood::Bored;
            }
        }
        break;
    }
    case Mood::Escape:
    {
        const auto box = creatureInfo.pathFinder.boxes[util::rand15(creatureInfo.pathFinder.boxes.size())];
        if(!item.isInsideZoneButNotInBox(engine, aiInfo.zone_number, *box) || creatureInfo.pathFinder.required_box != nullptr)
            break;

        if(item.inSameQuadrantAsBoxRelativeToLara(engine, *box))
        {
            creatureInfo.pathFinder.setRandomSearchTarget(box);
        }
        else if(aiInfo.canReachEnemyZone() && item.stalkBox(engine, *box))
        {
            creatureInfo.pathFinder.setRandomSearchTarget(box);
            creatureInfo.mood = Mood::Stalk;
        }
        break;
    }
    }

    if(creatureInfo.pathFinder.target_box == nullptr)
    {
        Expects(item.box != nullptr);
        creatureInfo.pathFinder.setRandomSearchTarget(item.box);
    }
    creatureInfo.pathFinder.calculateTarget(engine, creatureInfo.target, item);
}

AiInfo::AiInfo(Engine& engine, items::ItemState& item)
{
    if(item.creatureInfo == nullptr)
        return;

    const auto zoneRef = loader::file::Box::getZoneRef(
        engine.roomsAreSwapped(), item.creatureInfo->pathFinder.fly, item.creatureInfo->pathFinder.step);

    item.box = item.getCurrentSector()->box;
    zone_number = item.box->*zoneRef;
    engine.getLara().m_state.box = engine.getLara().m_state.getCurrentSector()->box;
    enemy_zone = engine.getLara().m_state.box->*zoneRef;
    enemy_unreachable = (!item.creatureInfo->pathFinder.canVisit(*engine.getLara().m_state.box)
                         || (!item.creatureInfo->pathFinder.nodes[item.box].traversable
                             && item.creatureInfo->pathFinder.visited.count(item.box) != 0));

    auto objectInfo = engine.getScriptEngine()["getObjectInfo"].call<script::ObjectInfo>(item.type.get());
    const core::Length pivotLength{objectInfo.pivot_length};
    const auto d = engine.getLara().m_state.position.position
                   - (item.position.position + util::pitch(pivotLength, item.rotation.Y));
    const auto pivotAngle = angleFromAtan(d.X, d.Z);
    distance = util::square(d.X) + util::square(d.Z);
    angle = pivotAngle - item.rotation.Y;
    enemy_facing = pivotAngle - 180_deg - engine.getLara().m_state.rotation.Y;
    ahead = angle > -90_deg && angle < 90_deg;
    bite = false;
    if(ahead)
    {
        const auto laraY = engine.getLara().m_state.position.position.Y;
        if(item.position.position.Y - core::QuarterSectorSize < laraY
           && item.position.position.Y + core::QuarterSectorSize > laraY)
        {
            bite = true;
        }
    }
}

CreatureInfo::CreatureInfo(const Engine& engine, const core::TypeId type)
    : pathFinder{engine}
{
    switch(type.get_as<TR1ItemId>())
    {
    case TR1ItemId::Wolf:
    case TR1ItemId::LionMale:
    case TR1ItemId::LionFemale:
    case TR1ItemId::Panther: pathFinder.drop = -core::SectorSize; break;

    case TR1ItemId::Bat:
    case TR1ItemId::CrocodileInWater:
    case TR1ItemId::Fish:
        pathFinder.step = 20 * core::SectorSize;
        pathFinder.drop = -20 * core::SectorSize;
        pathFinder.fly = 16_len;
        break;

    case TR1ItemId::Gorilla:
        pathFinder.step = core::SectorSize / 2;
        pathFinder.drop = -core::SectorSize;
        break;

    case TR1ItemId::TRex:
    case TR1ItemId::Mutant:
    case TR1ItemId::CentaurMutant:
        pathFinder.cannotVisitBlockable = true;
        pathFinder.cannotVisitBlocked = false;
        break;

    default:
        // silence compiler
        break;
    }
}

YAML::Node CreatureInfo::save(const Engine& engine) const
{
    YAML::Node node;
    node["headRot"] = head_rotation;
    node["neckRot"] = neck_rotation;
    node["maxTurn"] = maximum_turn;
    node["flags"] = flags;
    node["mood"] = toString(mood);
    node["pathFinder"] = pathFinder.save(engine);
    node["target"] = target.save();
    return node;
}

void CreatureInfo::load(const YAML::Node& n, const Engine& engine)
{
    head_rotation = n["headRot"].as<core::Angle>();
    neck_rotation = n["neckRot"].as<core::Angle>();
    maximum_turn = n["maxTurn"].as<core::Angle>();
    flags = n["flags"].as<uint16_t>();
    mood = parseMood(n["mood"].as<std::string>());
    pathFinder.load(n["pathFinder"], engine);
    target.load(n["target"]);
}
} // namespace ai
} // namespace engine
