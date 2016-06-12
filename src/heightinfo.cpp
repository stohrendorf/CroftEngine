#include "heightinfo.h"

#include "loader/trcamerascenenodeanimator.h"

bool HeightInfo::skipSteepSlants = false;

HeightInfo HeightInfo::fromFloor(const loader::Sector* roomSector, const loader::TRCoordinates& pos, const TRCameraSceneNodeAnimator* camera)
{
    HeightInfo hi;

    hi.slantClass = SlantClass::None;

    BOOST_ASSERT(roomSector != nullptr);
    while( roomSector->roomBelow != 0xff )
    {
        BOOST_ASSERT(roomSector->roomBelow < camera->getLevel()->m_rooms.size());
        auto room = &camera->getLevel()->m_rooms[roomSector->roomBelow];
        roomSector = room->getSectorByAbsolutePosition(pos);
    }

    hi.distance = roomSector->floorHeight * loader::QuarterSectorSize;
    hi.lastTriggerOrKill = nullptr;

    if( roomSector->floorDataIndex == 0 )
    {
        return hi;
    }

    const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
    while( true )
    {
        const bool isLast = loader::isLastFloordataEntry(*floorData);
        const auto currentFd = *floorData;
        ++floorData;
        switch( loader::extractFDFunction(currentFd) )
        {
        case loader::FDFunction::FloorSlant:
            {
                const int8_t xSlant = gsl::narrow_cast<int8_t>(*floorData & 0xff);
                const auto absX = std::abs(xSlant);
                const int8_t zSlant = gsl::narrow_cast<int8_t>((*floorData >> 8) & 0xff);
                const auto absZ = std::abs(zSlant);
                if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
                {
                    if( absX <= 2 && absZ <= 2 )
                        hi.slantClass = SlantClass::Max512;
                    else
                        hi.slantClass = SlantClass::Steep;

                    const irr::f32 localX = pos.X % loader::SectorSize;
                    const irr::f32 localZ = pos.Z % loader::SectorSize;

                    if( zSlant > 0 ) // lower edge at -Z
                    {
                        auto dist = loader::SectorSize - localZ;
                        hi.distance += dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                    else if( zSlant < 0 ) // lower edge at +Z
                    {
                        auto dist = localZ;
                        hi.distance -= dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }

                    if( xSlant > 0 ) // lower edge at -X
                    {
                        auto dist = loader::SectorSize - localX;
                        hi.distance += dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                    else if( xSlant < 0 ) // lower edge at +X
                    {
                        auto dist = localX;
                        hi.distance -= dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                }
            }
            // Fall-through
        case loader::FDFunction::CeilingSlant:
        case loader::FDFunction::PortalSector:
            ++floorData;
            break;
        case loader::FDFunction::Death:
            hi.lastTriggerOrKill = floorData - 1;
            break;
        case loader::FDFunction::Trigger:
            if( !hi.lastTriggerOrKill )
                hi.lastTriggerOrKill = floorData - 1;
            ++floorData;
            while( true )
            {
                const bool isLastTrigger = loader::isLastFloordataEntry(*floorData);

                const auto func = loader::extractTriggerFunction(*floorData);
                const auto param = loader::extractTriggerFunctionParam(*floorData);
                ++floorData;

                if( func != loader::TriggerFunction::Object )
                {
                    if( func == loader::TriggerFunction::CameraTarget )
                    {
                        ++floorData;
                    }
                }
                else
                {
                    BOOST_ASSERT(func == loader::TriggerFunction::Object);
                    //! @todo Query height patch from object @c param, e.g. trapdoors or falling floor.
                }

                if( isLastTrigger )
                    break;
            }
        default:
            break;
        }
        if( isLast )
            break;
    }

    return hi;
}

HeightInfo HeightInfo::fromCeiling(const loader::Sector* roomSector, const loader::TRCoordinates& pos, const TRCameraSceneNodeAnimator* camera)
{
    HeightInfo hi;

    BOOST_ASSERT(roomSector != nullptr);
    while( roomSector->roomAbove != 0xff )
    {
        BOOST_ASSERT(roomSector->roomAbove < camera->getLevel()->m_rooms.size());
        auto room = &camera->getLevel()->m_rooms[roomSector->roomAbove];
        roomSector = room->getSectorByAbsolutePosition(pos);
    }

    hi.distance = roomSector->ceilingHeight * loader::QuarterSectorSize;

    if( roomSector->floorDataIndex == 0 )
    {
        return hi;
    }

    const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
    while( true )
    {
        const bool isLast = loader::isLastFloordataEntry(*floorData);
        const auto currentFd = *floorData;
        ++floorData;
        switch( loader::extractFDFunction(currentFd) )
        {
        case loader::FDFunction::CeilingSlant:
            {
                const int8_t xSlant = gsl::narrow_cast<int8_t>(*floorData & 0xff);
                const auto absX = std::abs(xSlant);
                const int8_t zSlant = gsl::narrow_cast<int8_t>((*floorData >> 8) & 0xff);
                const auto absZ = std::abs(zSlant);
                if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
                {
                    const irr::f32 localX = pos.X % loader::SectorSize;
                    const irr::f32 localZ = pos.Z % loader::SectorSize;

                    if( zSlant > 0 ) // lower edge at -Z
                    {
                        auto dist = loader::SectorSize - localZ;
                        hi.distance -= dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                    else if( zSlant < 0 ) // lower edge at +Z
                    {
                        auto dist = localZ;
                        hi.distance += dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }

                    if( xSlant > 0 ) // lower edge at -X
                    {
                        auto dist = localX;
                        hi.distance -= dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                    else if( xSlant < 0 ) // lower edge at +X
                    {
                        auto dist = loader::SectorSize - localX;
                        hi.distance += dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                }
            }
            // Fall-through
        case loader::FDFunction::FloorSlant:
        case loader::FDFunction::PortalSector:
            ++floorData;
            break;
        case loader::FDFunction::Death:
            break;
        case loader::FDFunction::Trigger:
            ++floorData;
            while( true )
            {
                const bool isLastTrigger = loader::isLastFloordataEntry(*floorData);

                const auto func = loader::extractTriggerFunction(*floorData);
                const auto param = loader::extractTriggerFunctionParam(*floorData);
                ++floorData;

                if( func != loader::TriggerFunction::Object )
                {
                    if( func == loader::TriggerFunction::CameraTarget )
                    {
                        ++floorData;
                    }
                }
                else
                {
                    BOOST_ASSERT(func == loader::TriggerFunction::Object);
                    //! @todo Query height patch from object @c param.
                }

                if( isLastTrigger )
                    break;
            }
        default:
            break;
        }
        if( isLast )
            break;
    }

    return hi;
}
