#pragma once

#include "modelobject.h"

namespace engine::objects
{
class LightningBall final : public ModelObject
{
public:
  LightningBall(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : ModelObject{engine, position}
  {
  }

  LightningBall(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  static constexpr size_t SegmentPoints = 16;

  void serialize(const serialization::Serializer& ser) override;

private:
  static constexpr size_t ChildBolts = 5;

  void prepareRender();

  size_t m_poles = 0;
  bool m_laraHit = false;
  int m_chargeTimeout = 1;
  bool m_shooting = false;
  core::TRVec m_mainBoltEnd;

  struct ChildBolt
  {
    size_t startIndex = 0;
    core::TRVec end{};
    std::shared_ptr<render::scene::Mesh> mesh;
    std::shared_ptr<render::gl::VertexBuffer<glm::vec3>> vb;

    void serialize(const serialization::Serializer& ser);
  };

  std::array<ChildBolt, ChildBolts> m_childBolts;

  std::shared_ptr<render::scene::Mesh> m_mainBoltMesh;
  std::shared_ptr<render::gl::VertexBuffer<glm::vec3>> m_mainVb;

  void init(Engine& engine);
};
} // namespace engine::objects
