#include "datatypes.h"

#include "engine/world.h"
#include "io/sdlreader.h"
#include "io/util.h"
#include "level/level.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/sprite.h"
#include "render/textureanimator.h"
#include "serialization/quantity.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "serialization/vector_element.h"
#include "util.h"
#include "util/helpers.h"

#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>

namespace loader::file
{
namespace
{
#pragma pack(push, 1)

struct RenderVertex
{
  glm::vec3 position{};
  glm::vec4 color{1.0f};
  glm::vec3 normal{0.0f};

  static const gl::VertexFormat<RenderVertex>& getFormat()
  {
    static const gl::VertexFormat<RenderVertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
                                                       {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
                                                       {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color}};

    return format;
  }
};

#pragma pack(pop)

struct RenderMesh
{
  using IndexType = uint16_t;
  std::vector<IndexType> m_indices;
  std::shared_ptr<render::scene::Material> m_materialFull;
  std::shared_ptr<render::scene::Material> m_materialCSMDepthOnly;
  std::shared_ptr<render::scene::Material> m_materialDepthOnly;

  std::shared_ptr<render::scene::Mesh>
    toMesh(const gsl::not_null<std::shared_ptr<gl::VertexBuffer<RenderVertex>>>& vbuf,
           const gsl::not_null<std::shared_ptr<gl::VertexBuffer<render::TextureAnimator::AnimatedUV>>>& uvBuf)
  {
#ifndef NDEBUG
    for(auto idx : m_indices)
    {
      BOOST_ASSERT(idx < vbuf->size());
    }
#endif

    auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<IndexType>>();
    indexBuffer->setData(m_indices, gl::api::BufferUsageARB::StaticDraw);

    auto vBufs = std::make_tuple(vbuf, uvBuf);

    auto mesh = std::make_shared<render::scene::MeshImpl<IndexType, RenderVertex, render::TextureAnimator::AnimatedUV>>(
      std::make_shared<gl::VertexArray<IndexType, RenderVertex, render::TextureAnimator::AnimatedUV>>(
        indexBuffer,
        vBufs,
        std::vector{&m_materialFull->getShaderProgram()->getHandle(),
                    m_materialDepthOnly == nullptr ? nullptr : &m_materialDepthOnly->getShaderProgram()->getHandle(),
                    m_materialCSMDepthOnly == nullptr ? nullptr
                                                      : &m_materialCSMDepthOnly->getShaderProgram()->getHandle()}));
    mesh->getMaterial()
      .set(render::scene::RenderMode::Full, m_materialFull)
      .set(render::scene::RenderMode::CSMDepthOnly, m_materialCSMDepthOnly)
      .set(render::scene::RenderMode::DepthOnly, m_materialDepthOnly);

    return mesh;
  }
};

template<size_t N>
core::TRVec getCenter(const std::array<VertexIndex, N>& faceVertices, const std::vector<RoomVertex>& roomVertices)
{
  static_assert(N <= static_cast<size_t>(std::numeric_limits<core::Length::type>::max()));

  core::TRVec s{0_len, 0_len, 0_len};
  for(const auto& v : faceVertices)
  {
    const auto& rv = v.from(roomVertices);
    s += rv.position;
  }

  return s / static_cast<core::Length::type>(N);
}
} // namespace

void Room::createSceneNode(const size_t roomId,
                           const level::Level& level,
                           render::TextureAnimator& animator,
                           render::scene::MaterialManager& materialManager)
{
  const auto texMask = gameToEngine(level.m_gameVersion) == loader::file::level::Engine::TR4
                         ? loader::file::TextureIndexMaskTr4
                         : loader::file::TextureIndexMask;

  RenderMesh renderMesh;
  renderMesh.m_materialDepthOnly = materialManager.getDepthOnly(false);
  renderMesh.m_materialCSMDepthOnly = nullptr;
  renderMesh.m_materialFull = materialManager.getGeometry(isWaterRoom(), false, true);

  std::vector<RenderVertex> vbufData;
  std::vector<render::TextureAnimator::AnimatedUV> uvCoordsData;

  const auto label = "Room:" + std::to_string(roomId);
  auto vbuf = std::make_shared<gl::VertexBuffer<RenderVertex>>(RenderVertex::getFormat(), label);

  static const gl::VertexFormat<render::TextureAnimator::AnimatedUV> uvAttribs{
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&render::TextureAnimator::AnimatedUV::uv}},
    {VERTEX_ATTRIBUTE_TEXINDEX_NAME, gl::VertexAttribute{&render::TextureAnimator::AnimatedUV::index}},
  };
  auto uvCoords = std::make_shared<gl::VertexBuffer<render::TextureAnimator::AnimatedUV>>(uvAttribs, label + "-uv");

  for(const QuadFace& quad : rectangles)
  {
    // discard water surface polygons
    const auto center = getCenter(quad.vertices, vertices);
    if(const auto sector = getSectorByRelativePosition(center))
    {
      if(sector->roomAbove != nullptr)
      {
        const bool planarWithPortal = center.Y + position.Y == sector->ceilingHeight;
        if(planarWithPortal && sector->roomAbove->isWaterRoom() != isWaterRoom())
          continue;
        if(planarWithPortal && sector->roomAbove->alternateRoom.get() >= 0
           && level.m_rooms.at(sector->roomAbove->alternateRoom.get()).isWaterRoom() != isWaterRoom())
          continue;
      }
      if(sector->roomBelow != nullptr)
      {
        const bool planarWithPortal = center.Y + position.Y == sector->floorHeight;
        if(planarWithPortal && sector->roomBelow->isWaterRoom() != isWaterRoom())
          continue;
        if(planarWithPortal && sector->roomBelow->alternateRoom.get() >= 0
           && level.m_rooms.at(sector->roomBelow->alternateRoom.get()).isWaterRoom() != isWaterRoom())
          continue;
      }
    }

    const TextureTile& tile = level.m_textureTiles.at(quad.tileId.get());

    const auto firstVertex = vbufData.size();
    for(int i = 0; i < 4; ++i)
    {
      RenderVertex iv;
      iv.position = quad.vertices[i].from(vertices).position.toRenderSystem();
      iv.color = quad.vertices[i].from(vertices).color;
      uvCoordsData.emplace_back(tile.textureKey.tileAndFlag & texMask, tile.uvCoordinates[i].toGl());

      if(i <= 2)
      {
        static const std::array<int, 3> indices{0, 1, 2};
        iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(vertices).position,
                                   quad.vertices[indices[(i + 1) % 3]].from(vertices).position,
                                   quad.vertices[indices[(i + 2) % 3]].from(vertices).position);
      }
      else
      {
        static const std::array<int, 3> indices{0, 2, 3};
        iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(vertices).position,
                                   quad.vertices[indices[(i + 1) % 3]].from(vertices).position,
                                   quad.vertices[indices[(i + 2) % 3]].from(vertices).position);
      }

      vbufData.emplace_back(iv);
    }

    for(int i : {0, 1, 2, 0, 2, 3})
    {
      animator.registerVertex(quad.tileId, uvCoords, i, firstVertex + i);
      renderMesh.m_indices.emplace_back(gsl::narrow<RenderMesh::IndexType>(firstVertex + i));
    }
  }
  for(const Triangle& tri : triangles)
  {
    // discard water surface polygons
    const auto center = getCenter(tri.vertices, vertices);
    if(const auto sector = getSectorByRelativePosition(center))
    {
      if(sector->roomAbove != nullptr && sector->roomAbove->isWaterRoom() != isWaterRoom())
      {
        if(center.Y + position.Y == sector->ceilingHeight)
          continue;
      }
      if(sector->roomBelow != nullptr && sector->roomBelow->isWaterRoom() != isWaterRoom())
      {
        if(center.Y + position.Y == sector->floorHeight)
          continue;
      }
    }

    const TextureTile& tile = level.m_textureTiles.at(tri.tileId.get());

    const auto firstVertex = vbufData.size();
    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv;
      iv.position = tri.vertices[i].from(vertices).position.toRenderSystem();
      iv.color = tri.vertices[i].from(vertices).color;
      uvCoordsData.emplace_back(tile.textureKey.tileAndFlag & texMask, tile.uvCoordinates[i].toGl());

      static const std::array<int, 3> indices{0, 1, 2};
      iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(vertices).position,
                                 tri.vertices[indices[(i + 1) % 3]].from(vertices).position,
                                 tri.vertices[indices[(i + 2) % 3]].from(vertices).position);

      vbufData.push_back(iv);
    }

    for(int i : {0, 1, 2})
    {
      animator.registerVertex(tri.tileId, uvCoords, i, firstVertex + i);
      renderMesh.m_indices.emplace_back(gsl::narrow<RenderMesh::IndexType>(firstVertex + i));
    }
  }

  vbuf->setData(vbufData, gl::api::BufferUsageARB::StaticDraw);
  uvCoords->setData(uvCoordsData, gl::api::BufferUsageARB::DynamicDraw);

  auto resMesh = renderMesh.toMesh(vbuf, uvCoords);
  resMesh->getRenderState().setCullFace(true);
  resMesh->getRenderState().setCullFaceSide(gl::api::CullFaceMode::Back);
  resMesh->getRenderState().setBlend(false);

  node = std::make_shared<render::scene::Node>("Room:" + std::to_string(roomId));
  node->setRenderable(resMesh);
  node->addUniformSetter("u_lightAmbient",
                         [](const render::scene::Node& /*node*/, gl::Uniform& uniform) { uniform.set(1.0f); });

  node->addBufferBinder(
    "b_lights",
    [emptyBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::Lighting::Light>>("lights-buffer-empty")](
      const render::scene::Node&, gl::ShaderStorageBlock& shaderStorageBlock) {
      shaderStorageBlock.bind(*emptyBuffer);
    });

  for(const RoomStaticMesh& sm : staticMeshes)
  {
    const auto staticRenderMesh = level.findStaticRenderMeshById(sm.meshId);
    if(staticRenderMesh == nullptr)
      continue;

    auto subNode = std::make_shared<render::scene::Node>("staticMesh");
    subNode->setRenderable(staticRenderMesh);
    subNode->setLocalMatrix(translate(glm::mat4{1.0f}, (sm.position - position).toRenderSystem())
                            * rotate(glm::mat4{1.0f}, toRad(sm.rotation), glm::vec3{0, -1, 0}));

    subNode->addUniformSetter(
      "u_lightAmbient",
      [brightness = toBrightness(sm.shade)](const render::scene::Node& /*node*/, gl::Uniform& uniform) {
        uniform.set(brightness.get());
      });

    sceneryNodes.emplace_back(std::move(subNode));
  }
  node->setLocalMatrix(translate(glm::mat4{1.0f}, position.toRenderSystem()));

  for(const SpriteInstance& spriteInstance : sprites)
  {
    BOOST_ASSERT(spriteInstance.vertex.get() < vertices.size());

    const Sprite& sprite = level.m_sprites.at(spriteInstance.id.get());

    const auto mesh = render::scene::createSpriteMesh(static_cast<float>(sprite.render0.x),
                                                      static_cast<float>(-sprite.render0.y),
                                                      static_cast<float>(sprite.render1.x),
                                                      static_cast<float>(-sprite.render1.y),
                                                      sprite.uv0.toGl(),
                                                      sprite.uv1.toGl(),
                                                      materialManager.getSprite(),
                                                      sprite.texture_id.get_as<int32_t>());

    auto spriteNode = std::make_shared<render::scene::Node>("sprite");
    spriteNode->setRenderable(mesh);
    const RoomVertex& v = vertices.at(spriteInstance.vertex.get());
    spriteNode->setLocalMatrix(translate(glm::mat4{1.0f}, v.position.toRenderSystem()));
    spriteNode->addUniformSetter(
      "u_lightAmbient",
      [brightness = toBrightness(v.shade)](const render::scene::Node& /*node*/, gl::Uniform& uniform) {
        uniform.set(brightness.get());
      });
    bindSpritePole(*spriteNode, render::scene::SpritePole::Y);

    sceneryNodes.emplace_back(std::move(spriteNode));
  }
  for(auto& portal : portals)
    portal.buildMesh(materialManager.getPortal());

  resetScenery();
}

std::unique_ptr<StaticMesh> StaticMesh::read(io::SDLReader& reader)
{
  auto mesh = std::make_unique<StaticMesh>();
  mesh->id = reader.readU32();
  mesh->mesh = reader.readU16();

  mesh->visibility_box.min.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->visibility_box.max.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->visibility_box.min.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->visibility_box.max.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->visibility_box.min.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->visibility_box.max.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};

  mesh->collision_box.min.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->collision_box.max.X = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->collision_box.min.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->collision_box.max.Y = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->collision_box.min.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};
  mesh->collision_box.max.Z = core::Length{static_cast<core::Length::type>(reader.readI16())};

  mesh->flags = reader.readU16();
  return mesh;
}

void Room::patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height)
{
  auto room = object.m_state.position.room;
  // TODO Ugly const_cast
  gsl::not_null groundSector
    = const_cast<TypedSector*>(loader::file::findRealFloorSector(object.m_state.position.position, &room).get());
  const auto topSector = loader::file::findRealFloorSector(
    object.m_state.position.position + core::TRVec{0_len, height - core::SectorSize, 0_len}, &room);

  if(groundSector->floorHeight == -core::HeightLimit)
  {
    groundSector->floorHeight = topSector->ceilingHeight + height;
  }
  else
  {
    groundSector->floorHeight = topSector->floorHeight + height;
    if(groundSector->floorHeight == topSector->ceilingHeight)
      groundSector->floorHeight = -core::HeightLimit;
  }

  Expects(groundSector->box != nullptr);

  if(groundSector->box->blockable)
    groundSector->box->blocked = (height < 0_len);
}

std::unique_ptr<Room> Room::readTr1(io::SDLReader& reader)
{
  auto room = std::make_unique<Room>();

  // read and change coordinate system
  room->position.X = core::Length{reader.readI32()};
  room->position.Y = 0_len;
  room->position.Z = core::Length{reader.readI32()};
  room->lowestHeight = core::Length{reader.readI32()};
  room->greatestHeight = core::Length{reader.readI32()};

  const std::streamsize num_data_words = reader.readU32();

  const auto position = reader.tell();

  reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr1);
  reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(room->sprites, reader.readU16(), &SpriteInstance::read);

  // set to the right position in case that there is some unused data
  reader.seek(position + num_data_words * 2);

  room->portals.resize(reader.readU16());
  std::generate(
    room->portals.begin(), room->portals.end(), [&reader, &room]() { return Portal::read(reader, room->position); });

  room->sectorCountZ = reader.readU16();
  room->sectorCountX = reader.readU16();
  reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);
  room->typedSectors.resize(room->sectors.size());

  // read and make consistent
  room->ambientShade = core::Shade{reader.readI16()};
  // only in TR2-TR4
  room->intensity2 = room->ambientShade.get();
  // only in TR2
  room->lightMode = 0;

  reader.readVector(room->lights, reader.readU16(), &Light::readTr1);
  reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr1);

  room->alternateRoom = reader.readI16();
  room->alternateGroup = uint8_t(0); // Doesn't exist in TR1-3

  room->flags = reader.readU16();
  room->reverbInfo = ReverbType::MediumRoom;

  room->lightColor.r = 1.0f;
  room->lightColor.g = 1.0f;
  room->lightColor.b = 1.0f;
  room->lightColor.a = 1.0f;
  return room;
}

std::unique_ptr<Room> Room::readTr2(io::SDLReader& reader)
{
  auto room = std::make_unique<Room>();
  // read and change coordinate system
  room->position.X = core::Length{reader.readI32()};
  room->position.Y = 0_len;
  room->position.Z = core::Length{reader.readI32()};
  room->lowestHeight = core::Length{reader.readI32()};
  room->greatestHeight = core::Length{reader.readI32()};

  const std::streamsize num_data_words = reader.readU32();

  const auto position = reader.tell();

  reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr2);
  reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(room->sprites, reader.readU16(), &SpriteInstance::read);

  // set to the right position in case that there is some unused data
  reader.seek(position + num_data_words * 2);

  room->portals.resize(reader.readU16());
  for(size_t i = 0; i < room->portals.size(); i++)
    room->portals[i] = Portal::read(reader, room->position);

  room->sectorCountZ = reader.readU16();
  room->sectorCountX = reader.readU16();
  reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

  // read and make consistent
  room->ambientShade = core::Shade{gsl::narrow<core::Shade::type>((8191 - reader.readI16()) * 4)};
  room->intensity2 = (8191 - reader.readI16()) * 4;
  room->lightMode = reader.readI16();

  reader.readVector(room->lights, reader.readU16(), &Light::readTr2);
  reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr2);

  room->alternateRoom = reader.readI16();
  room->alternateGroup = uint8_t(0); // Doesn't exist in TR1-3

  room->flags = reader.readU16();

  if(room->flags & 0x0020u)
  {
    room->reverbInfo = ReverbType::Outside;
  }
  else
  {
    room->reverbInfo = ReverbType::MediumRoom;
  }

  room->lightColor.r = gsl::narrow_cast<float>(room->ambientShade.get()) / 16384.0f;
  room->lightColor.g = gsl::narrow_cast<float>(room->ambientShade.get()) / 16384.0f;
  room->lightColor.b = gsl::narrow_cast<float>(room->ambientShade.get()) / 16384.0f;
  room->lightColor.a = 1.0f;
  return room;
}

std::unique_ptr<Room> Room::readTr3(io::SDLReader& reader)
{
  auto room = std::make_unique<Room>();

  // read and change coordinate system
  room->position.X = core::Length{static_cast<core::Length::type>(reader.readI32())};
  room->position.Y = 0_len;
  room->position.Z = core::Length{static_cast<core::Length::type>(reader.readI32())};
  room->lowestHeight = core::Length{reader.readI32()};
  room->greatestHeight = core::Length{reader.readI32()};

  const std::streamsize num_data_words = reader.readU32();

  const auto position = reader.tell();

  reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr3);
  reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(room->sprites, reader.readU16(), &SpriteInstance::read);

  // set to the right position in case that there is some unused data
  reader.seek(position + num_data_words * 2);

  room->portals.resize(reader.readU16());
  for(size_t i = 0; i < room->portals.size(); i++)
    room->portals[i] = Portal::read(reader, room->position);

  room->sectorCountZ = reader.readU16();
  room->sectorCountX = reader.readU16();
  reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

  room->ambientShade = core::Shade{reader.readI16()};
  room->intensity2 = reader.readI16();

  // only in TR2
  room->lightMode = 0;

  reader.readVector(room->lights, reader.readU16(), &Light::readTr3);
  reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr3);

  room->alternateRoom = reader.readI16();
  room->alternateGroup = uint8_t(0); // Doesn't exist in TR1-3

  room->flags = reader.readU16();

  if(room->flags & 0x0080u)
  {
    room->flags |= 0x0002u; // Move quicksand flag to another bit to avoid confusion with NL flag.
    room->flags &= ~0x0080u;
  }

  // Only in TR3-5

  room->waterScheme = reader.readU8();
  room->reverbInfo = static_cast<ReverbType>(reader.readU8());

  reader.skip(1); // Alternate_group override?

  room->lightColor.r = gsl::narrow_cast<float>(room->ambientShade.get()) / 65535.0f;
  room->lightColor.g = gsl::narrow_cast<float>(room->ambientShade.get()) / 65535.0f;
  room->lightColor.b = gsl::narrow_cast<float>(room->ambientShade.get()) / 65535.0f;
  room->lightColor.a = 1.0f;
  return room;
}

std::unique_ptr<Room> Room::readTr4(io::SDLReader& reader)
{
  auto room = std::make_unique<Room>();
  // read and change coordinate system
  room->position.X = core::Length{static_cast<core::Length::type>(reader.readI32())};
  room->position.Y = 0_len;
  room->position.Z = core::Length{static_cast<core::Length::type>(reader.readI32())};
  room->lowestHeight = core::Length{reader.readI32()};
  room->greatestHeight = core::Length{reader.readI32()};

  const std::streamsize num_data_words = reader.readU32();

  const auto position = reader.tell();

  reader.readVector(room->vertices, reader.readU16(), &RoomVertex::readTr4);
  reader.readVector(room->rectangles, reader.readU16(), &QuadFace::readTr1);
  reader.readVector(room->triangles, reader.readU16(), &Triangle::readTr1);
  reader.readVector(room->sprites, reader.readU16(), &SpriteInstance::read);

  // set to the right position in case that there is some unused data
  reader.seek(position + num_data_words * 2);

  room->portals.resize(reader.readU16());
  for(size_t i = 0; i < room->portals.size(); i++)
    room->portals[i] = Portal::read(reader, room->position);

  room->sectorCountZ = reader.readU16();
  room->sectorCountX = reader.readU16();
  reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

  room->ambientShade = core::Shade{reader.readI16()};
  room->intensity2 = reader.readI16();

  // only in TR2
  room->lightMode = 0;

  reader.readVector(room->lights, reader.readU16(), &Light::readTr4);
  reader.readVector(room->staticMeshes, reader.readU16(), &RoomStaticMesh::readTr4);

  room->alternateRoom = reader.readI16();
  room->flags = reader.readU16();

  // Only in TR3-5

  room->waterScheme = reader.readU8();
  room->reverbInfo = static_cast<ReverbType>(reader.readU8());

  // Only in TR4-5

  room->alternateGroup = reader.readU8();

  room->lightColor.r = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room->intensity2), 0, 8)) / 255.0f;
  room->lightColor.g
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room->ambientShade.get()), 8, 8)) / 255.0f;
  room->lightColor.b
    = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room->ambientShade.get()), 0, 8)) / 255.0f;
  room->lightColor.a = gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room->intensity2), 8, 8)) / 255.0f;
  return room;
}

std::unique_ptr<Room> Room::readTr5(io::SDLReader& reader)
{
  if(reader.readU32() != 0x414C4558)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: 'XELA' not found";

  const std::streamsize room_data_size = reader.readU32();
  const std::streampos position = reader.tell();
  const std::streampos endPos = position + room_data_size;

  auto room = std::make_unique<Room>();
  room->ambientShade = core::Shade{core::Shade::type{32767}};
  room->intensity2 = 32767;
  room->lightMode = 0;

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator1 has wrong value";

  /*portal_offset = */
  reader.readI32();                                           // StartPortalOffset?   // endSDOffset
  const std::streampos sector_data_offset = reader.readU32(); // StartSDOffset
  auto temp = reader.readU32();
  if(temp != 0 && temp != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator2 has wrong value";

  const std::streampos static_meshes_offset = reader.readU32(); // endPortalOffset
  // static_meshes_offset or room_layer_offset
  // read and change coordinate system
  room->position.X = core::Length{reader.readI32()};
  room->position.Y = core::Length{reader.readI32()};
  room->position.Z = core::Length{reader.readI32()};
  room->lowestHeight = core::Length{reader.readI32()};
  room->greatestHeight = core::Length{reader.readI32()};

  room->sectorCountZ = reader.readU16();
  room->sectorCountX = reader.readU16();

  room->lightColor.b = gsl::narrow_cast<float>(reader.readU8()) / 255.0f;
  room->lightColor.g = gsl::narrow_cast<float>(reader.readU8()) / 255.0f;
  room->lightColor.r = gsl::narrow_cast<float>(reader.readU8()) / 255.0f;
  room->lightColor.a = gsl::narrow_cast<float>(reader.readU8()) / 255.0f;
  //room->light_color.a = 1.0f;

  room->lights.resize(reader.readU16());
  if(room->lights.size() > 512)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: lights.size() > 512";

  room->staticMeshes.resize(reader.readU16());
  if(room->staticMeshes.size() > 512)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: static_meshes.size() > 512";

  room->reverbInfo = static_cast<ReverbType>(reader.readU8());
  room->alternateGroup = reader.readU8();
  room->waterScheme = gsl::narrow<uint8_t>(reader.readU16());

  if(reader.readU32() != 0x00007FFF)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler1 has wrong value";

  if(reader.readU32() != 0x00007FFF)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: filler2 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator4 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator5 has wrong value";

  if(reader.readU32() != 0xFFFFFFFF)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator6 has wrong value";

  room->alternateRoom = reader.readI16();

  room->flags = reader.readU16();

  room->unknown_r1 = reader.readU32();
  room->unknown_r2 = reader.readU32();
  room->unknown_r3 = reader.readU32();

  temp = reader.readU32();
  if(temp != 0 && temp != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator7 has wrong value";

  room->unknown_r4a = reader.readU16();
  room->unknown_r4b = reader.readU16();

  room->room_x = reader.readF();
  room->unknown_r5 = reader.readU32();
  room->room_z = -reader.readF();

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator8 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator9 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator10 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator11 has wrong value";

  temp = reader.readU32();
  if(temp != 0 && temp != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator12 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator13 has wrong value";

  auto num_triangles = reader.readU32();
  if(num_triangles == 0xCDCDCDCD)
    num_triangles = 0;
  if(num_triangles > 512)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: triangles.size() > 512";
  room->triangles.resize(num_triangles);

  auto num_rectangles = reader.readU32();
  if(num_rectangles == 0xCDCDCDCD)
    num_rectangles = 0;
  if(num_rectangles > 1024)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: rectangles.size() > 1024";
  room->rectangles.resize(num_rectangles);

  if(reader.readU32() != 0)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator14 has wrong value";

  /*light_size = */
  reader.readU32();
  const auto numL2 = reader.readU32();
  if(numL2 != room->lights.size())
    BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: numLights2 != lights.size()"));

  room->unknown_r6 = reader.readU32();
  room->room_y_top = -reader.readF();
  room->room_y_bottom = -reader.readF();

  room->layers.resize(reader.readU32());

  const std::streampos layer_offset = reader.readU32();
  const std::streampos vertices_offset = reader.readU32();
  const std::streampos poly_offset = reader.readU32();
  const std::streampos poly_offset2 = reader.readU32();
  if(poly_offset != poly_offset2)
    BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: poly_offset != poly_offset2"));

  const auto vertices_size = reader.readU32();
  if(vertices_size % 28 != 0)
    BOOST_THROW_EXCEPTION(std::runtime_error("TR5 Room: vertices_size has wrong value"));

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator15 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator16 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator17 has wrong value";

  if(reader.readU32() != 0xCDCDCDCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room: separator18 has wrong value";

  std::generate(room->lights.begin(), room->lights.end(), [&reader]() { return Light::readTr5(reader); });

  reader.seek(position + std::streamoff(208) + sector_data_offset);

  reader.readVector(room->sectors, room->sectorCountZ * room->sectorCountX, &Sector::read);

  room->portals.resize(reader.readI16());
  for(size_t i = 0; i < room->portals.size(); i++)
    room->portals[i] = Portal::read(reader, room->position);

  reader.seek(position + std::streamoff(208) + static_meshes_offset);

  std::generate(
    room->staticMeshes.begin(), room->staticMeshes.end(), [&reader]() { return RoomStaticMesh::readTr4(reader); });

  reader.seek(position + std::streamoff(208) + layer_offset);

  std::generate(room->layers.begin(), room->layers.end(), [&reader]() { return Layer::read(reader); });

  reader.seek(position + std::streamoff(208) + poly_offset);

  {
    VertexIndex::index_type vertex_index = 0;
    uint32_t rectangle_index = 0;
    uint32_t triangle_index = 0;

    for(size_t i = 0; i < room->layers.size(); i++)
    {
      uint32_t j;

      for(j = 0; j < room->layers[i].num_rectangles; j++)
      {
        room->rectangles[rectangle_index] = QuadFace::readTr4(reader);
        room->rectangles[rectangle_index].vertices[0] += vertex_index;
        room->rectangles[rectangle_index].vertices[1] += vertex_index;
        room->rectangles[rectangle_index].vertices[2] += vertex_index;
        room->rectangles[rectangle_index].vertices[3] += vertex_index;
        rectangle_index++;
      }
      for(j = 0; j < room->layers[i].num_triangles; j++)
      {
        room->triangles[triangle_index] = Triangle::readTr4(reader);
        room->triangles[triangle_index].vertices[0] += vertex_index;
        room->triangles[triangle_index].vertices[1] += vertex_index;
        room->triangles[triangle_index].vertices[2] += vertex_index;
        triangle_index++;
      }
      vertex_index += room->layers[i].num_vertices;
    }
  }

  reader.seek(position + std::streamoff(208) + vertices_offset);

  {
    uint32_t vertex_index = 0;
    room->vertices.resize(vertices_size / 28);
    //int temp1 = room_data_size - (208 + vertices_offset + vertices_size);
    for(size_t i = 0; i < room->layers.size(); i++)
    {
      for(uint16_t j = 0; j < room->layers[i].num_vertices; j++)
        room->vertices[vertex_index++] = RoomVertex::readTr5(reader);
    }
  }

  reader.seek(endPos);

  return room;
}

std::optional<core::Length> Room::getWaterSurfaceHeight(const core::RoomBoundPosition& pos)
{
  auto sector = pos.room->getSectorByAbsolutePosition(pos.position);

  if(pos.room->isWaterRoom())
  {
    while(sector->roomAbove != nullptr)
    {
      if(!sector->roomAbove->isWaterRoom())
        return sector->ceilingHeight;

      sector = sector->roomAbove->getSectorByAbsolutePosition(pos.position);
    }
  }
  else
  {
    while(sector->roomBelow != nullptr)
    {
      if(sector->roomBelow->isWaterRoom())
        return sector->floorHeight;

      sector = sector->roomBelow->getSectorByAbsolutePosition(pos.position);
    }
  }

  return std::nullopt;
}

void Room::resetScenery()
{
  node->removeAllChildren();
  for(const auto& subNode : sceneryNodes)
  {
    addChild(node, subNode);
  }
}

void Room::serialize(const serialization::Serializer<engine::World>& ser)
{
  ser(S_NV("sectors", serialization::FrozenVector{typedSectors}));
}

gsl::not_null<const TypedSector*> findRealFloorSector(const core::TRVec& position,
                                                      const gsl::not_null<gsl::not_null<const Room*>*>& room)
{
  const TypedSector* sector;
  // follow portals
  while(true)
  {
    sector = (*room)->findFloorSectorWithClampedIndex((position.X - (*room)->position.X) / core::SectorSize,
                                                      (position.Z - (*room)->position.Z) / core::SectorSize);
    if(sector->portalTarget == nullptr)
    {
      break;
    }

    *room = sector->portalTarget;
  }

  // go up/down until we are in the room that contains our coordinates
  Expects(sector != nullptr);
  if(position.Y >= sector->floorHeight)
  {
    while(position.Y >= sector->floorHeight)
    {
      if(sector->roomBelow == nullptr)
        break;
      *room = sector->roomBelow;
      sector = (*room)->getSectorByAbsolutePosition(position);
      Expects(sector != nullptr);
    }
  }
  else
  {
    while(position.Y < sector->ceilingHeight && sector->roomAbove != nullptr)
    {
      if(sector->roomAbove == nullptr)
        break;
      *room = sector->roomAbove;
      sector = (*room)->getSectorByAbsolutePosition(position);
      Expects(sector != nullptr);
    }
  }

  return sector;
}

void Camera::serialize(const serialization::Serializer<engine::World>& ser)
{
  ser(S_NV("flags", flags));
}

std::unique_ptr<Camera> Camera::read(io::SDLReader& reader)
{
  auto camera = std::make_unique<Camera>();
  camera->position = readCoordinates32(reader);

  camera->room = reader.readU16();
  camera->flags = reader.readU16();
  return camera;
}

void Portal::buildMesh(const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
{
  struct Vertex
  {
    glm::vec3 pos;
  };

  std::array<Vertex, 4> glVertices{};
  for(size_t i = 0; i < 4; ++i)
    glVertices[i].pos = vertices[i].toRenderSystem();

  gl::VertexFormat<Vertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos}};
  auto vb = std::make_shared<gl::VertexBuffer<Vertex>>(format);
  vb->setData(&glVertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto vao = std::make_shared<gl::VertexArray<uint16_t, Vertex>>(
    indexBuffer, vb, std::vector{&material->getShaderProgram()->getHandle()});
  mesh = std::make_shared<render::scene::MeshImpl<uint16_t, Vertex>>(vao);
  mesh->getMaterial().set(render::scene::RenderMode::DepthOnly, material);
}

Portal Portal::read(io::SDLReader& reader, const core::TRVec& offset)
{
  Portal portal;
  portal.adjoining_room = reader.readU16();
  portal.normal = readCoordinates16(reader);
  portal.vertices[0] = readCoordinates16(reader) + offset;
  portal.vertices[1] = readCoordinates16(reader) + offset;
  portal.vertices[2] = readCoordinates16(reader) + offset;
  portal.vertices[3] = readCoordinates16(reader) + offset;
  return portal;
}

Sector Sector::read(io::SDLReader& reader)
{
  Sector sector;
  sector.floorDataIndex = reader.readU16();
  sector.boxIndex = reader.readI16();
  sector.roomIndexBelow = reader.readU8();
  sector.floorHeight = core::QuarterSectorSize * static_cast<core::Length::type>(reader.readI8());
  sector.roomIndexAbove = reader.readU8();
  sector.ceilingHeight = core::QuarterSectorSize * static_cast<core::Length::type>(reader.readI8());
  return sector;
}

void TypedSector::serialize(const serialization::Serializer<engine::World>& ser)
{
  ser(S_NVVE("box", ser.context.getBoxes(), box),
      S_NV("floorHeight", floorHeight),
      S_NV("ceilingHeight", ceilingHeight),
      S_NV("roomIndexBelow", m_roomIndexBelow),
      S_NV("roomIndexAbove", m_roomIndexAbove),
      S_NVVE("portalTarget", ser.context.getLevel().m_rooms, portalTarget),
      S_NVVE("floorData", ser.context.getLevel().m_floorData, floorData));

  if(ser.loading)
  {
    ser.lazy([this](const serialization::Serializer<engine::World>& ser) { connect(ser.context.getRooms()); });
  }
}

TypedSector::TypedSector(const Sector& src,
                         std::vector<Room>& rooms,
                         const std::vector<Box>& boxes,
                         const engine::floordata::FloorData& newFloorData)
    : box{src.boxIndex.get() >= 0 ? &boxes.at(src.boxIndex.get()) : nullptr}
    , floorHeight{src.floorHeight}
    , ceilingHeight{src.ceilingHeight}
    , m_roomIndexBelow{src.roomIndexBelow}
    , m_roomIndexAbove{src.roomIndexAbove}
{
  connect(rooms);

  if(src.floorDataIndex.index != 0)
  {
    floorData = &src.floorDataIndex.from(newFloorData);

    if(const auto newPortalTarget = engine::floordata::getPortalTarget(floorData); newPortalTarget.has_value())
    {
      portalTarget = &rooms.at(*newPortalTarget);
    }
  }
}

void TypedSector::connect(std::vector<Room>& rooms)
{
  if(m_roomIndexBelow.get() != 0xff)
  {
    roomBelow = &rooms.at(m_roomIndexBelow.get());
  }
  else
  {
    roomBelow = nullptr;
  }

  if(m_roomIndexAbove.get() != 0xff)
  {
    roomAbove = &rooms.at(m_roomIndexAbove.get());
  }
  else
  {
    roomAbove = nullptr;
  }
}

Light Light::readTr1(io::SDLReader& reader)
{
  Light light;
  light.position = readCoordinates32(reader);
  light.intensity = core::Intensity{reader.readI16()};
  light.fadeDistance = core::Length{reader.readI32()};
  return light;
}

Light Light::readTr2(io::SDLReader& reader)
{
  Light light;
  light.position = readCoordinates32(reader);
  light.intensity = core::Intensity{reader.readI16()};
  light.intensity2 = core::Intensity{reader.readI16()};
  light.fadeDistance = core::Length{reader.readI32()};
  light.fade2 = core::Length{reader.readI32()};

  light.r_outer = light.fadeDistance;
  light.r_inner = light.fadeDistance / 2;

  light.light_type = 1; // Point light

  // all white
  light.color.r = 0xff;
  light.color.g = 0xff;
  light.color.b = 0xff;
  return light;
}

Light Light::readTr3(io::SDLReader& reader)
{
  Light light;
  light.position = readCoordinates32(reader);
  light.color.r = reader.readU8();
  light.color.g = reader.readU8();
  light.color.b = reader.readU8();
  light.color.a = reader.readU8();
  light.fadeDistance = core::Length{reader.readI32()};
  light.fade2 = core::Length{reader.readI32()};

  light.r_outer = light.fadeDistance;
  light.r_inner = light.fadeDistance / 2;

  light.light_type = 1; // Point light
  return light;
}

Light Light::readTr4(io::SDLReader& reader)
{
  Light light;
  light.position = readCoordinates32(reader);
  light.color = ByteColor::readTr1(reader);
  light.light_type = reader.readU8();
  light.unknown = reader.readU8();
  light.intensity = core::Intensity{static_cast<int16_t>(static_cast<uint16_t>(reader.readU8()))};
  light.r_inner = core::Length{gsl::narrow<core::Length::type>(reader.readF())};
  light.r_outer = core::Length{gsl::narrow<core::Length::type>(reader.readF())};
  light.length = core::Length{gsl::narrow<core::Length::type>(reader.readF())};
  light.cutoff = core::Length{gsl::narrow<core::Length::type>(reader.readF())};
  light.dir = readCoordinatesF(reader);
  return light;
}

Light Light::readTr5(io::SDLReader& reader)
{
  Light light;
  light.position = readCoordinatesF(reader);
  light.color.r = gsl::narrow<uint8_t>(reader.readF() * 255); // r
  light.color.g = gsl::narrow<uint8_t>(reader.readF() * 255); // g
  light.color.b = gsl::narrow<uint8_t>(reader.readF() * 255); // b
  light.color.a = gsl::narrow<uint8_t>(reader.readF() * 255); // a
  /*
      if ((temp != 0) && (temp != 0xCDCDCDCD))
      BOOST_THROW_EXCEPTION( TR_ReadError("read_tr5_room_light: separator1 has wrong value") );
      */
  light.r_inner = core::Length{gsl::narrow<core::Length::type>(reader.readF())};
  light.r_outer = core::Length{gsl::narrow<core::Length::type>(reader.readF())};
  reader.readF(); // rad_input
  reader.readF(); // rad_output
  reader.readF(); // range
  light.dir = readCoordinatesF(reader);
  light.pos2 = readCoordinates32(reader);
  light.dir2 = readCoordinates32(reader);
  light.light_type = reader.readU8();

  auto temp = reader.readU8();
  if(temp != 0xCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: separator2 has wrong value";

  temp = reader.readU8();
  if(temp != 0xCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: separator3 has wrong value";

  temp = reader.readU8();
  if(temp != 0xCD)
    BOOST_LOG_TRIVIAL(warning) << "TR5 Room Light: separator4 has wrong value";

  return light;
}

Layer Layer::read(io::SDLReader& reader)
{
  Layer layer{};
  layer.num_vertices = reader.readU16();
  layer.unknown_l1 = reader.readU16();
  layer.unknown_l2 = reader.readU16();
  layer.num_rectangles = reader.readU16();
  layer.num_triangles = reader.readU16();
  layer.unknown_l3 = reader.readU16();
  layer.unknown_l4 = reader.readU16();
  if(reader.readU16() != 0)
    BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler2 has wrong value";

  layer.bounding_box_x1 = reader.readF();
  layer.bounding_box_y1 = -reader.readF();
  layer.bounding_box_z1 = -reader.readF();
  layer.bounding_box_x2 = reader.readF();
  layer.bounding_box_y2 = -reader.readF();
  layer.bounding_box_z2 = -reader.readF();
  if(reader.readU32() != 0)
    BOOST_LOG_TRIVIAL(warning) << "Room Layer: filler3 has wrong value";

  layer.unknown_l6a = reader.readI16();
  layer.unknown_l6b = reader.readI16();
  layer.unknown_l7a = reader.readI16();
  layer.unknown_l7b = reader.readI16();
  layer.unknown_l8a = reader.readI16();
  layer.unknown_l8b = reader.readI16();
  return layer;
}

RoomVertex RoomVertex::readTr1(io::SDLReader& reader)
{
  RoomVertex room_vertex;
  room_vertex.position = readCoordinates16(reader);
  // read and make consistent
  room_vertex.shade = core::Shade{reader.readI16()};
  // only in TR2
  room_vertex.lighting2 = room_vertex.shade.get();
  room_vertex.attributes = 0;
  // only in TR5
  room_vertex.normal = {0_len, 0_len, 0_len};
  const auto f = toBrightness(room_vertex.shade).get();
  room_vertex.color = {f, f, f, 1};
  return room_vertex;
}

RoomVertex RoomVertex::readTr2(io::SDLReader& reader)
{
  RoomVertex room_vertex;
  room_vertex.position = readCoordinates16(reader);
  // read and make consistent
  room_vertex.shade = core::Shade{gsl::narrow<core::Shade::type>((8191 - reader.readI16()) * 4)};
  room_vertex.attributes = reader.readU16();
  room_vertex.lighting2 = (8191 - reader.readI16()) * 4;
  // only in TR5
  room_vertex.normal = {0_len, 0_len, 0_len};
  const auto f = gsl::narrow_cast<float>(room_vertex.lighting2) / 32768.0f;
  room_vertex.color = {f, f, f, 1};
  return room_vertex;
}

RoomVertex RoomVertex::readTr3(io::SDLReader& reader)
{
  RoomVertex room_vertex;
  room_vertex.position = readCoordinates16(reader);
  // read and make consistent
  room_vertex.shade = core::Shade{reader.readI16()};
  room_vertex.attributes = reader.readU16();
  room_vertex.lighting2 = reader.readI16();
  // only in TR5
  room_vertex.normal = {0_len, 0_len, 0_len};
  room_vertex.color = {gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_vertex.lighting2), 10, 5)) / 62.0f,
                       gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_vertex.lighting2), 5, 5)) / 62.0f,
                       gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_vertex.lighting2), 0, 5)) / 62.0f,
                       1};
  return room_vertex;
}

RoomVertex RoomVertex::readTr4(io::SDLReader& reader)
{
  RoomVertex room_vertex;
  room_vertex.position = readCoordinates16(reader);
  // read and make consistent
  room_vertex.shade = core::Shade{reader.readI16()};
  room_vertex.attributes = reader.readU16();
  room_vertex.lighting2 = reader.readI16();
  // only in TR5
  room_vertex.normal = {0_len, 0_len, 0_len};

  room_vertex.color = {gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_vertex.lighting2), 10, 5)) / 31.0f,
                       gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_vertex.lighting2), 5, 5)) / 31.0f,
                       gsl::narrow_cast<float>(util::bits(static_cast<uint16_t>(room_vertex.lighting2), 0, 5)) / 31.0f,
                       1};
  return room_vertex;
}

RoomVertex RoomVertex::readTr5(io::SDLReader& reader)
{
  RoomVertex vert;
  vert.position = readCoordinatesF(reader);
  vert.normal = readCoordinatesF(reader);
  const auto b = reader.readU8();
  const auto g = reader.readU8();
  const auto r = reader.readU8();
  const auto a = reader.readU8();
  vert.color = {r, g, b, a};
  return vert;
}

std::unique_ptr<Sprite> Sprite::readTr1(io::SDLReader& reader)
{
  auto sprite = std::make_unique<Sprite>();

  sprite->texture_id = reader.readU16();
  if(sprite->texture_id.get() > 64)
  {
    BOOST_LOG_TRIVIAL(warning) << "TR1 Sprite Texture ID > 64";
  }

  sprite->uv0.x = UVCoordinates::Component{gsl::narrow_cast<UVCoordinates::Component::type>(reader.readU8() * 256)};
  sprite->uv0.y = UVCoordinates::Component{gsl::narrow_cast<UVCoordinates::Component::type>(reader.readU8() * 256)};
  auto uvSize = UVCoordinates::read(reader);
  sprite->uv1.x = sprite->uv0.x + uvSize.x;
  sprite->uv1.y = sprite->uv0.y + uvSize.y;

  sprite->render0.x = reader.readI16();
  sprite->render0.y = reader.readI16();
  sprite->render1.x = reader.readI16();
  sprite->render1.y = reader.readI16();

  return sprite;
}

std::unique_ptr<Sprite> Sprite::readTr4(io::SDLReader& reader)
{
  auto sprite = std::make_unique<Sprite>();
  sprite->texture_id = reader.readU16();
  if(sprite->texture_id.get() > 128)
  {
    BOOST_LOG_TRIVIAL(warning) << "TR4 Sprite Texture ID > 128";
  }

  sprite->render0.x = reader.readI8() * 256;
  sprite->render0.y = reader.readI8() * 256;
  sprite->render1.x = sprite->render0.x + reader.readI16();
  sprite->render1.y = sprite->render0.y + reader.readI16();
  sprite->uv0 = UVCoordinates::read(reader);
  sprite->uv1 = UVCoordinates::read(reader);

  return sprite;
}

std::unique_ptr<SpriteSequence> SpriteSequence::readTr1(io::SDLReader& reader)
{
  auto sprite_sequence = std::make_unique<SpriteSequence>();
  sprite_sequence->type = static_cast<core::TypeId::type>(reader.readU32());
  sprite_sequence->length = reader.readI16();
  sprite_sequence->offset = reader.readU16();

  if(sprite_sequence->type.get() >= 191 /*Plant1*/)
  {
    sprite_sequence->length = 0;
  }

  BOOST_ASSERT(sprite_sequence->length <= 0);

  return sprite_sequence;
}

std::unique_ptr<SpriteSequence> SpriteSequence::read(io::SDLReader& reader)
{
  auto sprite_sequence = std::make_unique<SpriteSequence>();
  sprite_sequence->type = static_cast<core::TypeId::type>(reader.readU32());
  sprite_sequence->length = reader.readI16();
  sprite_sequence->offset = reader.readU16();

  BOOST_ASSERT(sprite_sequence->length <= 0);

  return sprite_sequence;
}

std::unique_ptr<Box> Box::readTr1(io::SDLReader& reader)
{
  auto box = std::make_unique<Box>();
  box->zmin = 1_len * reader.readI32();
  box->zmax = 1_len * reader.readI32();
  box->xmin = 1_len * reader.readI32();
  box->xmax = 1_len * reader.readI32();
  box->floor = 1_len * static_cast<core::Length::type>(reader.readI16());
  const auto tmp = reader.readU16();
  box->overlap_index = tmp & ((1u << 14u) - 1u);
  box->blocked = (tmp & 0x4000u) != 0;
  box->blockable = (tmp & 0x8000u) != 0;

  Expects(box->xmax - box->xmin + 1_len >= core::SectorSize);
  Expects(box->zmax - box->zmin + 1_len >= core::SectorSize);

  return box;
}

std::unique_ptr<Box> Box::readTr2(io::SDLReader& reader)
{
  auto box = std::make_unique<Box>();
  box->zmin = core::SectorSize * static_cast<core::Length::type>(reader.readI8());
  box->zmax = core::SectorSize * static_cast<core::Length::type>(reader.readI8());
  box->xmin = core::SectorSize * static_cast<core::Length::type>(reader.readI8());
  box->xmax = core::SectorSize * static_cast<core::Length::type>(reader.readI8());
  box->floor = core::Length{static_cast<core::Length::type>(reader.readI16())};
  const auto tmp = reader.readU16();
  box->overlap_index = tmp & ((1u << 14u) - 1u);
  box->blocked = (tmp & 0x4000u) != 0;
  box->blockable = (tmp & 0x8000u) != 0;
  return box;
}

void Box::serialize(const serialization::Serializer<engine::World>& ser)
{
  ser(S_NV("blocked", blocked), S_NV("blockable", blockable));
}

std::unique_ptr<FlybyCamera> FlybyCamera::read(io::SDLReader& reader)
{
  auto camera = std::make_unique<FlybyCamera>();
  camera->cam_x = reader.readI32();
  camera->cam_y = reader.readI32();
  camera->cam_z = reader.readI32();
  camera->target_x = reader.readI32();
  camera->target_y = reader.readI32();
  camera->target_z = reader.readI32();

  camera->sequence = reader.readI8();
  camera->index = reader.readI8();

  camera->fov = reader.readU16();
  camera->roll = reader.readU16();
  camera->timer = core::Frame{static_cast<core::Frame::type>(reader.readU16())};
  camera->speed = reader.readU16();
  camera->flags = reader.readU16();

  camera->room_id = reader.readU32();
  return camera;
}

std::unique_ptr<AIObject> AIObject::read(io::SDLReader& reader)
{
  auto object = std::make_unique<AIObject>();
  object->object_id = reader.readU16();
  object->room = reader.readU16(); // 4

  object->x = reader.readI32();
  object->y = reader.readI32();
  object->z = reader.readI32(); // 16

  object->ocb = reader.readU16();
  object->flags = reader.readU16(); // 20
  object->angle = reader.readI32(); // 24
  return object;
}

std::unique_ptr<CinematicFrame> CinematicFrame::read(io::SDLReader& reader)
{
  auto cf = std::make_unique<CinematicFrame>();
  cf->lookAt = readCoordinates16(reader);
  cf->position = readCoordinates16(reader);
  cf->fov = core::auToAngle(reader.readI16());
  cf->rotZ = core::auToAngle(reader.readI16());
  return cf;
}

std::unique_ptr<LightMap> LightMap::read(io::SDLReader& reader)
{
  auto lightmap = std::make_unique<LightMap>();
  reader.readBytes(lightmap->map.data(), lightmap->map.size());
  return lightmap;
}

void Zones::read(const size_t boxCount, io::SDLReader& reader)
{
  reader.readVector(groundZone1, boxCount);
  reader.readVector(groundZone2, boxCount);
  reader.readVector(flyZone, boxCount);
}

SpriteInstance SpriteInstance::read(io::SDLReader& reader)
{
  SpriteInstance room_sprite;
  room_sprite.vertex = reader.readU16();
  room_sprite.id = reader.readU16();
  return room_sprite;
}
} // namespace loader::file
