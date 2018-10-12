#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Door final : public ModelItemNode
{
public:
    Door(const gsl::not_null<level::Level*>& level,
         const gsl::not_null<const loader::Room*>& room,
         const loader::Item& item,
         const loader::SkeletalModelType& animatedModel);

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

private:
    loader::Sector* m_sector{nullptr};
    loader::Sector m_sectorData;
    loader::Box* m_box{nullptr};
    loader::Sector* m_alternateSector{nullptr};
    loader::Sector m_alternateSectorData;
    loader::Box* m_alternateBox{nullptr};
    loader::Sector* m_targetSector{nullptr};
    loader::Sector m_targetSectorData;
    loader::Box* m_targetBox{nullptr};
    loader::Sector* m_alternateTargetSector{nullptr};
    loader::Sector m_alternateTargetSectorData;
    loader::Box* m_alternateTargetBox{nullptr};
};
}
}
