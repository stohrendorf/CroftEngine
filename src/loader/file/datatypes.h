#pragma once

#include "color.h"
#include "core/containeroffset.h"
#include "core/id.h"
#include "core/magic.h"
#include "core/vec.h"
#include "engine/floordata/types.h"
#include "meshes.h"
#include "primitives.h"
#include "render/scene/node.h"
#include "texture.h"

#include <array>
#include <cstdint>
#include <memory>

/**
 * @defgroup native Native data interface
 *
 * Contains structs and constants directly interfacing the native
 * TR level data or engine internals.
 */

namespace loader::file
{
namespace level
{
class Level;
}

namespace io
{
class SDLReader;
}

constexpr uint16_t TextureIndexMaskTr4 = 0x7FFF; // in some custom levels we need to use 0x7FFF flag
constexpr uint16_t TextureIndexMask = 0x0FFF;

struct Portal
{
  core::RoomId16 adjoining_room{uint16_t(0)}; ///< \brief which room this portal leads to.
  core::TRVec normal;
  //! Vertices in world space
  std::array<core::TRVec, 4> vertices;

  static Portal read(io::SDLReader& reader, const core::TRVec& offset);
};

struct Sector
{
  /**
     * @brief Index into FloorData[]
     *
     * @note If this is 0, no floor data is attached to this sector.
     */
  core::ContainerIndex<uint16_t, engine::floordata::FloorDataValue> floorDataIndex;

  core::BoxId boxIndex{int16_t(-1)};             //!< Index into Boxes[]/Zones[] (-1 if none)
  core::RoomId8 roomIndexBelow{uint8_t(-1)};     //!< The number of the room below this one (255 if none)
  core::Length floorHeight = -core::HeightLimit; //!< Absolute height of floor (multiply by 256 for world coordinates)
  core::RoomId8 roomIndexAbove{uint8_t(-1)};     //!< The number of the room above this one (255 if none)
  core::Length ceilingHeight
    = -core::HeightLimit; //!< Absolute height of ceiling (multiply by 256 for world coordinates)

  static Sector read(io::SDLReader& reader);
};

/*
* lights
*/
// cppcheck-suppress syntaxError
enum class LightType : uint8_t
{
  Null,
  Point,
  Spotlight,
  Sun,
  Shadow
};

struct Light
{
  core::TRVec position;       // world coords
  ByteColor color;            // three bytes rgb values
  core::Intensity intensity;  // Light intensity
  core::Intensity intensity2; // Almost always equal to Intensity1 [absent from TR1 data files]
  // distance of half light intensity
  core::Length fadeDistance = 0_len;
  core::Length fade2 = 0_len; // Falloff value 2 [absent from TR1 data files]
  uint8_t light_type = 0;     // same as D3D (i.e. 2 is for spotlight)
  uint8_t unknown = 0;        // always 0xff?
  core::Length r_inner = 0_len;

  core::Length r_outer = 0_len;

  core::Length length = 0_len;

  core::Length cutoff = 0_len;

  core::TRVec dir;  // direction
  core::TRVec pos2; // world coords
  core::TRVec dir2; // direction

  [[nodiscard]] LightType getLightType() const
  {
    switch(light_type)
    {
    case 0: return LightType::Sun;
    case 1: return LightType::Point;
    case 2: return LightType::Spotlight;
    case 3: return LightType::Shadow;
    default: return LightType::Null;
    }
  }

  /** \brief reads a room light definition.
      *
      * darkness gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 and fade2 are introduced in TR2 and are set to darkness and fade1 for TR1.
      */
  static Light readTr1(io::SDLReader& reader);

  static Light readTr2(io::SDLReader& reader);

  static Light readTr3(io::SDLReader& reader);

  static Light readTr4(io::SDLReader& reader);

  static Light readTr5(io::SDLReader& reader);
};

struct SpriteInstance
{
  DECLARE_ID(VertexId, uint16_t);

  VertexId vertex{uint16_t(0)}; // offset into vertex list
  core::SpriteInstanceId id{uint16_t(0)};

  /// \brief reads a room sprite definition.
  static SpriteInstance read(io::SDLReader& reader);
};

/** \brief Room layer (TR5).
  */
struct Layer
{
  uint16_t num_vertices;
  uint16_t unknown_l1;
  uint16_t unknown_l2;
  uint16_t num_rectangles;
  uint16_t num_triangles;
  uint16_t unknown_l3;
  uint16_t unknown_l4;

  //  The following 6 floats define the bounding box for the layer
  float bounding_box_x1;
  float bounding_box_y1;
  float bounding_box_z1;
  float bounding_box_x2;
  float bounding_box_y2;
  float bounding_box_z2;
  int16_t unknown_l6a;
  int16_t unknown_l6b;
  int16_t unknown_l7a;
  int16_t unknown_l7b;
  int16_t unknown_l8a;
  int16_t unknown_l8b;

  static Layer read(io::SDLReader& reader);
};

struct RoomVertex
{
  core::TRVec position; // where this vertex lies (relative to tr2_room_info::x/z)
  core::Shade shade{};

  uint16_t attributes = 0; // A set of flags for special rendering effects [absent from TR1 data files]
  // 0x8000 something to do with water surface
  // 0x4000 under water lighting modulation and
  // movement if viewed from above water surface
  // 0x2000 water/quicksand surface movement
  // 0x0010 "normal"
  int16_t lighting2 = 0; // Almost always equal to Lighting1 [absent from TR1 data files]
  // TR5 -->
  core::TRVec normal;
  glm::vec4 color{0.0f};

  /** \brief reads a room vertex definition.
      *
      * lighting1 gets converted, so it matches the 0-32768 range introduced in TR3.
      * lighting2 is introduced in TR2 and is set to lighting1 for TR1.
      * attributes is introduced in TR2 and is set 0 for TR1.
      * All other values are introduced in TR5 and get set to appropriate values.
      */
  static RoomVertex readTr1(io::SDLReader& reader);

  static RoomVertex readTr2(io::SDLReader& reader);

  static RoomVertex readTr3(io::SDLReader& reader);

  static RoomVertex readTr4(io::SDLReader& reader);

  static RoomVertex readTr5(io::SDLReader& reader);
};

// In TR3-5, there were 5 reverb / echo effect flags for each
// room, but they were never used in PC versions - however, level
// files still contain this info, so we now can re-use these flags
// to assign reverb/echo presets to each room->
// Also, underwater environment can be considered as additional
// reverb flag, so overall amount is 6.

enum class ReverbType : uint8_t
{
  Outside,    // EFX_REVERB_PRESET_CITY
  SmallRoom,  // EFX_REVERB_PRESET_LIVINGROOM
  MediumRoom, // EFX_REVERB_PRESET_WOODEN_LONGPASSAGE
  LargeRoom,  // EFX_REVERB_PRESET_DOME_TOMB
  Pipe,       // EFX_REVERB_PRESET_PIPE_LARGE
  Water,      // EFX_REVERB_PRESET_UNDERWATER
  Sentinel
};

struct Room
{
  // Various room flags specify various room options. Mostly, they
  // specify environment type and some additional actions which should
  // be performed in such rooms.
  static constexpr uint16_t TR_ROOM_FLAG_WATER = 0x0001;

  static constexpr uint16_t TR_ROOM_FLAG_QUICKSAND = 0x0002; // Moved from 0x0080 to avoid confusion with NL.
  static constexpr uint16_t TR_ROOM_FLAG_SKYBOX = 0x0008;

  static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN1 = 0x0010;

  static constexpr uint16_t TR_ROOM_FLAG_WIND = 0x0020;

  static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN2 = 0x0040; ///< @FIXME: Find what it means!!! Always set by Dxtre3d.
  static constexpr uint16_t TR_ROOM_FLAG_NO_LENSFLARE = 0x0080; // In TR4-5. Was quicksand in TR3.
  static constexpr uint16_t TR_ROOM_FLAG_MIST = 0x0100;
  static constexpr uint16_t TR1_ROOM_FLAG_SKYBOX = 0x0100;
  static constexpr uint16_t TR_ROOM_FLAG_CAUSTICS = 0x0200;

  static constexpr uint16_t TR_ROOM_FLAG_UNKNOWN3 = 0x0400;

  static constexpr uint16_t TR_ROOM_FLAG_DAMAGE = 0x0800; ///< @FIXME: Is it really damage (D)?
  static constexpr uint16_t TR_ROOM_FLAG_POISON = 0x1000; ///< @FIXME: Is it really poison (P)?

  core::TRVec position{};
  core::Length lowestHeight{0};
  core::Length greatestHeight{0};
  std::vector<Layer> layers{};
  std::vector<RoomVertex> vertices{};
  std::vector<QuadFace> rectangles{};
  std::vector<Triangle> triangles{};
  std::vector<SpriteInstance> sprites{};
  std::vector<Portal> portals{};

  int sectorCountZ{};            // "width" of sector list
  int sectorCountX{};            // "height" of sector list
  std::vector<Sector> sectors{}; // [NumXsectors * NumZsectors] list of sectors in this room
  core::Shade ambientShade{};
  int16_t intensity2{};        // Almost always the same value as AmbientIntensity1 [absent from TR1 data files]
  int16_t lightMode{};         // (present only in TR2: 0 is normal, 1 is flickering(?), 2 and 3 are uncertain)
  std::vector<Light> lights{}; // [NumLights] list of point lights
  std::vector<RoomStaticMesh> staticMeshes{};   // [NumStaticMeshes]list of static meshes
  core::RoomIdI16 alternateRoom{int16_t(-1)};   // number of the room that this room can alternate
  core::RoomGroupId alternateGroup{uint8_t(0)}; // number of group which is used to switch alternate rooms
  // with (e.g. empty/filled with water is implemented as an empty room that alternates with a full room)

  uint16_t flags{};

  // Flag bits:
  // 0x0001 - room is filled with water,
  // 0x0020 - Lara's ponytail gets blown by the wind;
  // TR1 has only the water flag and the extra unknown flag 0x0100.
  // TR3 most likely has flags for "is raining", "is snowing", "water is cold", and "is
  // filled by quicksand", among others.

  [[nodiscard]] bool isWaterRoom() const noexcept
  {
    return (flags & TR_ROOM_FLAG_WATER) != 0;
  }

  [[nodiscard]] bool isSkybox() const noexcept
  {
    return (flags & TR1_ROOM_FLAG_SKYBOX) != 0;
  }

  uint8_t waterScheme{};

  // Water scheme is used with various room options, for example, R and M room flags in TRLE.
  // Also, it specifies lighting scheme, when 0x4000 vertex attribute is set.

  ReverbType reverbInfo{};

  // Reverb info is used in TR3-5 and contains index that specifies reverb type.
  // 0 - Outside, 1 - Small room, 2 - Medium room, 3 - Large room, 4 - Pipe.

  FloatColor lightColor{}; // Present in TR5 only

  // TR5 only:

  float room_x{};
  float room_z{};
  float room_y_bottom{};
  float room_y_top{};
  uint32_t unknown_r1{};
  uint32_t unknown_r2{};
  uint32_t unknown_r3{};
  uint16_t unknown_r4a{};
  uint16_t unknown_r4b{};
  uint32_t unknown_r5{};
  uint32_t unknown_r6{};

  /** \brief reads a room definition.
      *
      * darkness gets converted, so it matches the 0-32768 range introduced in TR3.
      * intensity2 is introduced in TR2 and is set to darkness for TR1.
      * light_mode is only in TR2 and is set 0 for TR1.
      * light_color is only in TR3-4 and gets set appropriately.
      */
  static std::unique_ptr<Room> readTr1(io::SDLReader& reader);

  static std::unique_ptr<Room> readTr2(io::SDLReader& reader);

  static std::unique_ptr<Room> readTr3(io::SDLReader& reader);

  static std::unique_ptr<Room> readTr4(io::SDLReader& reader);

  static std::unique_ptr<Room> readTr5(io::SDLReader& reader);
};

struct Sprite
{
  core::TextureId texture_id{uint16_t(0)};

  UVCoordinates uv0;
  UVCoordinates uv1;

  glm::ivec2 render0;
  glm::ivec2 render1;

  static std::unique_ptr<Sprite> readTr1(io::SDLReader& reader);
  static std::unique_ptr<Sprite> readTr4(io::SDLReader& reader);
};

struct SpriteSequence
{
  core::TypeId type{uint16_t(0)}; // Item identifier (matched in Items[])
  int16_t length = 0;             // negative of "how many sprites are in this sequence"
  uint16_t offset = 0;            // where (in sprite texture list) this sequence starts

  static std::unique_ptr<SpriteSequence> readTr1(io::SDLReader& reader);
  static std::unique_ptr<SpriteSequence> read(io::SDLReader& reader);
};

struct Box
{
  core::Length zmin = 0_len;
  core::Length zmax = 0_len;
  core::Length xmin = 0_len;
  core::Length xmax = 0_len;

  core::Length floor = 0_len;

  //! @brief Index into the overlaps list, which lists all boxes that overlap with this one.
  //! @remark Mask @c 0x8000 possibly marks boxes that are not reachable by large NPCs, like the T-Rex.
  //! @remark Mask @c 0x4000 possible marks closed doors.
  uint16_t overlap_index = 0; // index into Overlaps[]. The high bit is sometimes set; this
  // occurs in front of swinging doors and the like.
  mutable bool blocked = true;
  bool blockable = true;

  static std::unique_ptr<Box> readTr1(io::SDLReader& reader);
  static std::unique_ptr<Box> readTr2(io::SDLReader& reader);
};

using ZoneId = uint16_t;
using ZoneData = std::vector<ZoneId>;

struct Zones
{
  void read(size_t boxCount, io::SDLReader& reader);

  ZoneData groundZone1{};
  ZoneData groundZone2{};
  ZoneData flyZone{};
};

struct Camera
{
  core::TRVec position{};

  union
  {
    uint16_t room{};
    uint16_t underwaterCurrentStrength;
  };

  union
  {
    // TODO mutable flags
    mutable uint16_t flags{};
    uint16_t box_index;
  };

  void serialize(const serialization::Serializer<engine::world::World>& ser);

  static std::unique_ptr<Camera> read(io::SDLReader& reader);

  constexpr bool isActive() const noexcept
  {
    return (flags & 1u) != 0;
  }

  void setActive(const bool flg) const noexcept
  {
    if(flg)
      flags |= 1u;
    else
      flags &= ~1u;
  }
};

struct FlybyCamera
{
  int32_t cam_x{};
  int32_t cam_y{};
  int32_t cam_z{};
  int32_t target_x{};
  int32_t target_y{};
  int32_t target_z{};
  uint8_t sequence{};
  uint8_t index{};
  uint16_t fov{};
  uint16_t roll{};
  core::Frame timer{0_frame};
  uint16_t speed{};
  uint16_t flags{};
  core::RoomId32 room_id{0u};

  static std::unique_ptr<FlybyCamera> read(io::SDLReader& reader);
};

struct AIObject
{
  core::ItemId object_id{uint16_t(0)}; // the objectID from the AI object (AI_FOLLOW is 402)
  uint16_t room{};
  int32_t x{};
  int32_t y{};
  int32_t z{};
  uint16_t ocb{};
  uint16_t flags{}; // The trigger flags (button 1-5, first button has value 2)
  int32_t angle{};

  static std::unique_ptr<AIObject> read(io::SDLReader& reader);
};

struct CinematicFrame
{
  core::TRVec lookAt;
  core::TRVec position;
  core::Angle fov;
  core::Angle rotZ;

  static std::unique_ptr<CinematicFrame> read(io::SDLReader& reader);
};

struct LightMap
{
  std::array<uint8_t, 32 * 256> map;

  /// \brief reads the lightmap.
  static std::unique_ptr<LightMap> read(io::SDLReader& reader);
};
} // namespace loader::file
