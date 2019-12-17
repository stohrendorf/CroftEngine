#pragma once

#include "aiagent.h"
#include "engine/ai/ai.h"

namespace engine::objects
{
class FlyingMutant : public AIAgent
{
public:
  FlyingMutant(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : AIAgent{engine, position}
  {
  }

  FlyingMutant(const gsl::not_null<Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{engine, room, item, animatedModel}
  {
  }

  void update() final;
};

class WalkingMutant final : public FlyingMutant
{
public:
  WalkingMutant(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : FlyingMutant{engine, position}
  {
  }

  WalkingMutant(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : FlyingMutant{engine, room, item, animatedModel}
  {
    for(size_t i = 0; i < getSkeleton()->getChildren().size(); ++i)
    {
      getSkeleton()->getChild(i)->setVisible((0xffe07fffu >> i) & 1u);
    }
  }
};

class CentaurMutant final : public AIAgent
{
public:
  CentaurMutant(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : AIAgent{engine, position}
  {
  }

  CentaurMutant(const gsl::not_null<Engine*>& engine,
                const gsl::not_null<const loader::file::Room*>& room,
                const loader::file::Item& item,
                const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{engine, room, item, animatedModel}
  {
  }

  void update() override;
};

class TorsoBoss final : public AIAgent
{
public:
  TorsoBoss(const gsl::not_null<Engine*>& engine, const core::RoomBoundPosition& position)
      : AIAgent{engine, position}
  {
  }

  TorsoBoss(const gsl::not_null<Engine*>& engine,
            const gsl::not_null<const loader::file::Room*>& room,
            const loader::file::Item& item,
            const gsl::not_null<const loader::file::SkeletalModelType*>& animatedModel)
      : AIAgent{engine, room, item, animatedModel}
  {
  }

  void update() override;
};

} // namespace engine::objects
