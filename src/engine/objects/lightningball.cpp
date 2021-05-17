#include "lightningball.h"

#include "engine/heightinfo.h"
#include "engine/presenter.h"
#include "engine/world/world.h"
#include "laraobject.h"
#include "render/scene/material.h"
#include "render/scene/materialmanager.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "serialization/array.h"
#include "serialization/serialization.h"

#include <gl/buffer.h>
#include <gl/vertexarray.h>
#include <gl/vertexbuffer.h>

namespace engine::objects
{
namespace
{
gsl::not_null<std::shared_ptr<render::scene::Mesh>>
  createBolt(const gsl::not_null<std::shared_ptr<render::scene::Material>>& material,
             float lineWidth,
             std::shared_ptr<gl::VertexBuffer<glm::vec3>>& vb)
{
  std::vector<glm::vec3> vertices(LightningBall::ControlPoints);

  static const gl::VertexLayout<glm::vec3> layout{
    {VERTEX_ATTRIBUTE_POSITION_NAME, gl::VertexAttribute<glm::vec3>::Trivial{}}};

  std::vector<uint16_t> indices;
  for(uint16_t i = 0; i < LightningBall::ControlPoints; ++i)
    indices.emplace_back(i);

  auto indexBuffer = std::make_shared<gl::ElementArrayBuffer<uint16_t>>("bolt");
  indexBuffer->setData(indices, gl::api::BufferUsage::StaticDraw);

  vb = std::make_shared<gl::VertexBuffer<glm::vec3>>(layout, 0, "bolt");
  vb->setData(
    &vertices[0], gsl::narrow_cast<gl::api::core::SizeType>(vertices.size()), gl::api::BufferUsage::DynamicDraw);

  auto vao = std::make_shared<gl::VertexArray<uint16_t, glm::vec3>>(
    indexBuffer, vb, std::vector{&material->getShaderProgram()->getHandle()}, "bolt");
  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, glm::vec3>>(vao, gl::api::PrimitiveType::LineStrip);

  mesh->getRenderState().setLineSmooth(true);
  mesh->getRenderState().setLineWidth(lineWidth);

  mesh->getMaterialGroup().set(render::scene::RenderMode::Full, material);

  return mesh;
}

using Bolt = std::array<glm::vec3, LightningBall::ControlPoints>;

Bolt updateBolt(const glm::vec3& start, const core::TRVec& end, const std::shared_ptr<gl::VertexBuffer<glm::vec3>>& vb)
{
  std::vector<glm::vec3> data{start, end.toRenderSystem()};
  float radius = core::QuarterSectorSize.get();
  for(size_t i = 0; i < LightningBall::SegmentSplits; ++i)
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
        auto randomVec = glm::normalize(glm::vec3{util::rand15s(), util::rand15s(), util::rand15s()});
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

  Expects(gsl::narrow<size_t>(vb->size()) == data.size());

  Bolt bolt;
  const auto boltData = vb->map(gl::api::BufferAccess::WriteOnly);
  std::copy(data.begin(), data.end(), boltData);
  std::copy(data.begin(), data.end(), bolt.begin());
  vb->unmap();

  return bolt;
}
} // namespace

LightningBall::LightningBall(const gsl::not_null<world::World*>& world,
                             const gsl::not_null<const world::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const world::SkeletalModelType*>& animatedModel)
    : ModelObject{world, room, item, true, animatedModel}
{
  if(!animatedModel->bones.empty())
  {
    m_poles = static_cast<size_t>(animatedModel->bones.size() - 1u);
  }

  init(*world);
}

void LightningBall::update()
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

  const auto radius = m_poles == 0 ? core::SectorSize : core::SectorSize * 5 / 2;
  if(getWorld().getObjectManager().getLara().isNear(*this, radius))
  {
    // target at lara
    m_mainBoltEnd = getWorld().getObjectManager().getLara().m_state.position.position - m_state.position.position;
    m_mainBoltEnd
      = core::TRVec{glm::vec3((-m_state.rotation).toMatrix() * glm::vec4(m_mainBoltEnd.toRenderSystem(), 1.0f))};

    getWorld().getObjectManager().getLara().m_state.health -= 400_hp;
    getWorld().getObjectManager().getLara().m_state.is_hit = true;

    m_laraHit = true;
  }
  else if(m_poles == 0)
  {
    // we don't have poles, so just shoot downwards
    m_mainBoltEnd = core::TRVec{};
    const auto sector = world::findRealFloorSector(m_state.position);
    m_mainBoltEnd.Y
      = -HeightInfo::fromFloor(sector, m_state.position.position, getWorld().getObjectManager().getObjects()).y;
    m_mainBoltEnd.Y -= m_state.position.position.Y;
  }
  else
  {
    // select a random "pole"
    const auto objectSpheres = getSkeleton()->getBoneCollisionSpheres(
      m_state, *getSkeleton()->getInterpolationInfo().getNearestFrame(), nullptr);
    m_mainBoltEnd = core::TRVec{objectSpheres[util::rand15(objectSpheres.size() - 1) + 1].getPosition()}
                    - m_state.position.position;
    m_mainBoltEnd
      = core::TRVec{glm::vec3((-m_state.rotation).toMatrix() * glm::vec4(m_mainBoltEnd.toRenderSystem(), 1.0f))};
  }

  for(auto& childBolt : m_childBolts)
  {
    childBolt.end
      = m_mainBoltEnd
        + core::TRVec{util::rand15s(core::QuarterSectorSize / 2), 0_len, util::rand15s(core::QuarterSectorSize / 2)};
  }

  if(!getWorld().roomsAreSwapped())
    getWorld().swapAllRooms();

  playSoundEffect(TR1SoundEffect::Chatter);
}

void LightningBall::collide(CollisionInfo& /*info*/)
{
  if(!m_laraHit)
    return;

  getWorld().getObjectManager().getLara().hit_direction = static_cast<core::Axis>(util::rand15(4));
  getWorld().getObjectManager().getLara().hit_frame += 1_frame;
  if(getWorld().getObjectManager().getLara().hit_frame > 34_frame)
    getWorld().getObjectManager().getLara().hit_frame = 34_frame;
}

void LightningBall::prepareRender()
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
  const auto segmentStart
    = glm::vec3(core::fromPackedAngles(nearestFrame->getAngleData()[0]) * glm::vec4(nearestFrame->pos.toGl(), 1.0f));

  const Bolt mainBolt = updateBolt(segmentStart, m_mainBoltEnd, m_mainVb);

  for(const auto& childBolt : m_childBolts)
  {
    updateBolt(mainBolt[util::rand15(ControlPoints - 1)], childBolt.end, childBolt.vb);
  }
}

void LightningBall::init(world::World& world)
{
  m_mainBoltMesh = createBolt(world.getPresenter().getMaterialManager()->getLightning(), 10, m_mainVb);
  m_mainBoltNode = std::make_shared<render::scene::Node>("lightning-bolt-main");
  m_mainBoltNode->setRenderable(m_mainBoltMesh);
  m_mainBoltNode->setVisible(false);
  addChild(getSkeleton(), m_mainBoltNode);

  for(auto& childBolt : m_childBolts)
  {
    childBolt.mesh = createBolt(world.getPresenter().getMaterialManager()->getLightning(), 3, childBolt.vb);

    childBolt.node = std::make_shared<render::scene::Node>("lightning-bolt-child");
    childBolt.node->setRenderable(childBolt.mesh);
    childBolt.node->setVisible(false);
    addChild(getSkeleton(), childBolt.node);
  }
}

void LightningBall::serialize(const serialization::Serializer<world::World>& ser)
{
  ModelObject::serialize(ser);
  ser(S_NV("poles", m_poles),
      S_NV("laraHit", m_laraHit),
      S_NV("chargeTimeout", m_chargeTimeout),
      S_NV("shooting", m_shooting),
      S_NV("mainBoltEnd", m_mainBoltEnd),
      S_NV("childBolts", m_childBolts));

  if(ser.loading)
  {
    init(ser.context);
  }
}

void LightningBall::ChildBolt::serialize(const serialization::Serializer<world::World>& ser)
{
  ser(S_NV("end", end));
}
} // namespace engine::objects
