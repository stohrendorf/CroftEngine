#pragma once

#include "game.h"

#include "io/sdlreader.h"

namespace loader
{
    // In TR3-5, there were 5 reverb / echo effect flags for each
    // room, but they were never used in PC versions - however, level
    // files still contain this info, so we now can re-use these flags
    // to assign reverb/echo presets to each room->
    // Also, underwater environment can be considered as additional
    // reverb flag, so overall amount is 6.

    enum class ReverbType : uint8_t
    {
        Outside, // EFX_REVERB_PRESET_CITY
        SmallRoom, // EFX_REVERB_PRESET_LIVINGROOM
        MediumRoom, // EFX_REVERB_PRESET_WOODEN_LONGPASSAGE
        LargeRoom, // EFX_REVERB_PRESET_DOME_TOMB
        Pipe, // EFX_REVERB_PRESET_PIPE_LARGE
        Water, // EFX_REVERB_PRESET_UNDERWATER
        Sentinel
    };


    /** \brief SoundSource.
    *
    * This structure contains the details of continuous-sound sources. Although
    * a SoundSource object has a position, it has no room membership; the sound
    * seems to propagate omnidirectionally for about 10 horizontal-grid sizes
    * without regard for the presence of walls.
    */
    struct SoundSource
    {
        int32_t x; // absolute X position of sound source (world coordinates)
        int32_t y; // absolute Y position of sound source (world coordinates)
        int32_t z; // absolute Z position of sound source (world coordinates)
        uint16_t sound_id; // internal sound index
        uint16_t flags; // 0x40, 0x80, or 0xc0

        static std::unique_ptr<SoundSource> read(io::SDLReader& reader)
        {
            std::unique_ptr<SoundSource> sound_source{new SoundSource()};
            sound_source->x = reader.readI32();
            sound_source->y = reader.readI32();
            sound_source->z = reader.readI32();

            sound_source->sound_id = reader.readU16();
            sound_source->flags = reader.readU16();
            return sound_source;
        }
    };

    // Looped field is located at offset 6 of SoundDetail structure and
    // combined with SampleIndexes value. This field is responsible for
    // looping behaviour of each sound.
    // L flag sets sound to continous looped state, while W flag waits
    // for any sound with similar ID to finish, and only then plays it
    // again. R flag rewinds sound, if sound with similar ID is being
    // sent to sources.
    enum class LoopType
    {
        None,
        Forward,
        PingPong,
        Wait
    };

    /** \brief SoundDetails.
    *
    * SoundDetails (also called SampleInfos in native TR sources) are properties
    * for each sound index from SoundMap. It contains all crucial information
    * that is needed to play certain sample, except offset to raw wave buffer,
    * which is unnecessary, as it is managed internally by DirectSound.
    */
    struct SoundDetails
    {
        size_t sample; // Index into SampleIndices -- NOT USED IN TR4-5!!!
        uint16_t volume; // Global sample value
        uint16_t sound_range; // Sound range
        uint16_t chance; // Chance to play
        int16_t pitch; // Pitch shift
        uint8_t num_samples_and_flags_1; // Bits 0-1: Looped flag, bits 2-5: num samples, bits 6-7: UNUSED
        uint8_t flags_2; // Bit 4: UNKNOWN, bit 5: Randomize pitch, bit 6: randomize volume
        // All other bits in flags_2 are unused.

        LoopType getLoopType(Engine engine) const
        {
            if( engine == Engine::TR1 )
            {
                switch( num_samples_and_flags_1 & 3 )
                {
                case 1:
                    return LoopType::PingPong;
                case 2:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
                }
            }
            else if( engine == Engine::TR2 )
            {
                switch( num_samples_and_flags_1 & 3 )
                {
                case 1:
                    return LoopType::PingPong;
                case 3:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
                }
            }
            else
            {
                switch( num_samples_and_flags_1 & 3 )
                {
                case 1:
                    return LoopType::Wait;
                case 2:
                    return LoopType::PingPong;
                case 3:
                    return LoopType::Forward;
                default:
                    return LoopType::None;
                }
            }
        }

        uint8_t getSampleCount() const
        {
            return (num_samples_and_flags_1 >> 2) & 0x0f;
        }

        bool useRandomPitch() const
        {
            return (flags_2 & 0x20) != 0;
        }

        bool useRandomVolume() const
        {
            return (flags_2 & 0x40) != 0;
        }

        // Default range and pitch values are required for compatibility with
        // TR1 and TR2 levels, as there is no such parameters in SoundDetails
        // structures.

        static constexpr const int DefaultRange = 8;
        //! @todo Check default value
        static constexpr const int DefaultPitch = 128; // 0.0 - only noise

        static std::unique_ptr<SoundDetails> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<SoundDetails> sound_details{new SoundDetails()};
            sound_details->sample = reader.readU16();
            sound_details->volume = reader.readU16();
            sound_details->chance = reader.readU16();
            sound_details->num_samples_and_flags_1 = reader.readU8();
            sound_details->flags_2 = reader.readU8();
            sound_details->sound_range = DefaultRange;
            sound_details->pitch = DefaultPitch;
            return sound_details;
        }

        static std::unique_ptr<SoundDetails> readTr3(io::SDLReader& reader)
        {
            std::unique_ptr<SoundDetails> sound_details{new SoundDetails()};
            sound_details->sample = reader.readU16();
            sound_details->volume = reader.readU8();
            sound_details->sound_range = reader.readU8();
            sound_details->chance = reader.readU8();
            sound_details->pitch = reader.readI8();
            sound_details->num_samples_and_flags_1 = reader.readU8();
            sound_details->flags_2 = reader.readU8();
            return sound_details;
        }
    };
}
