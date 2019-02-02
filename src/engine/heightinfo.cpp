#include "heightinfo.h"

#include "cameracontroller.h"
#include "level/level.h"

namespace engine
{
bool HeightInfo::skipSteepSlants = false;

HeightInfo HeightInfo::fromFloor(gsl::not_null<const loader::Sector*> roomSector,
                                 const core::TRVec& pos,
                                 const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList)
{
    HeightInfo hi;

    hi.slantClass = SlantClass::None;

    while( roomSector->roomBelow != nullptr )
    {
        roomSector = roomSector->roomBelow->getSectorByAbsolutePosition( pos );
    }

    hi.y = roomSector->floorHeight * loader::QuarterSectorSize;
    hi.lastCommandSequenceOrDeath = nullptr;

    if( roomSector->floorData == nullptr )
    {
        return hi;
    }

    const engine::floordata::FloorDataValue* fd = roomSector->floorData;
    while( true )
    {
        const floordata::FloorDataChunk chunkHeader{*fd++};
        switch( chunkHeader.type )
        {
            case floordata::FloorDataChunkType::FloorSlant:
            {
                const auto xSlant = gsl::narrow_cast<int8_t>( fd->get() & 0xff );
                const auto absX = std::abs( xSlant );
                const auto zSlant = gsl::narrow_cast<int8_t>( (fd->get() >> 8) & 0xff );
                const auto absZ = std::abs( zSlant );
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
                        const auto dist = loader::SectorSize - localZ;
                        hi.y += dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                    else if( zSlant < 0 ) // lower edge at +Z
                    {
                        const auto dist = localZ;
                        hi.y -= dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }

                    if( xSlant > 0 ) // lower edge at -X
                    {
                        const auto dist = loader::SectorSize - localX;
                        hi.y += dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                    else if( xSlant < 0 ) // lower edge at +X
                    {
                        const auto dist = localX;
                        hi.y -= dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                    }
                }
            }
                // Fall-through
            case floordata::FloorDataChunkType::CeilingSlant:
            case floordata::FloorDataChunkType::PortalSector:
                ++fd;
                break;
            case floordata::FloorDataChunkType::Death:
                hi.lastCommandSequenceOrDeath = fd - 1;
                break;
            case floordata::FloorDataChunkType::CommandSequence:
                if( hi.lastCommandSequenceOrDeath == nullptr )
                    hi.lastCommandSequenceOrDeath = fd - 1;
                ++fd;
                while( true )
                {
                    const floordata::Command command{*fd++};

                    if( command.opcode == floordata::CommandOpcode::Activate )
                    {
                        itemList.at( command.parameter )->patchFloor( pos, hi.y );
                    }
                    else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
                    {
                        command.isLast = floordata::CameraParameters{*fd++}.isLast;
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

HeightInfo HeightInfo::fromCeiling(gsl::not_null<const loader::Sector*> roomSector,
                                   const core::TRVec& pos,
                                   const std::map<uint16_t, gsl::not_null<std::shared_ptr<items::ItemNode>>>& itemList)
{
    HeightInfo hi;

    while( roomSector->roomAbove != nullptr )
    {
        roomSector = roomSector->roomAbove->getSectorByAbsolutePosition( pos );
    }

    hi.y = roomSector->ceilingHeight * loader::QuarterSectorSize;

    if( roomSector->floorData != nullptr )
    {
        const engine::floordata::FloorDataValue* fd = roomSector->floorData;
        floordata::FloorDataChunk chunkHeader{*fd};
        ++fd;

        if( chunkHeader.type == floordata::FloorDataChunkType::FloorSlant )
        {
            ++fd;

            chunkHeader = floordata::FloorDataChunk{*fd};
            ++fd;
        }

        if( chunkHeader.type == floordata::FloorDataChunkType::CeilingSlant )
        {
            const auto xSlant = gsl::narrow_cast<int8_t>( fd->get() & 0xff );
            const auto absX = std::abs( xSlant );
            const auto zSlant = gsl::narrow_cast<int8_t>( (fd->get() >> 8) & 0xff );
            const auto absZ = std::abs( zSlant );
            if( !skipSteepSlants || (absX <= 2 && absZ <= 2) )
            {
                const auto localX = pos.X % loader::SectorSize;
                const auto localZ = pos.Z % loader::SectorSize;

                if( zSlant > 0 ) // lower edge at -Z
                {
                    const auto dist = loader::SectorSize - localZ;
                    hi.y -= dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                }
                else if( zSlant < 0 ) // lower edge at +Z
                {
                    const auto dist = localZ;
                    hi.y += dist * zSlant * loader::QuarterSectorSize / loader::SectorSize;
                }

                if( xSlant > 0 ) // lower edge at -X
                {
                    const auto dist = localX;
                    hi.y -= dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                }
                else if( xSlant < 0 ) // lower edge at +X
                {
                    const auto dist = loader::SectorSize - localX;
                    hi.y += dist * xSlant * loader::QuarterSectorSize / loader::SectorSize;
                }
            }
        }
    }

    while( roomSector->roomBelow != nullptr )
    {
        roomSector = roomSector->roomBelow->getSectorByAbsolutePosition( pos );
    }

    if( roomSector->floorData == nullptr )
        return hi;

    const engine::floordata::FloorDataValue* fd = roomSector->floorData;
    while( true )
    {
        const floordata::FloorDataChunk chunkHeader{*fd};
        ++fd;
        switch( chunkHeader.type )
        {
            case floordata::FloorDataChunkType::CeilingSlant:
            case floordata::FloorDataChunkType::FloorSlant:
            case floordata::FloorDataChunkType::PortalSector:
                ++fd;
                break;
            case floordata::FloorDataChunkType::Death:
                break;
            case floordata::FloorDataChunkType::CommandSequence:
                ++fd;
                while( true )
                {
                    const floordata::Command command{*fd++};

                    if( command.opcode == floordata::CommandOpcode::Activate )
                    {
                        itemList.at( command.parameter )->patchCeiling( pos, hi.y );
                    }
                    else if( command.opcode == floordata::CommandOpcode::SwitchCamera )
                    {
                        command.isLast = floordata::CameraParameters{*fd++}.isLast;
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
