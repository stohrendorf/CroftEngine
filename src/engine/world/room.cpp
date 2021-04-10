#include "room.h"

#include "engine/lighting.h"
#include "engine/objects/object.h"
#include "loader/file/level/level.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/shaderprogram.h"
#include "render/scene/sprite.h"
#include "render/textureanimator.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "util.h"
#include "world.h"

#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>

namespace engine::world
{
namespace
{
#pragma pack(push, 1)

struct RenderVertex
{
  glm::vec3 position{};
  glm::vec4 color{1.0f};
  glm::vec3 normal{0.0f};
  glm::int32 isQuad{0};
  glm::vec3 quadVert1{};
  glm::vec3 quadVert2{};
  glm::vec3 quadVert3{};
  glm::vec3 quadVert4{};
  glm::vec2 quadUv1{};
  glm::vec2 quadUv2{};
  glm::vec2 quadUv3{};
  glm::vec2 quadUv4{};

  static const gl::VertexFormat<RenderVertex>& getFormat()
  {
    static const gl::VertexFormat<RenderVertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
                                                       {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
                                                       {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color},
                                                       {VERTEX_ATTRIBUTE_IS_QUAD, &RenderVertex::isQuad},
                                                       {VERTEX_ATTRIBUTE_QUAD_VERT1, &RenderVertex::quadVert1},
                                                       {VERTEX_ATTRIBUTE_QUAD_VERT2, &RenderVertex::quadVert2},
                                                       {VERTEX_ATTRIBUTE_QUAD_VERT3, &RenderVertex::quadVert3},
                                                       {VERTEX_ATTRIBUTE_QUAD_VERT4, &RenderVertex::quadVert4},
                                                       {VERTEX_ATTRIBUTE_QUAD_UV1, &RenderVertex::quadUv1},
                                                       {VERTEX_ATTRIBUTE_QUAD_UV2, &RenderVertex::quadUv2},
                                                       {VERTEX_ATTRIBUTE_QUAD_UV3, &RenderVertex::quadUv3},
                                                       {VERTEX_ATTRIBUTE_QUAD_UV4, &RenderVertex::quadUv4}};

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
    mesh->getMaterialGroup()
      .set(render::scene::RenderMode::Full, m_materialFull)
      .set(render::scene::RenderMode::CSMDepthOnly, m_materialCSMDepthOnly)
      .set(render::scene::RenderMode::DepthOnly, m_materialDepthOnly);

    return mesh;
  }
};

template<size_t N>
core::TRVec getCenter(const std::array<loader::file::VertexIndex, N>& faceVertices,
                      const std::vector<loader::file::RoomVertex>& roomVertices)
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

void Portal::buildMesh(const loader::file::Portal& srcPortal,
                       const gsl::not_null<std::shared_ptr<render::scene::Material>>& material)
{
  struct Vertex
  {
    glm::vec3 pos;
  };

  std::array<Vertex, 4> glVertices{};
  for(size_t i = 0; i < 4; ++i)
    glVertices[i].pos = srcPortal.vertices[i].toRenderSystem();

  gl::VertexFormat<Vertex> format{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos}};
  auto vb = std::make_shared<gl::VertexBuffer<Vertex>>(format);
  vb->setData(&glVertices[0], 4, gl::api::BufferUsageARB::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(&indices[0], 6, gl::api::BufferUsageARB::StaticDraw);

  auto vao = std::make_shared<gl::VertexArray<uint16_t, Vertex>>(
    indexBuffer, vb, std::vector{&material->getShaderProgram()->getHandle()});
  mesh = std::make_shared<render::scene::MeshImpl<uint16_t, Vertex>>(vao);
  mesh->getMaterialGroup().set(render::scene::RenderMode::DepthOnly, material);
}

void Room::createSceneNode(const loader::file::Room& srcRoom,
                           const size_t roomId,
                           World& world,
                           render::TextureAnimator& animator,
                           render::scene::MaterialManager& materialManager)
{
  RenderMesh renderMesh;
  renderMesh.m_materialDepthOnly = materialManager.getDepthOnly(false);
  renderMesh.m_materialCSMDepthOnly = nullptr;
  renderMesh.m_materialFull = materialManager.getGeometry(isWaterRoom, false, true);

  std::vector<RenderVertex> vbufData;
  std::vector<render::TextureAnimator::AnimatedUV> uvCoordsData;

  const auto label = "Room:" + std::to_string(roomId);
  auto vbuf = std::make_shared<gl::VertexBuffer<RenderVertex>>(RenderVertex::getFormat(), label);

  static const gl::VertexFormat<render::TextureAnimator::AnimatedUV> uvAttribs{
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&render::TextureAnimator::AnimatedUV::uv}},
    {VERTEX_ATTRIBUTE_TEXINDEX_NAME, gl::VertexAttribute{&render::TextureAnimator::AnimatedUV::index}},
  };
  auto uvCoords = std::make_shared<gl::VertexBuffer<render::TextureAnimator::AnimatedUV>>(uvAttribs, label + "-uv");

  for(const loader::file::QuadFace& quad : srcRoom.rectangles)
  {
    // discard water surface polygons
    const auto center = getCenter(quad.vertices, srcRoom.vertices);
    if(const auto sector = getSectorByRelativePosition(center))
    {
      if(sector->roomAbove != nullptr)
      {
        const bool planarWithPortal = center.Y + position.Y == sector->ceilingHeight;
        if(planarWithPortal && sector->roomAbove->isWaterRoom != isWaterRoom)
          continue;
        if(planarWithPortal && sector->roomAbove->alternateRoom != nullptr
           && sector->roomAbove->alternateRoom->isWaterRoom != isWaterRoom)
          continue;
      }
      if(sector->roomBelow != nullptr)
      {
        const bool planarWithPortal = center.Y + position.Y == sector->floorHeight;
        if(planarWithPortal && sector->roomBelow->isWaterRoom != isWaterRoom)
          continue;
        if(planarWithPortal && sector->roomBelow->alternateRoom != nullptr
           && sector->roomBelow->alternateRoom->isWaterRoom != isWaterRoom)
          continue;
      }
    }

    const auto& tile = world.getAtlasTiles().at(quad.tileId.get());

    bool useQuadHandling = isDistortedQuad(quad.vertices[0].from(srcRoom.vertices).position.toRenderSystem(),
                                           quad.vertices[1].from(srcRoom.vertices).position.toRenderSystem(),
                                           quad.vertices[2].from(srcRoom.vertices).position.toRenderSystem(),
                                           quad.vertices[3].from(srcRoom.vertices).position.toRenderSystem());

    const auto firstVertex = vbufData.size();
    for(int i = 0; i < 4; ++i)
    {
      RenderVertex iv;
      iv.position = quad.vertices[i].from(srcRoom.vertices).position.toRenderSystem();
      iv.color = quad.vertices[i].from(srcRoom.vertices).color;
      uvCoordsData.emplace_back(tile.textureKey.tileAndFlag & loader::file::TextureIndexMask, tile.uvCoordinates[i]);

      if(useQuadHandling)
      {
        iv.isQuad = 1;
        iv.quadVert1 = quad.vertices[0].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadVert2 = quad.vertices[1].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadVert3 = quad.vertices[2].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadVert4 = quad.vertices[3].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadUv1 = tile.uvCoordinates[0];
        iv.quadUv2 = tile.uvCoordinates[1];
        iv.quadUv3 = tile.uvCoordinates[2];
        iv.quadUv4 = tile.uvCoordinates[3];
      }

      if(i <= 2)
      {
        static const std::array<int, 3> indices{0, 1, 2};
        iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(srcRoom.vertices).position,
                                   quad.vertices[indices[(i + 1) % 3]].from(srcRoom.vertices).position,
                                   quad.vertices[indices[(i + 2) % 3]].from(srcRoom.vertices).position);
      }
      else
      {
        static const std::array<int, 3> indices{0, 2, 3};
        iv.normal = generateNormal(quad.vertices[indices[(i + 0) % 3]].from(srcRoom.vertices).position,
                                   quad.vertices[indices[(i + 1) % 3]].from(srcRoom.vertices).position,
                                   quad.vertices[indices[(i + 2) % 3]].from(srcRoom.vertices).position);
      }

      vbufData.emplace_back(iv);
    }

    for(int i : {0, 1, 2, 0, 2, 3})
    {
      animator.registerVertex(quad.tileId, uvCoords, i, firstVertex + i);
      renderMesh.m_indices.emplace_back(gsl::narrow<RenderMesh::IndexType>(firstVertex + i));
    }
  }
  for(const loader::file::Triangle& tri : srcRoom.triangles)
  {
    // discard water surface polygons
    const auto center = getCenter(tri.vertices, srcRoom.vertices);
    if(const auto sector = getSectorByRelativePosition(center))
    {
      if(sector->roomAbove != nullptr && sector->roomAbove->isWaterRoom != isWaterRoom)
      {
        if(center.Y + position.Y == sector->ceilingHeight)
          continue;
      }
      if(sector->roomBelow != nullptr && sector->roomBelow->isWaterRoom != isWaterRoom)
      {
        if(center.Y + position.Y == sector->floorHeight)
          continue;
      }
    }

    const auto& tile = world.getAtlasTiles().at(tri.tileId.get());

    const auto firstVertex = vbufData.size();
    for(int i = 0; i < 3; ++i)
    {
      RenderVertex iv;
      iv.position = tri.vertices[i].from(srcRoom.vertices).position.toRenderSystem();
      iv.color = tri.vertices[i].from(srcRoom.vertices).color;
      uvCoordsData.emplace_back(tile.textureKey.tileAndFlag & loader::file::TextureIndexMask, tile.uvCoordinates[i]);

      static const std::array<int, 3> indices{0, 1, 2};
      iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(srcRoom.vertices).position,
                                 tri.vertices[indices[(i + 1) % 3]].from(srcRoom.vertices).position,
                                 tri.vertices[indices[(i + 2) % 3]].from(srcRoom.vertices).position);

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
  node->bind("u_lightAmbient",
             [](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform) {
               uniform.set(1.0f);
             });

  node->bind("b_lights",
             [emptyBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::Lighting::Light>>("lights-buffer-empty")](
               const render::scene::Node&,
               const render::scene::Mesh& /*mesh*/,
               gl::ShaderStorageBlock& shaderStorageBlock) { shaderStorageBlock.bind(*emptyBuffer); });

  for(const RoomStaticMesh& sm : staticMeshes)
  {
    if(sm.staticMesh->renderMesh == nullptr)
      continue;

    auto subNode = std::make_shared<render::scene::Node>("staticMesh");
    subNode->setRenderable(sm.staticMesh->renderMesh);
    subNode->setLocalMatrix(translate(glm::mat4{1.0f}, (sm.position - position).toRenderSystem())
                            * rotate(glm::mat4{1.0f}, toRad(sm.rotation), glm::vec3{0, -1, 0}));

    subNode->bind("u_lightAmbient",
                  [brightness = toBrightness(sm.shade)](const render::scene::Node& /*node*/,
                                                        const render::scene::Mesh& /*mesh*/,
                                                        gl::Uniform& uniform) { uniform.set(brightness.get()); });

    sceneryNodes.emplace_back(std::move(subNode));
  }
  node->setLocalMatrix(translate(glm::mat4{1.0f}, position.toRenderSystem()));

  for(const loader::file::SpriteInstance& spriteInstance : srcRoom.sprites)
  {
    BOOST_ASSERT(spriteInstance.vertex.get() < srcRoom.vertices.size());

    const auto& sprite = world.getSprites().at(spriteInstance.id.get());

    const auto mesh = render::scene::createSpriteMesh(static_cast<float>(sprite.render0.x),
                                                      static_cast<float>(-sprite.render0.y),
                                                      static_cast<float>(sprite.render1.x),
                                                      static_cast<float>(-sprite.render1.y),
                                                      sprite.uv0,
                                                      sprite.uv1,
                                                      materialManager.getSprite(),
                                                      sprite.textureId.get_as<int32_t>());

    auto spriteNode = std::make_shared<render::scene::Node>("sprite");
    spriteNode->setRenderable(mesh);
    const auto& v = srcRoom.vertices.at(spriteInstance.vertex.get());
    spriteNode->setLocalMatrix(translate(glm::mat4{1.0f}, v.position.toRenderSystem()));
    spriteNode->bind("u_lightAmbient",
                     [brightness = toBrightness(v.shade)](const render::scene::Node& /*node*/,
                                                          const render::scene::Mesh& /*mesh*/,
                                                          gl::Uniform& uniform) { uniform.set(brightness.get()); });
    bindSpritePole(*spriteNode, render::scene::SpritePole::Y);

    sceneryNodes.emplace_back(std::move(spriteNode));
  }

  std::transform(srcRoom.portals.begin(),
                 srcRoom.portals.end(),
                 std::back_inserter(portals),
                 [material = materialManager.getWaterSurface(), &world](const loader::file::Portal& portal) {
                   Portal p{&world.getRooms().at(portal.adjoining_room.get()),
                            portal.normal.toRenderSystem(),
                            {portal.vertices[0].toRenderSystem(),
                             portal.vertices[1].toRenderSystem(),
                             portal.vertices[2].toRenderSystem(),
                             portal.vertices[3].toRenderSystem()},
                            nullptr};
                   p.buildMesh(portal, material);
                   return p;
                 });

  resetScenery();
}

void patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height)
{
  auto room = object.m_state.position.room;
  // TODO Ugly const_cast
  gsl::not_null groundSector = const_cast<Sector*>(findRealFloorSector(object.m_state.position.position, &room).get());
  const auto topSector = findRealFloorSector(
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

std::optional<core::Length> getWaterSurfaceHeight(const core::RoomBoundPosition& pos)
{
  auto sector = pos.room->getSectorByAbsolutePosition(pos.position);

  if(pos.room->isWaterRoom)
  {
    while(sector->roomAbove != nullptr)
    {
      if(!sector->roomAbove->isWaterRoom)
        return sector->ceilingHeight;

      sector = sector->roomAbove->getSectorByAbsolutePosition(pos.position);
      Expects(sector != nullptr);
    }

    return sector->ceilingHeight;
  }
  else
  {
    while(sector->roomBelow != nullptr)
    {
      if(sector->roomBelow->isWaterRoom)
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

void Room::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("sectors", serialization::FrozenVector{sectors}));
}

const Sector* Room::getSectorByIndex(const int dx, const int dz) const
{
  if(dx < 0 || dx >= sectorCountX)
  {
    BOOST_LOG_TRIVIAL(warning) << "Sector coordinates " << dx << "/" << dz << " out of bounds " << sectorCountX << "/"
                               << sectorCountZ << " for room " << node->getName();
    return nullptr;
  }
  if(dz < 0 || dz >= sectorCountZ)
  {
    BOOST_LOG_TRIVIAL(warning) << "Sector coordinates " << dx << "/" << dz << " out of bounds " << sectorCountX << "/"
                               << sectorCountZ << " for room " << node->getName();
    return nullptr;
  }
  return &sectors[sectorCountZ * dx + dz];
}

gsl::not_null<const Sector*> findRealFloorSector(const core::TRVec& position,
                                                 const gsl::not_null<gsl::not_null<const Room*>*>& room)
{
  const Sector* sector;
  while(true)
  {
    sector = (*room)->getBoundarySectorByIndex((position.X - (*room)->position.X) / core::SectorSize,
                                               (position.Z - (*room)->position.Z) / core::SectorSize);
    if(sector->boundaryRoom == nullptr)
    {
      break;
    }

    *room = sector->boundaryRoom;
  }

  // go up/down until we are in the room that contains our coordinates
  Expects(sector != nullptr);
  if(position.Y >= sector->floorHeight)
  {
    while(position.Y >= sector->floorHeight && sector->roomBelow != nullptr)
    {
      *room = sector->roomBelow;
      sector = (*room)->getSectorByAbsolutePosition(position);
      Expects(sector != nullptr);
    }
  }
  else
  {
    while(position.Y < sector->ceilingHeight && sector->roomAbove != nullptr)
    {
      *room = sector->roomAbove;
      sector = (*room)->getSectorByAbsolutePosition(position);
      Expects(sector != nullptr);
    }
  }

  return sector;
}
} // namespace engine::world
