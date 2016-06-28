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
        int16_t intensity1; //!< (constant lighting; -1 means use mesh lighting)
        int16_t intensity2; //!< Like Intensity1, and almost always with the same value. [absent from TR1 data files]
        int16_t ocb; //!< Object code bit - used for altering entity behaviour. Only in TR4-5.

        /**
        * @brief Flags
        *
        * @details
        * @li 0x0100 indicates "initially invisible"
        * @li 0x3e00 is Activation Mask
        * @li 0x3e00 indicates "open" or "activated"
        *
        * These can be XORed with related FloorData::FDlist fields (e.g. for switches).
        */
        uint16_t flags;

        uint16_t getActivationMask() const
        {
            return (flags & 0x3e00);
        }

        uint8_t getTriggerMask() const
        {
            return (flags >> 9) & 0x1f;
        }

        bool getEventBit() const
        {
            return ((flags >> 14) & 1) != 0;
        }

        bool getLockBit() const
        {
            return ((flags >> 15) & 1) != 0;
        }

        bool isInitiallyInvisible() const
        {
            return (flags & 0x0100) != 0;
        }

        /// \brief reads an item definition.
        static std::unique_ptr<Item> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{new Item()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = io::readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->intensity1 = reader.readU16();
            if( item->intensity1 >= 0 )
                item->intensity1 = (8191 - item->intensity1) << 2;
            item->intensity2 = item->intensity1;
            item->ocb = 0; // Not present in TR1!
            item->flags = reader.readU16();
            return item;
        }

        static std::unique_ptr<Item> readTr2(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{new Item()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = io::readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->intensity1 = reader.readU16();
            if( item->intensity1 >= 0 )
                item->intensity1 = (8191 - item->intensity1) << 2;
            item->intensity2 = reader.readU16();
            if( item->intensity2 >= 0 )
                item->intensity2 = (8191 - item->intensity2) << 2;
            item->ocb = 0; // Not present in TR2!
            item->flags = reader.readU16();
            return item;
        }

        static std::unique_ptr<Item> readTr3(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{new Item()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = io::readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->intensity1 = reader.readU16();
            item->intensity2 = reader.readU16();
            item->ocb = 0; // Not present in TR3!
            item->flags = reader.readU16();
            return item;
        }

        static std::unique_ptr<Item> readTr4(io::SDLReader& reader)
        {
            std::unique_ptr<Item> item{new Item()};
            item->type = reader.readU16();
            item->room = reader.readU16();
            item->position = io::readCoordinates32(reader);
            item->rotation = reader.readI16();
            item->intensity1 = reader.readU16();
            item->intensity2 = item->intensity1;
            item->ocb = reader.readU16();
            item->flags = reader.readU16();
            return item;
        }
    };
}
