#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Door final : public ModelItemNode
{
public:
  Door(const gsl::not_null<Engine*>& engine,
       const gsl::not_null<const loader::file::Room*>& room,
       const loader::file::Item& item,
       const loader::file::SkeletalModelType& animatedModel);

  void update() override;

  void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

private:
  struct Info
  {
    loader::file::Sector* sector{nullptr};
    loader::file::Sector sectorData;
    loader::file::Box* box{nullptr};

    void open()
    {
      if(sector == nullptr)
        return;

      *sector = sectorData;
      if(box != nullptr)
        box->blocked = false;
    }

    void close()
    {
      if(sector == nullptr)
        return;

      sector->reset();
      if(box != nullptr)
        box->blocked = true;
    }

    const loader::file::Room* init(const loader::file::Room& room, const core::TRVec& wingsPosition)
    {
      sector = const_cast<loader::file::Sector*>(room.getSectorByAbsolutePosition(wingsPosition));
      Expects(sector != nullptr);
      sectorData = *sector;

      if(sector->portalTarget == nullptr)
      {
        box = const_cast<loader::file::Box*>(sector->box);
      }
      else
      {
        box = const_cast<loader::file::Box*>(sector->portalTarget->getSectorByAbsolutePosition(wingsPosition)->box);
      }
      if(box != nullptr && !box->blockable)
      {
        box = nullptr;
      }

      const auto portalTarget = sector->portalTarget;
      close();
      return portalTarget;
    }
  };

  Info m_info;
  Info m_alternateInfo;
  Info m_target;
  Info m_alternateTarget;
};
} // namespace items
} // namespace engine
