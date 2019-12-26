#include "lightningball.h"

#include "engine/heightinfo.h"
#include "laraobject.h"
#include "render/gl/buffer.h"
#include "render/gl/vertexarray.h"
#include "render/scene/mesh.h"
#include "render/scene/names.h"
#include "serialization/array.h"

namespace engine::objects
{
namespace
{
gsl::not_null<std::shared_ptr<render::scene::Mesh>>
  createBolt(uint16_t points,
             const gsl::not_null<std::shared_ptr<render::scene::Material>>& material,
             float lineWidth,
             std::shared_ptr<render::gl::StructuredArrayBuffer<glm::vec3>>& vb)
{
  std::vector<glm::vec3> vertices(points);

  static const render::gl::StructureLayout<glm::vec3> attribs{
    {VERTEX_ATTRIBUTE_POSITION_NAME, render::gl::StructureMember<glm::vec3>::Trivial{}}};

  std::vector<uint16_t> indices;
  for(uint16_t i = 0; i < points; ++i)
    indices.emplace_back(i);

  auto indexBuffer = std::make_shared<render::gl::ElementArrayBuffer<uint16_t>>();
  indexBuffer->setData(indices, gl::BufferUsageARB::StaticDraw);

  vb = std::make_shared<render::gl::StructuredArrayBuffer<glm::vec3>>(attribs);
  vb->setData(&vertices[0], points, gl::BufferUsageARB::DynamicDraw);

  auto vao = std::make_shared<render::gl::VertexArray<uint16_t, glm::vec3>>(
    indexBuffer, vb, std::vector<const render::gl::Program*>{&material->getShaderProgram()->getHandle()});
  auto mesh = std::make_shared<render::scene::MeshImpl<uint16_t, glm::vec3>>(vao, gl::PrimitiveType::LineStrip);

  mesh->getRenderState().setLineSmooth(true);
  mesh->getRenderState().setLineWidth(lineWidth);

  mesh->getMaterial().set(render::scene::RenderMode::Full, material);

  return mesh;
}

using Bolt = std::array<core::TRVec, LightningBall::SegmentPoints>;

Bolt updateBolt(core::TRVec start,
                const core::TRVec& end,
                const std::shared_ptr<render::gl::StructuredArrayBuffer<glm::vec3>>& vb)
{
  const auto segmentSize = (end - start) / LightningBall::SegmentPoints;

  Bolt bolt;

  BOOST_ASSERT(vb->size() == LightningBall::SegmentPoints);
  const auto boltData = vb->map(gl::BufferAccessARB::WriteOnly);
  for(size_t j = 0; j < LightningBall::SegmentPoints; j++)
  {
    core::TRVec buckling{util::rand15s(core::QuarterSectorSize),
                         util::rand15s(core::QuarterSectorSize),
                         util::rand15s(core::QuarterSectorSize)};

    if(j == LightningBall::SegmentPoints - 1)
      buckling.Y = 0_len;

    bolt[j] = start + buckling;
    boltData[j] = bolt[j].toRenderSystem();
    start += segmentSize;
  }
  vb->unmap();

  return bolt;
}
} // namespace

LightningBall::LightningBall(const gsl::not_null<Engine*>& engine,
                             const gsl::not_null<const loader::file::Room*>& room,
                             const loader::file::Item& item,
                             const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
    : ModelObject{engine, room, item, true, animatedModel}
{
  if(animatedModel->nMeshes >= 1)
  {
    m_poles = static_cast<size_t>(animatedModel->nMeshes - 1);
  }

  init(*engine);
}

void LightningBall::update()
{
  if(!m_state.updateActivationTimeout())
  {
    m_chargeTimeout = 1;
    m_shooting = false;
    m_laraHit = false;
    if(getEngine().roomsAreSwapped())
      getEngine().swapAllRooms();

    deactivate();
    m_state.triggerState = TriggerState::Inactive;
    prepareRender();
    return;
  }

  prepareRender();

  if(--m_chargeTimeout > 0)
    return;

  if(m_shooting != 0)
  {
    m_shooting = false;
    m_chargeTimeout = 35 + util::rand15(45);
    m_laraHit = false;
    if(getEngine().roomsAreSwapped())
      getEngine().swapAllRooms();

    return;
  }

  m_shooting = true;
  m_chargeTimeout = 20;
  m_laraHit = false;

  const auto radius = m_poles == 0 ? core::SectorSize : core::SectorSize * 5 / 2;
  if(getEngine().getLara().isNear(*this, radius))
  {
    // target at lara
    m_mainBoltEnd = getEngine().getLara().m_state.position.position - m_state.position.position;
    m_mainBoltEnd
      = core::TRVec{glm::vec3((-m_state.rotation).toMatrix() * glm::vec4(m_mainBoltEnd.toRenderSystem(), 1.0f))};

    getEngine().getLara().m_state.health -= 400_hp;
    getEngine().getLara().m_state.is_hit = true;

    m_laraHit = true;
  }
  else if(m_poles == 0)
  {
    // we don't have poles, so just shoot downwards
    m_mainBoltEnd = core::TRVec{};
    const auto sector = loader::file::findRealFloorSector(m_state.position);
    m_mainBoltEnd.Y = -HeightInfo::fromFloor(sector, m_state.position.position, getEngine().getObjects()).y;
    m_mainBoltEnd.Y -= m_state.position.position.Y;
  }
  else
  {
    // select a random "pole"
    const auto objectSpheres = getSkeleton()->getBoneCollisionSpheres(
      m_state, *getSkeleton()->getInterpolationInfo(m_state).getNearestFrame(), nullptr);
    m_mainBoltEnd = core::TRVec{objectSpheres[util::rand15(objectSpheres.size() - 1) + 1].getPosition()}
                    - m_state.position.position;
    m_mainBoltEnd
      = core::TRVec{glm::vec3((-m_state.rotation).toMatrix() * glm::vec4(m_mainBoltEnd.toRenderSystem(), 1.0f))};
  }

  for(auto& childBolt : m_childBolts)
  {
    childBolt.startIndex = util::rand15(SegmentPoints - 1);
    childBolt.end
      = m_mainBoltEnd
        + core::TRVec{util::rand15s(core::QuarterSectorSize), 0_len, util::rand15s(core::QuarterSectorSize)};
  }

  if(!getEngine().roomsAreSwapped())
    getEngine().swapAllRooms();

  playSoundEffect(TR1SoundId::Chatter);
}

void LightningBall::collide(CollisionInfo& /*info*/)
{
  if(!m_laraHit)
    return;

  getEngine().getLara().hit_direction = static_cast<core::Axis>(util::rand15(4));
  getEngine().getLara().hit_frame += 1_frame;
  if(getEngine().getLara().hit_frame > 34_frame)
    getEngine().getLara().hit_frame = 34_frame;
}

void LightningBall::prepareRender()
{
  ModelObject::update();

  if(m_shooting == 0)
  {
    for(size_t i = 1; i < getSkeleton()->getChildren().size(); ++i)
    {
      getSkeleton()->getChildren()[i]->setVisible(false);
    }
    return;
  }

  for(size_t i = 1; i < getSkeleton()->getChildren().size(); ++i)
  {
    getSkeleton()->getChildren()[i]->setVisible(i - 1 >= m_poles);
  }

  const auto nearestFrame = getSkeleton()->getInterpolationInfo(m_state).getNearestFrame();
  const auto segmentStart = core::TRVec{
    glm::vec3(core::fromPackedAngles(nearestFrame->getAngleData()[0]) * glm::vec4(nearestFrame->pos.toGl(), 1.0f))};

  const Bolt mainBolt = updateBolt(segmentStart, m_mainBoltEnd, m_mainVb);

  for(const auto& childBolt : m_childBolts)
  {
    updateBolt(mainBolt[childBolt.startIndex], childBolt.end, childBolt.vb);
  }
}

void LightningBall::init(Engine& engine)
{
  for(size_t i = 1; i < getSkeleton()->getChildren().size(); ++i)
  {
    getSkeleton()->getChildren()[i]->setRenderable(nullptr);
    getSkeleton()->getChildren()[i]->setVisible(false);
  }

  m_mainBoltMesh = createBolt(SegmentPoints, engine.getMaterialManager()->getLightning(), 10, m_mainVb);
  auto node = std::make_shared<render::scene::Node>("lightning-bolt-main");
  node->setRenderable(m_mainBoltMesh);
  addChild(getSkeleton(), node);

  for(auto& childBolt : m_childBolts)
  {
    childBolt.mesh = createBolt(SegmentPoints, engine.getMaterialManager()->getLightning(), 3, childBolt.vb);

    node = std::make_shared<render::scene::Node>("lightning-bolt-child");
    node->setRenderable(childBolt.mesh);
    addChild(getSkeleton(), node);
  }
}

void LightningBall::serialize(const serialization::Serializer& ser)
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
    init(ser.engine);
  }
}

void LightningBall::ChildBolt::serialize(const serialization::Serializer& ser)
{
  ser(S_NV("startIndex", startIndex), S_NV("end", end));
}
} // namespace engine::objects
