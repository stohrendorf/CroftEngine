#pragma once

#include "io/util.h"
#include "core/angle.h"
#include "core/id.h"

namespace loader
{
namespace file
{
struct Item
{
    core::TypeId type{
        uint16_t( 0 )
    }; //!< Object Identifier (matched in AnimatedModels[], or SpriteSequences[], as appropriate)
    core::RoomId16 room{ uint16_t( 0 ) }; //!< Owning room
    core::TRVec position; //!< world coords
    core::Angle rotation; //!< ((0xc000 >> 14) * 90) degrees around Y axis
    int16_t darkness; //!< (constant lighting; -1 means use mesh lighting)
    int16_t intensity2 = 0; //!< Like Intensity1, and almost always with the same value. [absent from TR1 data files]
    int16_t ocb = 0; //!< Object code bit - used for altering entity behaviour. Only in TR4-5.

    uint16_t activationState = 0;

    float getBrightness() const
    {
        return 1.0f - darkness / 8191.0f;
    }

    static std::unique_ptr<Item> readTr1(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ std::make_unique<Item>() };
        item->type = reader.readU16();
        item->room = reader.readU16();
        item->position = readCoordinates32( reader );
        item->rotation = core::auToAngle( reader.readI16() );
        item->darkness = reader.readI16();
        item->activationState = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr2(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ std::make_unique<Item>() };
        item->type = reader.readU16();
        item->room = reader.readU16();
        item->position = readCoordinates32( reader );
        item->rotation = core::auToAngle( reader.readI16() );
        item->darkness = reader.readI16();
        item->intensity2 = reader.readU16();
        item->activationState = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr3(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ std::make_unique<Item>() };
        item->type = reader.readU16();
        item->room = reader.readU16();
        item->position = readCoordinates32( reader );
        item->rotation = core::auToAngle( reader.readI16() );
        item->darkness = reader.readU16();
        item->intensity2 = reader.readU16();
        item->activationState = reader.readU16();
        return item;
    }

    static std::unique_ptr<Item> readTr4(io::SDLReader& reader)
    {
        std::unique_ptr<Item> item{ std::make_unique<Item>() };
        item->type = reader.readU16();
        item->room = reader.readU16();
        item->position = readCoordinates32( reader );
        item->rotation = core::auToAngle( reader.readI16() );
        item->darkness = reader.readU16();
        item->intensity2 = item->darkness;
        item->ocb = reader.readU16();
        item->activationState = reader.readU16();
        return item;
    }
};
}
}
