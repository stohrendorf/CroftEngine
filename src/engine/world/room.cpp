#include "room.h"

#include "atlastile.h"
#include "box.h"
#include "core/angle.h"
#include "core/containeroffset.h"
#include "core/genericvec.h"
#include "core/i18n.h"
#include "core/id.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/lighting.h"
#include "engine/location.h"
#include "engine/objects/laraobject.h"
#include "engine/objects/object.h"
#include "engine/objects/objectstate.h"
#include "engine/presenter.h"
#include "loader/file/datatypes.h"
#include "loader/file/primitives.h"
#include "loader/file/texture.h"
#include "render/material/material.h"
#include "render/material/materialgroup.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/shaderprogram.h"
#include "render/rendersettings.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/node.h"
#include "render/textureanimator.h"
#include "sector.h"
#include "serialization/serialization.h"
#include "serialization/vector.h"
#include "serialization/vector_element.h"
#include "sprite.h"
#include "staticmesh.h"
#include "util.h"
#include "world.h"

#include <boost/assert.hpp>
#include <boost/log/trivial.hpp>
#include <cstdint>
#include <exception>
#include <gl/buffer.h>
#include <gl/program.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gslu.h>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <set>
#include <string>
#include <tuple>
#include <utility>

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
  glm::vec4 reflective{};

  static const gl::VertexLayout<RenderVertex>& getLayout()
  {
    static const gl::VertexLayout<RenderVertex> layout{
      {VERTEX_ATTRIBUTE_POSITION_NAME, &RenderVertex::position},
      {VERTEX_ATTRIBUTE_NORMAL_NAME, &RenderVertex::normal},
      {VERTEX_ATTRIBUTE_COLOR_NAME, &RenderVertex::color},
      {VERTEX_ATTRIBUTE_IS_QUAD, &RenderVertex::isQuad},
      {VERTEX_ATTRIBUTE_QUAD_VERT1, &RenderVertex::quadVert1},
      {VERTEX_ATTRIBUTE_QUAD_VERT2, &RenderVertex::quadVert2},
      {VERTEX_ATTRIBUTE_QUAD_VERT3, &RenderVertex::quadVert3},
      {VERTEX_ATTRIBUTE_QUAD_VERT4, &RenderVertex::quadVert4},
      {VERTEX_ATTRIBUTE_REFLECTIVE_NAME, &RenderVertex::reflective},
    };

    return layout;
  }
};

#pragma pack(pop)

struct RenderMesh
{
  using IndexType = uint16_t;
  std::vector<IndexType> m_indices;
  std::shared_ptr<render::material::Material> m_materialFull;
  std::shared_ptr<render::material::Material> m_materialCSMDepthOnly;
  std::shared_ptr<render::material::Material> m_materialDepthOnly;

  std::shared_ptr<render::scene::Mesh> toMesh(const gslu::nn_shared<gl::VertexBuffer<RenderVertex>>& vbuf,
                                              const gslu::nn_shared<gl::VertexBuffer<render::AnimatedUV>>& uvBuf,
                                              const std::string& label)
  {
#ifndef NDEBUG
    for(auto idx : m_indices)
    {
      BOOST_ASSERT(idx < vbuf->size());
    }
#endif

    auto indexBuffer
      = gsl::make_shared<gl::ElementArrayBuffer<IndexType>>(label, gl::api::BufferUsage::StaticDraw, m_indices);

    auto vBufs = std::make_tuple(vbuf, uvBuf);

    auto mesh = std::make_shared<render::scene::MeshImpl<IndexType, RenderVertex, render::AnimatedUV>>(
      gsl::make_shared<gl::VertexArray<IndexType, RenderVertex, render::AnimatedUV>>(
        indexBuffer,
        vBufs,
        std::vector{&m_materialFull->getShaderProgram()->getHandle(),
                    m_materialDepthOnly == nullptr ? nullptr : &m_materialDepthOnly->getShaderProgram()->getHandle(),
                    m_materialCSMDepthOnly == nullptr ? nullptr
                                                      : &m_materialCSMDepthOnly->getShaderProgram()->getHandle()},
        label));
    mesh->getMaterialGroup()
      .set(render::material::RenderMode::Full, m_materialFull)
      .set(render::material::RenderMode::CSMDepthOnly, m_materialCSMDepthOnly)
      .set(render::material::RenderMode::DepthOnly, m_materialDepthOnly);

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
                       const gslu::nn_shared<render::material::Material>& material)
{
  struct Vertex
  {
    glm::vec3 pos;
  };

  static const constexpr float PortalOffset = 8;

  std::array<Vertex, 4> glVertices{};
  const auto offset = glm::normalize(srcPortal.normal.toRenderSystem()) * PortalOffset;
  for(size_t i = 0; i < 4; ++i)
    glVertices[i].pos = srcPortal.vertices[i].toRenderSystem() - offset;

  gl::VertexLayout<Vertex> layout{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos}};
  auto vb = gsl::make_shared<gl::VertexBuffer<Vertex>>(layout, "portal", gl::api::BufferUsage::StaticDraw, glVertices);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer
    = gsl::make_shared<gl::ElementArrayBuffer<uint16_t>>("portal", gl::api::BufferUsage::StaticDraw, indices);

  auto vao = gsl::make_shared<gl::VertexArray<uint16_t, Vertex>>(
    indexBuffer, vb, std::vector{&material->getShaderProgram()->getHandle()}, "portal");
  mesh = std::make_shared<render::scene::MeshImpl<uint16_t, Vertex>>(vao);
  mesh->getMaterialGroup().set(render::material::RenderMode::DepthOnly, material);
}

void Room::createSceneNode(const loader::file::Room& srcRoom,
                           const size_t roomId,
                           World& world,
                           const std::vector<uint16_t>& textureAnimData,
                           render::material::MaterialManager& materialManager)
{
  RenderMesh renderMesh;
  renderMesh.m_materialDepthOnly = materialManager.getDepthOnly(false,
                                                                []()
                                                                {
                                                                  return false;
                                                                });
  renderMesh.m_materialCSMDepthOnly = nullptr;
  renderMesh.m_materialFull = materialManager.getGeometry(
    isWaterRoom,
    false,
    true,
    []()
    {
      return false;
    },
    [&world]()
    {
      const auto& settings = world.getEngine().getEngineConfig()->renderSettings;
      return !settings.lightingModeActive ? 0 : settings.lightingMode;
    });

  std::vector<RenderVertex> vbufData;
  std::vector<render::AnimatedUV> uvCoordsData;

  textureAnimator = std::make_unique<render::TextureAnimator>(textureAnimData);

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

      uvCoordsData.emplace_back(tile.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask,
                                tile.uvCoordinates[i],
                                glm::vec4{tile.uvCoordinates[0], tile.uvCoordinates[1]},
                                glm::vec4{tile.uvCoordinates[2], tile.uvCoordinates[3]});
      if(useQuadHandling)
      {
        iv.isQuad = 1;
        iv.quadVert1 = quad.vertices[0].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadVert2 = quad.vertices[1].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadVert3 = quad.vertices[2].from(srcRoom.vertices).position.toRenderSystem();
        iv.quadVert4 = quad.vertices[3].from(srcRoom.vertices).position.toRenderSystem();
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
      renderMesh.m_indices.emplace_back(gsl::narrow<RenderMesh::IndexType>(firstVertex + i));
    }
    for(int i : {0, 1, 2, 3})
    {
      textureAnimator->registerVertex(quad.tileId, i, firstVertex + i);
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
      uvCoordsData.emplace_back(tile.textureKey.atlasIdAndFlag & loader::file::AtlasIdMask,
                                tile.uvCoordinates[i],
                                glm::vec4{tile.uvCoordinates[0], tile.uvCoordinates[1]},
                                glm::vec4{tile.uvCoordinates[2], tile.uvCoordinates[3]});

      static const std::array<int, 3> indices{0, 1, 2};
      iv.normal = generateNormal(tri.vertices[indices[(i + 0) % 3]].from(srcRoom.vertices).position,
                                 tri.vertices[indices[(i + 1) % 3]].from(srcRoom.vertices).position,
                                 tri.vertices[indices[(i + 2) % 3]].from(srcRoom.vertices).position);

      vbufData.push_back(iv);
    }

    for(int i : {0, 1, 2})
    {
      renderMesh.m_indices.emplace_back(gsl::narrow<RenderMesh::IndexType>(firstVertex + i));
    }
    for(int i : {0, 1, 2})
    {
      textureAnimator->registerVertex(tri.tileId, i, firstVertex + i);
    }
  }

  const auto label = "Room:" + std::to_string(roomId);
  auto vbuf = gsl::make_shared<gl::VertexBuffer<RenderVertex>>(
    RenderVertex::getLayout(), label, gl::api::BufferUsage::StaticDraw, vbufData);

  static const gl::VertexLayout<render::AnimatedUV> uvAttribs{
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&render::AnimatedUV::uv}},
    {VERTEX_ATTRIBUTE_QUAD_UV12, &render::AnimatedUV::quadUv12},
    {VERTEX_ATTRIBUTE_QUAD_UV34, &render::AnimatedUV::quadUv34},
  };
  uvCoordsBuffer = std::make_shared<gl::VertexBuffer<render::AnimatedUV>>(
    uvAttribs, label + "-uv", gl::api::BufferUsage::DynamicDraw, uvCoordsData);

  auto resMesh = renderMesh.toMesh(vbuf, gsl::not_null{uvCoordsBuffer}, label);
  resMesh->getRenderState().setCullFace(true);
  resMesh->getRenderState().setCullFaceSide(gl::api::TriangleFace::Back);

  node = std::make_shared<render::scene::Node>("Room:" + std::to_string(roomId));
  node->setRenderable(resMesh);
  node->bind("u_lightAmbient",
             [](const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(1.0f);
             });
  node->bind("b_dynLights",
             [&world, emptyLightsBuffer = ShaderLight::getEmptyBuffer()](
               const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& block)
             {
               if(const auto lara = world.getObjectManager().getLaraPtr();
                  lara != nullptr && !lara->flashLightsBufferData.empty())
               {
                 block.bindRange(*lara->flashLightsBuffer, 0, lara->flashLightsBufferData.size());
               }
               else
               {
                 block.bind(*emptyLightsBuffer);
               }
             });

  node->bind("b_lights",
             [this, &world, emptyBuffer = ShaderLight::getEmptyBuffer()](const render::scene::Node*,
                                                                         const render::scene::Mesh& /*mesh*/,
                                                                         gl::ShaderStorageBlock& shaderStorageBlock)
             {
               if(world.getEngine().getEngineConfig()->renderSettings.lightingModeActive)
                 shaderStorageBlock.bind(*lightsBuffer);
               else
                 shaderStorageBlock.bind(*emptyBuffer);
             });

  for(const RoomStaticMesh& sm : staticMeshes)
  {
    if(sm.staticMesh->renderMesh == nullptr)
      continue;

    auto subNode = std::make_shared<render::scene::Node>("staticMesh");
    subNode->setRenderable(sm.staticMesh->renderMesh);
    subNode->getRenderState().setScissorTest(false);
    subNode->setLocalMatrix(translate(glm::mat4{1.0f}, (sm.position - position).toRenderSystem())
                            * rotate(glm::mat4{1.0f}, toRad(sm.rotation), glm::vec3{0, -1, 0}));

    subNode->bind("u_lightAmbient",
                  [brightness = toBrightness(ambientShade)](
                    const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                  {
                    uniform.set(brightness.get());
                  });
    subNode->bind("b_dynLights",
                  [&world, emptyLightsBuffer = ShaderLight::getEmptyBuffer()](const render::scene::Node* /*node*/,
                                                                              const render::scene::Mesh& /*mesh*/,
                                                                              gl::ShaderStorageBlock& block)
                  {
                    if(const auto lara = world.getObjectManager().getLaraPtr();
                       lara != nullptr && !lara->flashLightsBufferData.empty())
                    {
                      block.bindRange(*lara->flashLightsBuffer, 0, lara->flashLightsBufferData.size());
                    }
                    else
                    {
                      block.bind(*emptyLightsBuffer);
                    }
                  });

    subNode->bind("b_lights",
                  [this](const render::scene::Node*,
                         const render::scene::Mesh& /*mesh*/,
                         gl::ShaderStorageBlock& shaderStorageBlock)
                  {
                    shaderStorageBlock.bind(*lightsBuffer);
                  });

    sceneryNodes.emplace_back(std::move(subNode));
  }
  node->setLocalMatrix(translate(glm::mat4{1.0f}, position.toRenderSystem()));

  for(const loader::file::SpriteInstance& spriteInstance : srcRoom.sprites)
  {
    BOOST_ASSERT(spriteInstance.vertex.get() < srcRoom.vertices.size());

    const auto& sprite = world.getSprites().at(spriteInstance.id.get());

    auto spriteNode = std::make_shared<render::scene::Node>("sprite");
    spriteNode->setRenderable(sprite.yBoundMesh);
    const auto& v = srcRoom.vertices.at(spriteInstance.vertex.get());
    spriteNode->setLocalMatrix(translate(glm::mat4{1.0f}, v.position.toRenderSystem()));
    spriteNode->bind("u_lightAmbient",
                     [this, &world, brightness = toBrightness(v.shade)](
                       const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     {
                       if(world.getEngine().getEngineConfig()->renderSettings.lightingModeActive)
                         uniform.set(toBrightness(ambientShade).get() * 0);
                       else
                         uniform.set(brightness.get());
                     });
    spriteNode->bind("b_dynLights",
                     [&world, emptyLightsBuffer = ShaderLight::getEmptyBuffer()](const render::scene::Node* /*node*/,
                                                                                 const render::scene::Mesh& /*mesh*/,
                                                                                 gl::ShaderStorageBlock& block)
                     {
                       if(const auto lara = world.getObjectManager().getLaraPtr();
                          lara != nullptr && !lara->flashLightsBufferData.empty())
                       {
                         block.bindRange(*lara->flashLightsBuffer, 0, lara->flashLightsBufferData.size());
                       }
                       else
                       {
                         block.bind(*emptyLightsBuffer);
                       }
                     });
    spriteNode->bind("b_lights",
                     [emptyLightsBuffer = ShaderLight::getEmptyBuffer()](const render::scene::Node*,
                                                                         const render::scene::Mesh& /*mesh*/,
                                                                         gl::ShaderStorageBlock& shaderStorageBlock)
                     {
                       shaderStorageBlock.bind(*emptyLightsBuffer);
                     });
    spriteNode->bind(
      "b_lights",
      [this, &world, emptyLightsBuffer = ShaderLight::getEmptyBuffer()](
        const render::scene::Node*, const render::scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
      {
        if(world.getEngine().getEngineConfig()->renderSettings.lightingModeActive)
          shaderStorageBlock.bind(*lightsBuffer);
        else
          shaderStorageBlock.bind(*emptyLightsBuffer);
      });

    sceneryNodes.emplace_back(std::move(spriteNode));
  }

  std::transform(srcRoom.portals.begin(),
                 srcRoom.portals.end(),
                 std::back_inserter(portals),
                 [material = materialManager.getWaterSurface(), &world](const loader::file::Portal& portal)
                 {
                   Portal p{gsl::not_null{&world.getRooms().at(portal.adjoining_room.get())},
                            portal.normal.toRenderSystem(),
                            {portal.vertices[0].toRenderSystem(),
                             portal.vertices[1].toRenderSystem(),
                             portal.vertices[2].toRenderSystem(),
                             portal.vertices[3].toRenderSystem()},
                            nullptr};
                   p.buildMesh(portal, material);
                   return p;
                 });

  collectShaderLights(world.getEngine().getEngineConfig()->renderSettings.getLightCollectionDepth());

  for(const auto& v : srcRoom.vertices)
  {
    const auto vv = v.position.toRenderSystem();
    verticesBBoxMin = glm::min(verticesBBoxMin, vv);
    verticesBBoxMax = glm::max(verticesBBoxMax, vv);
  }

  regenerateDust(nullptr,
                 materialManager.getDustParticle(),
                 world.getEngine().getEngineConfig()->renderSettings.dustActive,
                 world.getEngine().getEngineConfig()->renderSettings.dustDensity);

  resetScenery();

  particles.setAmbient(*this);
}

void patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height)
{
  auto tmp = object.m_state.location;
  // TODO Ugly const_cast
  const auto groundSector = gsl::not_null{const_cast<Sector*>(tmp.updateRoom().get())};
  const auto topSector = tmp.moved(0_len, height - 1_sectors, 0_len).updateRoom();

  if(groundSector->floorHeight == core::InvalidHeight)
  {
    groundSector->floorHeight = topSector->ceilingHeight + height;
  }
  else
  {
    groundSector->floorHeight += height;
    if(groundSector->floorHeight == topSector->ceilingHeight)
      groundSector->floorHeight = core::InvalidHeight;
  }

  if(groundSector->box != nullptr && groundSector->box->blockable)
    groundSector->box->blocked = (height < 0_len);
}

std::optional<core::Length> getWaterSurfaceHeight(const Location& location)
{
  auto sector = location.room->getSectorByAbsolutePosition(location.position);
  gsl_Assert(sector != nullptr);

  if(location.room->isWaterRoom)
  {
    while(sector->roomAbove != nullptr)
    {
      if(!sector->roomAbove->isWaterRoom)
        return sector->ceilingHeight;

      sector = sector->roomAbove->getSectorByAbsolutePosition(location.position);
      gsl_Assert(sector != nullptr);
    }

    return sector->ceilingHeight;
  }
  else
  {
    while(sector->roomBelow != nullptr)
    {
      if(sector->roomBelow->isWaterRoom)
        return sector->floorHeight;

      sector = sector->roomBelow->getSectorByAbsolutePosition(location.position);
      gsl_Assert(sector != nullptr);
    }
  }

  return std::nullopt;
}

void Room::resetScenery()
{
  node->removeAllChildren();
  for(const auto& subNode : sceneryNodes)
  {
    addChild(gsl::not_null{node}, subNode);
  }
}

void Room::serialize(const serialization::Serializer<World>& ser) const
{
  ser(S_NV("sectors", serialization::SerializingFrozenVector{std::cref(sectors)}),
      S_NV_VECTOR_ELEMENT("alternateRoom", std::cref(ser.context.getRooms()), std::cref(alternateRoom)));
}

void Room::deserialize(const serialization::Deserializer<World>& ser)
{
  ser(S_NV("sectors", serialization::DeserializingFrozenVector{std::ref(sectors)}),
      S_NV_VECTOR_ELEMENT("alternateRoom", std::cref(ser.context.getRooms()), std::ref(alternateRoom)));
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

void Room::collectShaderLights(size_t depth)
{
  bufferLights.clear();
  if(lights.empty())
  {
    if(lightsBuffer == nullptr || lightsBuffer->size() != 0)
    {
      lightsBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>(
        "lights-buffer", gl::api::BufferUsage::StaticDraw, bufferLights);
    }
    return;
  }

  std::set<gsl::not_null<const Room*>> testRooms;
  testRooms.emplace(this);
  for(size_t i = 0; i < depth; ++i)
  {
    std::set<gsl::not_null<const Room*>> newTestRooms;
    for(const auto& room : testRooms)
    {
      newTestRooms.emplace(room);
      for(const auto& portal : room->portals)
      {
        newTestRooms.emplace(portal.adjoiningRoom);
      }
    }
    testRooms = std::move(newTestRooms);
  }

  for(const auto& room : testRooms)
  {
    // http://www-f9.ijs.si/~matevz/docs/PovRay/pov274.htm
    // 1 / ( 1 + (d/fade_distance) ^ fade_power );
    // assuming fade_power = 1, multiply numerator and denominator with fade_distance (identity transform):
    // fade_distance / ( fade_distance + d )

    for(const auto& light : room->lights)
    {
      if(light.intensity.get() <= 0)
        continue;
      bufferLights.emplace_back(ShaderLight{glm::vec4{light.position.toRenderSystem(), 0.0f},
                                            glm::vec4{toBrightness(light.intensity).get()},
                                            light.fadeDistance.get<float>()});
    }
  }

  lightsBuffer = std::make_shared<gl::ShaderStorageBuffer<engine::ShaderLight>>(
    "lights-buffer", gl::api::BufferUsage::StaticDraw, bufferLights);
}

void Room::regenerateDust(const std::shared_ptr<engine::Presenter>& presenter,
                          const gslu::nn_shared<render::material::Material>& dustMaterial,
                          bool isDustEnabled,
                          uint8_t dustDensityDivisor)
{
  if(!isDustEnabled)
  {
    dust = nullptr;
    return;
  }

  if(const auto it = dustCache.find(dustDensityDivisor); it != dustCache.end())
  {
    dust = it->second;
    return;
  }
  if(presenter != nullptr)
    presenter->drawLoadingScreen(_("Generating Dust Particles..."));
  auto dustNode = createParticleMesh(node->getName() + "/dust", dustMaterial, dustDensityDivisor);
  dustCache.emplace(dustDensityDivisor, dustNode);
  dust = dustNode;
}

std::shared_ptr<render::scene::Node> Room::createParticleMesh(
  const std::string& label, const gslu::nn_shared<render::material::Material>& dustMaterial, uint8_t dustDensityDivisor)
{
  static const constexpr auto BaseGridAxisSubdivision = 12;
  const auto resolution = (cbrt(dustDensityDivisor) / BaseGridAxisSubdivision * 1_sectors).cast<float>().get();

  std::vector<glm::vec3> vertices;
  vertices.reserve(std::lround(std::max(0.0f,
                                        ((verticesBBoxMax.x - verticesBBoxMin.x) / resolution)
                                          * ((verticesBBoxMax.y - verticesBBoxMin.y) / resolution)
                                          * ((verticesBBoxMax.z - verticesBBoxMin.z) / resolution))));
  std::vector<uint32_t> indices;
  indices.reserve(vertices.capacity());
  BOOST_LOG_TRIVIAL(debug) << "generating " << vertices.capacity() << " particles for " << label;

  for(float x = verticesBBoxMin.x + resolution / 2; x <= verticesBBoxMax.x - resolution / 2; x += resolution)
  {
    for(float y = verticesBBoxMin.y + resolution / 2; y <= verticesBBoxMax.y - resolution / 2; y += resolution)
    {
      for(float z = verticesBBoxMin.z + resolution / 2; z <= verticesBBoxMax.z - resolution / 2; z += resolution)
      {
        indices.emplace_back(gsl::narrow_cast<uint32_t>(vertices.size()));
        vertices.emplace_back(x, y, z);
      }
    }
  }

  static const gl::VertexLayout<glm::vec3> layout{
    {VERTEX_ATTRIBUTE_POSITION_NAME, gl::VertexAttribute<glm::vec3>::Single{}}};

  auto vbuf = gsl::make_shared<gl::VertexBuffer<glm::vec3>>(
    layout, label + "-particles", gl::api::BufferUsage::StaticDraw, vertices);
  auto indexBuffer = gsl::make_shared<gl::ElementArrayBuffer<uint32_t>>(
    label + "-particles", gl::api::BufferUsage::StaticDraw, indices);

  auto vao = gsl::make_shared<gl::VertexArray<uint32_t, glm::vec3>>(
    indexBuffer, vbuf, std::vector{&dustMaterial->getShaderProgram()->getHandle()}, label + "-particles");
  auto mesh = std::make_shared<render::scene::MeshImpl<uint32_t, glm::vec3>>(vao, gl::api::PrimitiveType::Points);
  mesh->getMaterialGroup().set(render::material::RenderMode::Full, dustMaterial);

  mesh->bind("u_baseColor",
             [color = isWaterRoom ? glm::vec3{0.146f, 0.485f, 0.216f} : glm::vec3{0.431f, 0.386f, 0.375f}](
               const render::scene::Node* /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(color);
             });

  auto dustNode = std::make_shared<render::scene::Node>(label + "-particles");
  dustNode->setLocalMatrix(translate(glm::mat4{1.0f}, position.toRenderSystem()));
  dustNode->setRenderable(mesh);
  dustNode->setVisible(true);

  return dustNode;
}
} // namespace engine::world
