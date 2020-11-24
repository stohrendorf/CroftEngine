#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class FlyingMutant : public AIAgent
{
public:
  FlyingMutant(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : AIAgent{world, position}
  {
  }

  FlyingMutant(const gsl::not_null<World*>& world,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
  }

  void update() final;
};

class WalkingMutant final : public FlyingMutant
{
public:
  WalkingMutant(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : FlyingMutant{world, position}
  {
  }

  WalkingMutant(const gsl::not_null<World*>& world,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : FlyingMutant{world, room, item, animatedModel}
  {
    for(size_t i = 0; i < getSkeleton()->getBoneCount(); ++i)
    {
      getSkeleton()->setVisible(i, (0xffe07fffu >> i) & 1u);
    }
    getSkeleton()->rebuildMesh();
  }
};

class CentaurMutant final : public AIAgent
{
public:
  CentaurMutant(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : AIAgent{world, position}
  {
  }

  CentaurMutant(const gsl::not_null<World*>& world,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
  }

  void update() override;
};

class TorsoBoss final : public AIAgent
{
public:
  TorsoBoss(const gsl::not_null<World*>& world, const core::RoomBoundPosition& position)
      : AIAgent{world, position}
  {
  }

  TorsoBoss(const gsl::not_null<World*>& world,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{world, room, item, animatedModel}
  {
  }

  void update() override;
};

} // namespace engine::objects
