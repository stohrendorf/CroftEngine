#pragma once

#include "itemnode.h"

namespace engine
{
namespace items
{
class Door final : public ModelItemNode
{
public:
    Door(const gsl::not_null<loader::file::level::Level*>& level,
         const gsl::not_null<const loader::file::Room*>& room,
         const loader::file::Item& item,
         const loader::file::SkeletalModelType& animatedModel);

    void update() override;

    void collide(LaraNode& lara, CollisionInfo& collisionInfo) override;

private:
    loader::file::Sector* m_sector{nullptr};
    loader::file::Sector m_sectorData;
    loader::file::Box* m_box{nullptr};
    loader::file::Sector* m_alternateSector{nullptr};
    loader::file::Sector m_alternateSectorData;
    loader::file::Box* m_alternateBox{nullptr};
    loader::file::Sector* m_targetSector{nullptr};
    loader::file::Sector m_targetSectorData;
    loader::file::Box* m_targetBox{nullptr};
    loader::file::Sector* m_alternateTargetSector{nullptr};
    loader::file::Sector m_alternateTargetSectorData;
    loader::file::Box* m_alternateTargetBox{nullptr};
};
}
}
