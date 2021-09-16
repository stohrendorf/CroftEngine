#pragma once

#include "core/vec.h"
#include "modelobject.h"
#include "serialization/serialization_fwd.h"

#include <array>
#include <cstddef>
#include <gl/soglb_fwd.h>
#include <glm/fwd.hpp>
#include <gsl/gsl-lite.hpp>
#include <memory>
#include <string>

// IWYU pragma: no_forward_declare serialization::Serializer

namespace engine
{
struct CollisionInfo;
struct Location;
} // namespace engine

namespace engine::world
{
class World;
struct Room;
struct SkeletalModelType;
} // namespace engine::world

namespace loader::file
{
struct Item;
}

namespace render::scene
{
class Node;
class Mesh;
} // namespace render::scene

namespace engine::objects
{
class LightningEmitter final : public ModelObject
{
public:
  LightningEmitter(const gsl::not_null<world::World*>& world, const Location& location)
      : ModelObject{world, location}
  {
  }

  LightningEmitter(const std::string& name,
                   const gsl::not_null<world::World*>& world,
                   const gsl::not_null<const world::Room*>& room,
                   const loader::file::Item& item,
                   const gsl::not_null<const world::SkeletalModelType*>& animatedModel);

  void update() override;

  void collide(CollisionInfo& info) override;

  static constexpr size_t SegmentSplits = 8;
  static constexpr size_t ControlPoints = (1u << SegmentSplits) + 1;

  void serialize(const serialization::Serializer<world::World>& ser) override;

private:
  static constexpr size_t ChildBolts = 5;

  void prepareRender();

  size_t m_poles = 0;
  bool m_laraHit = false;
  int m_chargeTimeout = 1;
  bool m_shooting = false;
  core::TRVec m_mainBoltEnd{};

  struct ChildBolt
  {
    std::shared_ptr<render::scene::Mesh> mesh = nullptr;
    std::shared_ptr<render::scene::Node> node = nullptr;
    std::shared_ptr<gl::VertexBuffer<glm::vec3>> vb = nullptr;
  };

  std::array<ChildBolt, ChildBolts> m_childBolts{};

  std::shared_ptr<render::scene::Mesh> m_mainBoltMesh = nullptr;
  std::shared_ptr<render::scene::Node> m_mainBoltNode = nullptr;
  std::shared_ptr<gl::VertexBuffer<glm::vec3>> m_mainVb = nullptr;

  void init(world::World& world);
};
} // namespace engine::objects
