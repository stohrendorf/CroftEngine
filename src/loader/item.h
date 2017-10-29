#pragma once

#include "datatypes.h"


namespace loader
{
    struct Item
    {
        uint16_t type; //!< Object Identifier (matched in AnimatedModels[], or SpriteSequences[], as appropriate)
        uint16_t room; //!< Owning room
        core::TRCoordinates position; //!< world coords
        int16_t rotation; //!< ((0xc000 >> 14) * 90) degrees around Y axis
        int16_t darkness; //!< (constant lighting; -1 means use mesh lighting)
        int16_t intensity2 = 0; //!< Like Intensity1, and almost always with the same value. [absent from TR1 data files]
        int16_t ocb = 0; //!< Object code bit - used for altering entity behaviour. Only in TR4-5.

        uint16_t activationState = 0;


        static std::unique_ptr<Item> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{std::make_unique<Item>()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->darkness = reader.readI16();
            item->activationState = reader.readU16();
            return item;
        }


        static std::unique_ptr<Item> readTr2(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{std::make_unique<Item>()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->darkness = reader.readI16();
            item->intensity2 = reader.readU16();
            item->activationState = reader.readU16();
            return item;
        }


        static std::unique_ptr<Item> readTr3(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{std::make_unique<Item>()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->darkness = reader.readU16();
            item->intensity2 = reader.readU16();
            item->activationState = reader.readU16();
            return item;
        }


        static std::unique_ptr<Item> readTr4(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{std::make_unique<Item>()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->darkness = reader.readU16();
            item->intensity2 = item->darkness;
            item->ocb = reader.readU16();
            item->activationState = reader.readU16();
            return item;
        }
    };
}
