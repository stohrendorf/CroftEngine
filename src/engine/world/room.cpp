#include "room.h"

#include "atlastile.h"
#include "box.h"
#include "core/containeroffset.h"
#include "core/id.h"
#include "engine/engine.h"
#include "engine/engineconfig.h"
#include "engine/lighting.h"
#include "engine/location.h"
#include "engine/objects/object.h"
#include "engine/objects/objectstate.h"
#include "loader/file/datatypes.h"
#include "loader/file/primitives.h"
#include "loader/file/texture.h"
#include "render/rendersettings.h"
#include "render/scene/material.h"
#include "render/scene/materialgroup.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/node.h"
#include "render/scene/rendermode.h"
#include "render/scene/shaderprogram.h"
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
#include <glm/ext/scalar_int_sized.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <gslu.h>
#include <initializer_list>
#include <iosfwd>
#include <iterator>
#include <limits>
#include <random>
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
  glm::vec4 quadUv12{};
  glm::vec4 quadUv34{};

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
      {VERTEX_ATTRIBUTE_QUAD_UV12, &RenderVertex::quadUv12},
      {VERTEX_ATTRIBUTE_QUAD_UV34, &RenderVertex::quadUv34},
    };

    return layout;
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
           const gsl::not_null<std::shared_ptr<gl::VertexBuffer<render::TextureAnimator::AnimatedUV>>>& uvBuf,
           const std::string& label)
  {
#ifndef NDEBUG
    for(auto idx : m_indices)
    {
      BOOST_ASSERT(idx < vbuf->size());
    }
#endif

    auto indexBuffer = gslu::make_nn_shared<gl::ElementArrayBuffer<IndexType>>(label);
    indexBuffer->setData(m_indices, gl::api::BufferUsage::StaticDraw);

    auto vBufs = std::make_tuple(vbuf, uvBuf);

    auto mesh = std::make_shared<render::scene::MeshImpl<IndexType, RenderVertex, render::TextureAnimator::AnimatedUV>>(
      gslu::make_nn_shared<gl::VertexArray<IndexType, RenderVertex, render::TextureAnimator::AnimatedUV>>(
        indexBuffer,
        vBufs,
        std::vector{&m_materialFull->getShaderProgram()->getHandle(),
                    m_materialDepthOnly == nullptr ? nullptr : &m_materialDepthOnly->getShaderProgram()->getHandle(),
                    m_materialCSMDepthOnly == nullptr ? nullptr
                                                      : &m_materialCSMDepthOnly->getShaderProgram()->getHandle()},
        label));
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

  static const constexpr float PortalOffset = 8;

  std::array<Vertex, 4> glVertices{};
  const auto offset = glm::normalize(srcPortal.normal.toRenderSystem()) * PortalOffset;
  for(size_t i = 0; i < 4; ++i)
    glVertices[i].pos = srcPortal.vertices[i].toRenderSystem() - offset;

  gl::VertexLayout<Vertex> layout{{VERTEX_ATTRIBUTE_POSITION_NAME, &Vertex::pos}};
  auto vb = gslu::make_nn_shared<gl::VertexBuffer<Vertex>>(layout, "portal");
  vb->setData(glVertices, gl::api::BufferUsage::StaticDraw);

  static const std::array<uint16_t, 6> indices{0, 1, 2, 0, 2, 3};

  auto indexBuffer = gslu::make_nn_shared<gl::ElementArrayBuffer<uint16_t>>("portal");
  indexBuffer->setData(indices, gl::api::BufferUsage::StaticDraw);

  auto vao = gslu::make_nn_shared<gl::VertexArray<uint16_t, Vertex>>(
    indexBuffer, vb, std::vector{&material->getShaderProgram()->getHandle()}, "portal");
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
  auto vbuf = gslu::make_nn_shared<gl::VertexBuffer<RenderVertex>>(RenderVertex::getLayout(), label);

  static const gl::VertexLayout<render::TextureAnimator::AnimatedUV> uvAttribs{
    {VERTEX_ATTRIBUTE_TEXCOORD_PREFIX_NAME, gl::VertexAttribute{&render::TextureAnimator::AnimatedUV::uv}},
  };
  auto uvCoords = gslu::make_nn_shared<gl::VertexBuffer<render::TextureAnimator::AnimatedUV>>(uvAttribs, label + "-uv");

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
        iv.quadUv12 = glm::vec4{tile.uvCoordinates[0], tile.uvCoordinates[1]};
        iv.quadUv34 = glm::vec4{tile.uvCoordinates[2], tile.uvCoordinates[3]};
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

  vbuf->setData(vbufData, gl::api::BufferUsage::StaticDraw);
  uvCoords->setData(uvCoordsData, gl::api::BufferUsage::DynamicDraw);

  auto resMesh = renderMesh.toMesh(vbuf, uvCoords, label);
  resMesh->getRenderState().setCullFace(true);
  resMesh->getRenderState().setCullFaceSide(gl::api::CullFaceMode::Back);

  node = std::make_shared<render::scene::Node>("Room:" + std::to_string(roomId));
  node->setRenderable(resMesh);
  node->bind("u_lightAmbient",
             [](const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
             {
               uniform.set(1.0f);
             });

  node->bind("b_lights",
             [emptyBuffer = ShaderLight::getEmptyBuffer()](const render::scene::Node&,
                                                           const render::scene::Mesh& /*mesh*/,
                                                           gl::ShaderStorageBlock& shaderStorageBlock)
             {
               shaderStorageBlock.bind(*emptyBuffer);
             });

  for(const RoomStaticMesh& sm : staticMeshes)
  {
    if(sm.staticMesh->renderMesh == nullptr)
      continue;

    auto subNode = std::make_shared<render::scene::Node>("staticMesh");
    subNode->setRenderable(sm.staticMesh->renderMesh);
    subNode->setLocalMatrix(translate(glm::mat4{1.0f}, (sm.position - position).toRenderSystem())
                            * rotate(glm::mat4{1.0f}, toRad(sm.rotation), glm::vec3{0, -1, 0}));

    subNode->bind("u_lightAmbient",
                  [brightness = toBrightness(ambientShade)](
                    const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                  {
                    uniform.set(brightness.get());
                  });

    subNode->bind("b_lights",
                  [this](const render::scene::Node&,
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
                     [brightness = toBrightness(v.shade)](
                       const render::scene::Node& /*node*/, const render::scene::Mesh& /*mesh*/, gl::Uniform& uniform)
                     {
                       uniform.set(brightness.get());
                     });
    spriteNode->bind("b_lights",
                     [emptyLightsBuffer = ShaderLight::getEmptyBuffer()](const render::scene::Node&,
                                                                         const render::scene::Mesh& /*mesh*/,
                                                                         gl::ShaderStorageBlock& shaderStorageBlock)
                     {
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

  {
    glm::vec3 min{std::numeric_limits<float>::max()};
    glm::vec3 max{std::numeric_limits<float>::lowest()};
    for(const auto& v : srcRoom.vertices)
    {
      const auto vv = v.position.toRenderSystem();
      min = glm::min(min, vv);
      max = glm::max(max, vv);
    }
    createParticleMesh(label, min, max, materialManager);
  }

  resetScenery();
}

void patchHeightsForBlock(const engine::objects::Object& object, const core::Length& height)
{
  auto tmp = object.m_state.location;
  // TODO Ugly const_cast
  const auto groundSector = gsl::not_null{const_cast<Sector*>(tmp.updateRoom().get())};
  const auto topSector = tmp.moved(0_len, height - core::SectorSize, 0_len).updateRoom();

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

  Expects(groundSector->box != nullptr);

  if(groundSector->box->blockable)
    groundSector->box->blocked = (height < 0_len);
}

std::optional<core::Length> getWaterSurfaceHeight(const Location& location)
{
  auto sector = location.room->getSectorByAbsolutePosition(location.position);
  Expects(sector != nullptr);

  if(location.room->isWaterRoom)
  {
    while(sector->roomAbove != nullptr)
    {
      if(!sector->roomAbove->isWaterRoom)
        return sector->ceilingHeight;

      sector = sector->roomAbove->getSectorByAbsolutePosition(location.position);
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

      sector = sector->roomBelow->getSectorByAbsolutePosition(location.position);
      Expects(sector != nullptr);
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

void Room::serialize(const serialization::Serializer<World>& ser)
{
  ser(S_NV("sectors", serialization::FrozenVector{sectors}),
      S_NV_VECTOR_ELEMENT("alternateRoom", ser.context.getRooms(), alternateRoom));
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
    lightsBuffer->setData(bufferLights, gl::api::BufferUsage::StaticDraw);
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
                                            toBrightness(light.intensity).get(),
                                            light.fadeDistance.get<float>()});
    }
  }

  lightsBuffer->setData(bufferLights, gl::api::BufferUsage::StaticDraw);
}

void Room::createParticleMesh(const std::string& label,
                              const glm::vec3& min,
                              const glm::vec3& max,
                              render::scene::MaterialManager& materialManager)
{
  std::vector<glm::vec3> vertices;
  std::vector<uint32_t> indices;

  static const constexpr auto Resolution = (core::SectorSize / 4).cast<float>().get();
  std::uniform_real_distribution<float> rdist{-Resolution / 2, Resolution / 2};
  std::random_device rd; // Will be used to obtain a seed for the random number engine
  std::default_random_engine gen{rd()};
  for(float x = min.x + Resolution; x < max.x - Resolution; x += Resolution)
  {
    for(float y = min.y + Resolution; y < max.y - Resolution; y += Resolution)
    {
      for(float z = min.z + Resolution; z < max.z - Resolution; z += Resolution)
      {
        glm::vec3 p0 = glm::vec3{x, y, z};
        glm::vec3 offset{rdist(gen), rdist(gen), rdist(gen)};
        vertices.emplace_back(p0 + offset);
        indices.emplace_back(gsl::narrow_cast<uint32_t>(vertices.size()));
      }
    }
  }

  static const gl::VertexLayout<glm::vec3> layout{
    {VERTEX_ATTRIBUTE_POSITION_NAME, gl::VertexAttribute<glm::vec3>::Trivial{}}};

  auto vbuf = gslu::make_nn_shared<gl::VertexBuffer<glm::vec3>>(layout, label + "-particles");
  vbuf->setData(vertices, gl::api::BufferUsage::StaticDraw);
  auto indexBuffer = gslu::make_nn_shared<gl::ElementArrayBuffer<uint32_t>>(label + "-particles");
  indexBuffer->setData(indices, gl::api::BufferUsage::StaticDraw);

  const auto particleMaterial = materialManager.getDustParticle();

  auto vao = gslu::make_nn_shared<gl::VertexArray<uint32_t, glm::vec3>>(
    indexBuffer, vbuf, std::vector{&particleMaterial->getShaderProgram()->getHandle()}, label + "-particles");
  auto mesh = std::make_shared<render::scene::MeshImpl<uint32_t, glm::vec3>>(vao, gl::api::PrimitiveType::Points);
  mesh->getMaterialGroup().set(render::scene::RenderMode::Full, particleMaterial);
  mesh->getRenderState().setDepthWrite(false);

  dust = std::make_shared<render::scene::Node>(label + "-particles");
  dust->setRenderable(mesh);
  dust->setVisible(true);
  sceneryNodes.emplace_back(dust);
}
} // namespace engine::world
