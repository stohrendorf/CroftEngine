#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class StubItem final : public ModelItemNode
{
public:
  StubItem(const gsl::not_null<Engine*>& engine,
           const gsl::not_null<const loader::file::Room*>& room,
           const loader::file::Item& item,
           const loader::file::SkeletalModelType& animatedModel)
      : ModelItemNode{engine, room, item, false, animatedModel}
  {
  }
};

class ScriptedItem final : public ModelItemNode
{
public:
  ScriptedItem(const gsl::not_null<engine::Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const loader::file::SkeletalModelType& animatedModel,
               const sol::table& objectInfo)
      : ModelItemNode{engine, room, item, false, animatedModel}
      , m_objectInfo{objectInfo}
  {
    auto initialise = objectInfo["initialise"];
    if(initialise)
      initialise.call(m_state);
  }

private:
  sol::table m_objectInfo;
};
} // namespace items
} // namespace engine
