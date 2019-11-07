#pragma once

#include "engine/ai/ai.h"
#include "itemnode.h"

namespace engine
{
namespace items
{
class Doppelganger final : public ModelItemNode
{
public:
  Doppelganger(const gsl::not_null<Engine*>& engine,
               const gsl::not_null<const loader::file::Room*>& room,
               const loader::file::Item& item,
               const loader::file::SkeletalModelType& animatedModel);

  void update() override;

private:
  bool m_flag = false;
};
} // namespace items
} // namespace engine
