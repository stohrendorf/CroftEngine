#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class CutsceneActor1 final : public ModelItemNode
{
public:
    CutsceneActor1(const gsl::not_null<level::Level*>& level,
                   const std::string& name,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel);

    void update() override;
};


class CutsceneActor2 final : public ModelItemNode
{
public:
    CutsceneActor2(const gsl::not_null<level::Level*>& level,
                   const std::string& name,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel);

    void update() override;
};


class CutsceneActor3 final : public ModelItemNode
{
public:
    CutsceneActor3(const gsl::not_null<level::Level*>& level,
                   const std::string& name,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel);

    void update() override;
};


class CutsceneActor4 final : public ModelItemNode
{
public:
    CutsceneActor4(const gsl::not_null<level::Level*>& level,
                   const std::string& name,
                   const gsl::not_null<const loader::Room*>& room,
                   const loader::Item& item,
                   const loader::SkeletalModelType& animatedModel);

    void update() override;
};
}
}
