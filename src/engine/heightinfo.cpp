#include "heightinfo.h"

#include "cameracontroller.h"
#include "level/level.h"


namespace engine
{
    bool HeightInfo::skipSteepSlants = false;


    HeightInfo HeightInfo::fromFloor(gsl::not_null<const loader::Sector*> roomSector, const core::TRCoordinates& pos, const CameraController* camera)
    {
        HeightInfo hi;

        hi.slantClass = SlantClass::None;

        while( roomSector->roomBelow != nullptr )
        {
            roomSector = roomSector->roomBelow->getSectorByAbsolutePosition(pos);
        }

        hi.distance = roomSector->floorHeight * loader::QuarterSectorSize;
        hi.lastCommandSequenceOrDeath = nullptr;

        if( roomSector->floorDataIndex == 0 )
        {
            return hi;
        }

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while( true )
        {
            const floordata::FloorDataChunk chunkHeader{*floorData++};
            switch( chunkHeader.type )
            {
                case floordata::FloorDataChunkType::FloorSlant:
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

                        const auto localX = pos.X % loader::SectorSize;
                        const auto localZ = pos.Z % loader::SectorSize;

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
                case floordata::FloorDataChunkType::CeilingSlant:
                case floordata::FloorDataChunkType::PortalSector:
                    ++floorData;
                    break;
                case floordata::FloorDataChunkType::Death:
                    hi.lastCommandSequenceOrDeath = floorData - 1;
                    break;
                case floordata::FloorDataChunkType::CommandSequence:
                    if( hi.lastCommandSequenceOrDeath == nullptr )
                        hi.lastCommandSequenceOrDeath = floorData - 1;
                    ++floorData;
                    while( true )
                    {
                        const floordata::Command command{*floorData++};

                        if( command.opcode == floordata::CommandOpcode::Activate )
                        {
                            auto it = camera->getLevel()->m_itemNodes.find(command.parameter);
                            Expects(it != camera->getLevel()->m_itemNodes.end());
                            it->second->patchFloor(pos, hi.distance);
                        }
                        else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
                        {
                            command.isLast = floordata::CameraParameters{ *floorData++ }.isLast;
                        }

                        if( command.isLast )
                            break;
                    }
                default:
                    break;
            }
            if( chunkHeader.isLast )
                break;
        }

        return hi;
    }


    HeightInfo HeightInfo::fromCeiling(gsl::not_null<const loader::Sector*> roomSector, const core::TRCoordinates& pos, const CameraController* camera)
    {
        HeightInfo hi;

        while( roomSector->roomAbove != nullptr )
        {
            roomSector = roomSector->roomAbove->getSectorByAbsolutePosition(pos);
        }

        hi.distance = roomSector->ceilingHeight * loader::QuarterSectorSize;

        if( roomSector->floorDataIndex != 0 )
        {
            const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
            floordata::FloorDataChunk chunkHeader{*floorData};
            ++floorData;

            if( chunkHeader.type == floordata::FloorDataChunkType::FloorSlant )
            {
                ++floorData;

                chunkHeader = floordata::FloorDataChunk{*floorData};
                ++floorData;
            }

            if( chunkHeader.type == floordata::FloorDataChunkType::CeilingSlant )
            {
                const int8_t xSlant = gsl::narrow_cast<int8_t>(*floorData & 0xff);
                const auto absX = std::abs(xSlant);
                const int8_t zSlant = gsl::narrow_cast<int8_t>((*floorData >> 8) & 0xff);
                const auto absZ = std::abs(zSlant);
                if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
                {
                    const auto localX = pos.X % loader::SectorSize;
                    const auto localZ = pos.Z % loader::SectorSize;

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
        }

        while(roomSector->roomBelow != nullptr)
        {
            roomSector = roomSector->roomBelow->getSectorByAbsolutePosition(pos);
        }

        if( roomSector->floorDataIndex == 0 )
            return hi;

        const uint16_t* floorData = &camera->getLevel()->m_floorData[roomSector->floorDataIndex];
        while( true )
        {
            floordata::FloorDataChunk chunkHeader{*floorData};
            ++floorData;
            switch( chunkHeader.type )
            {
                case floordata::FloorDataChunkType::CeilingSlant:
                case floordata::FloorDataChunkType::FloorSlant:
                case floordata::FloorDataChunkType::PortalSector:
                    ++floorData;
                    break;
                case floordata::FloorDataChunkType::Death:
                    break;
                case floordata::FloorDataChunkType::CommandSequence:
                    ++floorData;
                    while( true )
                    {
                        const floordata::Command command{*floorData++};

                        if( command.opcode == floordata::CommandOpcode::Activate )
                        {
                            auto it = camera->getLevel()->m_itemNodes.find(command.parameter);
                            Expects(it != camera->getLevel()->m_itemNodes.end());
                            it->second->patchCeiling(pos, hi.distance);
                        }
                        else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
                        {
                            command.isLast = floordata::CameraParameters{ *floorData++ }.isLast;
                        }

                        if( command.isLast )
                            break;
                    }
                default:
                    break;
            }
            if( chunkHeader.isLast )
                break;
        }

        return hi;
    }
}
