#pragma once

#include "animation/skeletalmodel.h"
#include "animation/texture.h"
#include "audio/engine.h"
#include "bordered_texture_atlas.h"
#include "camera.h"
#include "core/sprite.h"
#include "object.h"

#include <boost/optional.hpp>

#include <map>
#include <memory>
#include <vector>

class btCollisionShape;
class btRigidBody;
enum class MenuItemType;

namespace world
{
class Character;
struct StaticMesh;
struct InventoryItem;

namespace core
{
struct SpriteBuffer;
struct Light;
} // namespace core

// Action type specifies a kind of action which trigger performs. Mostly
// it's only related to item activation, as any other trigger operations
// are not affected by action type in original engines.
enum class ActionType
{
    Normal,
    Anti,
    Switch,
    Bypass  //!< Used for "dummy" triggers from originals.
};

// Activator specifies a kind of triggering event (NOT to be confused
// with activator type mentioned below) to occur, like ordinary trigger,
// triggering by inserting a key, turning a switch or picking up item.
enum class ActivatorType
{
    Normal,
    Switch,
    Key,
    Pickup
};

// Activator type is used to identify activator kind for specific
// trigger types (so-called HEAVY triggers). HEAVY means that trigger
// is activated by some other item, rather than Lara herself.

#define TR_ACTIVATORTYPE_LARA 0
#define TR_ACTIVATORTYPE_MISC 1

//Room light mode flags (TR2 ONLY)

#define TR_ROOM_LIGHTMODE_FLICKER   0x1

// Sector flags specify various unique sector properties.
// Derived from native TR floordata functions.

#define SECTOR_FLAG_CLIMB_NORTH     0x00000001  // subfunction 0x01
#define SECTOR_FLAG_CLIMB_EAST      0x00000002  // subfunction 0x02
#define SECTOR_FLAG_CLIMB_SOUTH     0x00000004  // subfunction 0x04
#define SECTOR_FLAG_CLIMB_WEST      0x00000008  // subfunction 0x08
#define SECTOR_FLAG_CLIMB_CEILING   0x00000010
#define SECTOR_FLAG_MINECART_LEFT   0x00000020
#define SECTOR_FLAG_MINECART_RIGHT  0x00000040
#define SECTOR_FLAG_TRIGGERER_MARK  0x00000080
#define SECTOR_FLAG_BEETLE_MARK     0x00000100
#define SECTOR_FLAG_DEATH           0x00000200

// Sector material specifies audio response from character footsteps, as well as
// footstep texture option, plus possible vehicle physics difference in the future.

#define SECTOR_MATERIAL_MUD         0
#define SECTOR_MATERIAL_SNOW        1
#define SECTOR_MATERIAL_SAND        2
#define SECTOR_MATERIAL_GRAVEL      3
#define SECTOR_MATERIAL_ICE         4
#define SECTOR_MATERIAL_WATER       5
#define SECTOR_MATERIAL_STONE       6   // Classic one, TR1-2.
#define SECTOR_MATERIAL_WOOD        7
#define SECTOR_MATERIAL_METAL       8
#define SECTOR_MATERIAL_MARBLE      9
#define SECTOR_MATERIAL_GRASS       10
#define SECTOR_MATERIAL_CONCRETE    11
#define SECTOR_MATERIAL_OLDWOOD     12
#define SECTOR_MATERIAL_OLDMETAL    13

// Maximum number of flipmaps specifies how many flipmap indices to store. Usually,
// TR1-3 doesn't contain flipmaps above 10, while in TR4-5 number of flipmaps could
// be as much as 14-16. To make sure flipmap array will be suitable for all game
// versions, it is set to 32.

#define FLIPMAP_MAX_NUMBER          32

// Activation mask operation can be either XOR (for switch triggers) or OR (for any
// other types of triggers).

#define AMASK_OP_OR  0
#define AMASK_OP_XOR 1

class Room;
class Camera;
struct Portal;
class Render;
class Entity;

namespace core
{
class BaseMesh;
class Frustum;
struct Polygon;
} // namespace core

namespace animation
{
class Skeleton;
class SkeletalModel;
} // namespace animation

struct RoomBox
{
    int32_t     x_min;
    int32_t     x_max;
    int32_t     y_min;
    int32_t     y_max;
    int32_t     true_floor;
    int32_t     overlap_index;
};

// Tween is a short word for "inbeTWEEN vertical polygon", which is needed to fill
// the gap between two sectors with different heights. If adjacent sector heights are
// similar, it means that tween is degenerated (doesn't exist physically) - in that
// case we use NONE type. If only one of two heights' pairs is similar, then tween is
// either right or left pointed triangle (where "left" or "right" is derived by viewing
// triangle from front side). If none of the heights are similar, we need quad tween.
enum class TweenType
{
    None,          //!< Degenerated vertical polygon.
    TriangleRight, //!< Triangle pointing right (viewed front).
    TriangleLeft,  //!< Triangle pointing left (viewed front).
    Quad,
    TwoTriangles   //!< it looks like a butterfly
};

struct SectorTween
{
    irr::core::vector3df floor_corners[4];
    TweenType floor_tween_type = TweenType::None;

    irr::core::vector3df ceiling_corners[4];
    TweenType ceiling_tween_type = TweenType::None;

    void setCeilingConfig()
    {
        if(ceiling_corners[0].Z > ceiling_corners[1].Z)
        {
            std::swap(ceiling_corners[0].Z, ceiling_corners[1].Z);
            std::swap(ceiling_corners[2].Z, ceiling_corners[3].Z);
        }

        if(ceiling_corners[3].Z > ceiling_corners[2].Z)
        {
            ceiling_tween_type = TweenType::TwoTriangles;            // like a butterfly
        }
        else if(ceiling_corners[0].Z != ceiling_corners[1].Z &&
                ceiling_corners[2].Z != ceiling_corners[3].Z)
        {
            ceiling_tween_type = TweenType::Quad;
        }
        else if(ceiling_corners[0].Z != ceiling_corners[1].Z)
        {
            ceiling_tween_type = TweenType::TriangleLeft;
        }
        else if(ceiling_corners[2].Z != ceiling_corners[3].Z)
        {
            ceiling_tween_type = TweenType::TriangleRight;
        }
        else
        {
            ceiling_tween_type = TweenType::None;
        }
    }

    void setFloorConfig()
    {
        if(floor_corners[0].Z > floor_corners[1].Z)
        {
            std::swap(floor_corners[0].Z, floor_corners[1].Z);
            std::swap(floor_corners[2].Z, floor_corners[3].Z);
        }

        if(floor_corners[3].Z > floor_corners[2].Z)
        {
            floor_tween_type = TweenType::TwoTriangles;              // like a butterfly
        }
        else if(floor_corners[0].Z != floor_corners[1].Z &&
                floor_corners[2].Z != floor_corners[3].Z)
        {
            floor_tween_type = TweenType::Quad;
        }
        else if(floor_corners[0].Z != floor_corners[1].Z)
        {
            floor_tween_type = TweenType::TriangleLeft;
        }
        else if(floor_corners[2].Z != floor_corners[3].Z)
        {
            floor_tween_type = TweenType::TriangleRight;
        }
        else
        {
            floor_tween_type = TweenType::None;
        }
    }
};

struct RoomSprite
{
    core::Sprite* sprite = nullptr;
    irr::core::vector3df pos{ 0,0,0 };
    mutable bool was_rendered = false;
};

class World
{
    TRACK_LIFETIME();

public:
    explicit World(engine::Engine* engine);

    engine::Engine* m_engine;

    loader::Engine m_engineVersion = loader::Engine::Unknown;

    std::vector< std::shared_ptr<Room> > m_rooms;

    std::vector<RoomBox> m_roomBoxes;

    struct FlipInfo
    {
        uint8_t map = 0; // Flipped room activity
        uint8_t state = 0; // Flipped room state
    };

    std::vector<FlipInfo> m_flipData;

    std::unique_ptr<BorderedTextureAtlas> m_textureAtlas;
    std::vector<GLuint> m_textures;               // OpenGL textures indexes

    std::vector<animation::TextureAnimationSequence> m_textureAnimations;         // Animated textures

    std::vector<std::shared_ptr<core::BaseMesh>> m_meshes;                 // Base meshes data

    std::vector<core::Sprite> m_sprites;                // Base sprites data

    std::map<animation::ModelId, std::shared_ptr<animation::SkeletalModel>> m_skeletalModels;

    std::shared_ptr<Character> m_character;              // this is an unique Lara's pointer =)
    std::shared_ptr<animation::SkeletalModel> m_skyBox = nullptr;                // global skybox

    std::map<ObjectId, std::shared_ptr<Entity>> m_entities;            // tree of world active objects
    ObjectId m_nextEntityId = 0;
    std::map<ObjectId, std::shared_ptr<InventoryItem>> m_items;

    std::vector<StatCameraSink> m_camerasAndSinks;

    std::vector<int16_t> m_animCommands;

    void updateAnimTextures();
    irr::video::SColorf calculateWaterTint() const;

    void addEntity(std::shared_ptr<Entity> entity);
    bool createInventoryItem(ObjectId item_id, animation::ModelId model_id, animation::ModelId world_model_id, MenuItemType type, size_t count, const std::string &name);
    int deleteItem(ObjectId item_id);
    core::Sprite* getSpriteByID(core::SpriteId ID);
    std::shared_ptr<animation::SkeletalModel> getModelByID(animation::ModelId id);           // binary search the model by ID

    void prepare();
    void empty();

    std::shared_ptr<Entity> spawnEntity(animation::ModelId model_id, ObjectId room_id, const irr::core::vector3df* pos, const irr::core::vector3df* ang, boost::optional<ObjectId> id);
    bool deleteEntity(ObjectId id);

    std::shared_ptr<Entity> getEntityByID(ObjectId id) const;
    std::shared_ptr<Character> getCharacterByID(ObjectId id);

    std::shared_ptr<InventoryItem> getBaseItemByID(ObjectId id);
    std::shared_ptr<Room> findRoomByPosition(const irr::core::vector3df& pos) const;
    std::shared_ptr<Room> getByID(ObjectId ID);

    Room* Room_FindPosCogerrence(const irr::core::vector3df& new_pos, Room* room) const;

    const Room* Room_FindPosCogerrence(const irr::core::vector3df& new_pos, const Room *room) const
    {
        return Room_FindPosCogerrence(new_pos, const_cast<Room*>(room));
    }
};

} // namespace world
