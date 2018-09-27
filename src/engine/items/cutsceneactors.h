#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class CutsceneActor : public ModelItemNode
{
public:
    CutsceneActor(const gsl::not_null<level::Level*>& level,
                  const gsl::not_null<const loader::Room*>& room,
                  const loader::Item& item,
                  const loader::SkeletalModelType& animatedModel);

    void update() override;
};


class CutsceneActor1 final : public CutsceneActor
{
public:
    CutsceneActor1(const gsl::not_null<level::Level*>& level,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel)
            : CutsceneActor( level, room, item, animatedModel )
    {}
};


class CutsceneActor2 final : public CutsceneActor
{
public:
    CutsceneActor2(const gsl::not_null<level::Level*>& level,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel)
            : CutsceneActor( level, room, item, animatedModel )
    {}
};


class CutsceneActor3 final : public CutsceneActor
{
public:
    CutsceneActor3(const gsl::not_null<level::Level*>& level,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel)
            : CutsceneActor( level, room, item, animatedModel )
    {}
};


class CutsceneActor4 final : public CutsceneActor
{
public:
    CutsceneActor4(const gsl::not_null<level::Level*>& level,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel)
            : CutsceneActor( level, room, item, animatedModel )
    {}

    void update() override;
};
}
}
