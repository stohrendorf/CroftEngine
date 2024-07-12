#include "lightningemitter.h"

#include "core/angle.h"
#include "core/genericvec.h"
#include "core/magic.h"
#include "core/units.h"
#include "engine/heightinfo.h"
#include "engine/location.h"
#include "engine/objectmanager.h"
#include "engine/presenter.h"
#include "engine/skeletalmodelnode.h"
#include "engine/soundeffects_tr1.h"
#include "engine/world/skeletalmodeltype.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "loader/file/animation.h"
#include "modelobject.h"
#include "objectstate.h"
#include "qs/qs.h"
#include "render/material/material.h"
#include "render/material/materialgroup.h"
#include "render/material/materialmanager.h"
#include "render/material/rendermode.h"
#include "render/material/shaderprogram.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "render/scene/node.h"
#include "serialization/serialization.h"
#include "util/helpers.h"

#include <algorithm>
#include <array>
#include <boost/assert.hpp>
#include <cstddef>
#include <cstdint>
#include <gl/buffer.h>
#include <gl/constants.h>
#include <gl/renderstate.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>
#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace engine::objects
{
namespace
{
std::tuple<gslu::nn_shared<render::scene::Mesh>, gslu::nn_shared<gl::VertexBuffer<glm::vec3>>>
  createBolt(const gslu::nn_shared<render::material::Material>& material, float lineWidth)
{
  std::array<glm::vec3, LightningEmitter::ControlPoints> vertices{};

  static const gl::VertexLayout<glm::vec3> layout{
    {VERTEX_ATTRIBUTE_POSITION_NAME, gl::VertexAttribute<glm::vec3>::Single{}}};

  std::vector<uint16_t> indices;
  for(uint16_t i = 0; i < LightningEmitter::ControlPoints; ++i)
    indices.emplace_back(i);

  auto indexBuffer = gsl::make_shared<gl::ElementArrayBuffer<uint16_t>>(
    "bolt" + gl::IndexBufferSuffix, gl::api::BufferUsage::StreamDraw, indices);

  auto vb = gsl::make_shared<gl::VertexBuffer<glm::vec3>>(
    layout, "bolt" + gl::VboSuffix, gl::api::BufferUsage::StreamDraw, vertices);

  auto opaqueVao = gsl::make_shared<gl::VertexArray<uint16_t, glm::vec3>>(
    indexBuffer, vb, std::vector{&material->getShaderProgram()->getHandle()}, "bolt" + gl::VaoSuffix);
  auto mesh = gsl::make_shared<render::scene::MeshImpl<uint16_t, glm::vec3>>(
    opaqueVao, nullptr, gl::api::PrimitiveType::LineStrip);

  mesh->getRenderState().setLineSmooth(true);
  mesh->getRenderState().setLineWidth(lineWidth);
  mesh->getRenderState().setScissorTest(false);

  mesh->getMaterialGroup().set(render::material::RenderMode::FullOpaque, material);

  return {mesh, vb};
}

using Bolt = std::array<glm::vec3, LightningEmitter::ControlPoints>;

Bolt updateBolt(const glm::vec3& start, const glm::vec3& end, const std::shared_ptr<gl::VertexBuffer<glm::vec3>>& vb)
{
  std::vector<glm::vec3> data{start, end};
  static constexpr float RadiusRatio = 0.3f;
  auto radius = static_cast<float>(glm::distance(start, end) * RadiusRatio);
  for(size_t i = 0; i < LightningEmitter::SegmentSplits; ++i)
  {
    std::vector<glm::vec3> splitData;
    splitData.reserve(data.size() * 2);
    for(size_t j = 0; j < data.size() - 1; ++j)
    {
      const auto& a = data[j];
      const auto& b = data[j + 1];
      splitData.emplace_back(a);

      const auto d = glm::normalize(b - a);
      while(true)
      {
        const auto randomVec = glm::normalize(glm::vec3{util::rand15s(), util::rand15s(), util::rand15s()});
        if(glm::abs(glm::dot(randomVec, d)) > 0.999f)
        {
          continue;
        }

        const auto randomNormal = glm::normalize(randomVec - glm::dot(randomVec, d) * d);
        BOOST_ASSERT(glm::abs(glm::dot(randomNormal, d)) < 0.001f);
        splitData.emplace_back((a + b) / 2.0f + randomNormal * radius);
        break;
      }
    }
    splitData.emplace_back(data.back());
    data = std::move(splitData);
    radius /= 2;
  }

  gsl_Assert(gsl::narrow<size_t>(vb->size()) == data.size());

  Bolt bolt;
  auto boltData
    = vb->map(gl::api::MapBufferAccessMask::MapWriteBit | gl::api::MapBufferAccessMask::MapFlushExplicitBit);
  gsl_Assert(boltData.size() == data.size());
  gsl_Assert(boltData.size() == bolt.size());
  std::copy(data.begin(), data.end(), boltData.begin());
  std::copy(data.begin(), data.end(), bolt.begin());
  boltData.flush();

  return bolt;
}
} // namespace

LightningEmitter::LightningEmitter(const std::string& name,
                                   const gsl::not_null<world::World*>& world,
                                   const gsl::not_null<const world::Room*>& room,
                                   const loader::file::Item& item,
                                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{name, world, room, item, true, animatedModel, false}
{
  if(!animatedModel->bones.empty())
  {
    m_poles = gsl::narrow_cast<size_t>(animatedModel->bones.size() - 1u);
  }

  init(*world);
  prepareRender();
  getSkeleton()->getRenderState().setScissorTest(false);
}

void LightningEmitter::update()
{
  if(!m_state.updateActivationTimeout())
  {
    m_chargeTimeout = 1;
    m_shooting = false;
    m_laraHit = false;
    if(getWorld().roomsAreSwapped())
      getWorld().swapAllRooms();

    deactivate();
    m_state.triggerState = TriggerState::Inactive;
    prepareRender();
    return;
  }

  prepareRender();

  if(--m_chargeTimeout > 0)
    return;

  if(m_shooting)
  {
    m_shooting = false;
    m_chargeTimeout = 35 + util::rand15(45);
    m_laraHit = false;
    if(getWorld().roomsAreSwapped())
      getWorld().swapAllRooms();

    return;
  }

  m_shooting = true;
  m_chargeTimeout = 20;
  m_laraHit = false;

  const auto radius = m_poles == 0 ? 1_sectors : 5_sectors / 2;
  if(getWorld().getObjectManager().getLara().isNearInexact(m_state.location.position, radius))
  {
    // target at lara
    m_mainBoltEnd = getWorld().getObjectManager().getLara().m_state.location.position - m_state.location.position;
    m_mainBoltEnd
      = core::TRVec{glm::vec3((-m_state.rotation).toMatrix() * glm::vec4(m_mainBoltEnd.toRenderSystem(), 1.0f))};

    getWorld().hitLara(400_hp);

    m_laraHit = true;
  }
  else if(m_poles == 0)
  {
    // we don't have poles, so just shoot downwards
    m_mainBoltEnd = core::TRVec{};
    const auto sector = m_state.location.updateRoom();
    m_mainBoltEnd.Y
      = -HeightInfo::fromFloor(sector, m_state.location.position, getWorld().getObjectManager().getObjects()).y;
    m_mainBoltEnd.Y -= m_state.location.position.Y;
  }
  else
  {
    // select a random "pole"
    const auto objectSpheres = getSkeleton()->getBoneCollisionSpheres();
    m_mainBoltEnd = core::TRVec{objectSpheres[util::rand15(objectSpheres.size() - 1) + 1].getCollisionPosition()}
                    - m_state.location.position;
    m_mainBoltEnd
      = core::TRVec{glm::vec3((-m_state.rotation).toMatrix() * glm::vec4(m_mainBoltEnd.toRenderSystem(), 1.0f))};
  }

  if(!getWorld().roomsAreSwapped())
    getWorld().swapAllRooms();

  playSoundEffect(TR1SoundEffect::Chatter);
}

void LightningEmitter::collide(CollisionInfo& /*info*/)
{
  if(!m_laraHit)
    return;

  auto& lara = getWorld().getObjectManager().getLara();
  lara.hit_direction = static_cast<core::Axis>(util::rand15(4));
  lara.hit_frame += 1_frame;
  if(lara.hit_frame > 34_frame)
    lara.hit_frame = 34_frame;
}

void LightningEmitter::prepareRender()
{
  ModelObject::update();
  for(size_t i = 1; i < getSkeleton()->getBoneCount(); ++i)
    getSkeleton()->setVisible(i, false);
  getSkeleton()->rebuildMesh();

  m_mainBoltNode->setVisible(m_shooting);
  for(const auto& child : m_childBolts)
    child.node->setVisible(m_shooting);
  if(!m_shooting)
    return;

  const auto nearestFrame = getSkeleton()->getInterpolationInfo().getNearestFrame();
  const auto segmentStart = glm::vec3(core::fromPackedAngles(nearestFrame->getAngleData().data())
                                      * glm::vec4(nearestFrame->pos.toGl(), 1.0f));

  const Bolt mainBolt = updateBolt(segmentStart, m_mainBoltEnd.toRenderSystem(), m_mainVb);

  for(const auto& childBolt : m_childBolts)
  {
    const auto end
      = m_mainBoltEnd
        + core::TRVec{util::rand15s(core::QuarterSectorSize / 2), 0_len, util::rand15s(core::QuarterSectorSize / 2)};
    updateBolt(mainBolt[util::rand15(ControlPoints - 1)], end.toRenderSystem(), childBolt.vb);
  }
}

void LightningEmitter::init(world::World& world)
{
  std::tie(m_mainBoltMesh, m_mainVb) = createBolt(world.getPresenter().getMaterialManager()->getLightning(), 10);
  m_mainBoltNode = std::make_shared<render::scene::Node>("lightning-bolt-main");
  m_mainBoltNode->setRenderable(m_mainBoltMesh);
  m_mainBoltNode->setVisible(false);
  addChild(gsl::not_null{getSkeleton()}, gsl::not_null{m_mainBoltNode});

  for(auto& childBolt : m_childBolts)
  {
    std::tie(childBolt.mesh, childBolt.vb) = createBolt(world.getPresenter().getMaterialManager()->getLightning(), 3);

    childBolt.node = std::make_shared<render::scene::Node>("lightning-bolt-child");
    childBolt.node->setRenderable(childBolt.mesh);
    childBolt.node->setVisible(false);
    addChild(gsl::not_null{getSkeleton()}, gsl::not_null{childBolt.node});
  }
}

void LightningEmitter::serialize(const serialization::Serializer<world::World>& ser) const
{
  ModelObject::serialize(ser);
  ser(S_NV("poles", m_poles),
      S_NV("laraHit", m_laraHit),
      S_NV("chargeTimeout", m_chargeTimeout),
      S_NV("shooting", m_shooting),
      S_NV("mainBoltEnd", m_mainBoltEnd));
}

void LightningEmitter::deserialize(const serialization::Deserializer<world::World>& ser)
{
  ModelObject::deserialize(ser);
  ser(S_NV("poles", m_poles),
      S_NV("laraHit", m_laraHit),
      S_NV("chargeTimeout", m_chargeTimeout),
      S_NV("shooting", m_shooting),
      S_NV("mainBoltEnd", m_mainBoltEnd));

  init(*ser.context);
  ser << [this](const serialization::Deserializer<world::World>& /*ser*/)
  {
    prepareRender();
  };

  getSkeleton()->getRenderState().setScissorTest(false);
}
} // namespace engine::objects
