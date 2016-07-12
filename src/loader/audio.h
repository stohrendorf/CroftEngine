#pragma once

#include "io/sdlreader.h"
#include "level/game.h"

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

    enum class PlaybackType
    {
        //! Play the sample once, then release the resources
        None,
        //! Loop the sample
        Looping,
        //! Restart already playing sample
        Restart,
        //! Sample cannot be played more than once at the same time
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
        // Default range and pitch values are required for compatibility with
        // TR1 and TR2 levels, as there is no such parameters in SoundDetails
        // structures.

        static constexpr const int DefaultRange = 8;
        //! @todo Check default value
        static constexpr const int DefaultPitch = 128; // 0.0 - only noise

        size_t sample; // Index into SampleIndices -- NOT USED IN TR4-5!!!
        uint16_t volume; // Global sample value
        uint16_t sound_range = DefaultRange; // Sound range
        uint16_t chance; // Chance to play
        int16_t pitch = DefaultPitch; // Pitch shift
        uint8_t sampleCountAndLoopType; // Bits 0-1: Looped flag, bits 2-5: num samples, bits 6-7: UNUSED
        uint8_t flags;

        PlaybackType getPlaybackType(level::Engine engine) const
        {
            if( engine == level::Engine::TR1 )
            {
                switch( sampleCountAndLoopType & 3 )
                {
                case 1:
                    return PlaybackType::Restart;
                case 2:
                    return PlaybackType::Looping;
                default:
                    return PlaybackType::None;
                }
            }
            else if( engine == level::Engine::TR2 )
            {
                switch( sampleCountAndLoopType & 3 )
                {
                case 1:
                    return PlaybackType::Restart;
                case 3:
                    return PlaybackType::Looping;
                default:
                    return PlaybackType::None;
                }
            }
            else
            {
                switch( sampleCountAndLoopType & 3 )
                {
                case 1:
                    return PlaybackType::Wait;
                case 2:
                    return PlaybackType::Restart;
                case 3:
                    return PlaybackType::Looping;
                default:
                    return PlaybackType::None;
                }
            }
        }

        uint8_t getSampleCount() const
        {
            return (sampleCountAndLoopType >> 2) & 0x0f;
        }

        //! @brief Whether to play this sample without orientation (no panning).
        bool ignoreOrientation() const
        {
            return (flags & 0x10) != 0;
        }

        bool useRandomPitch() const
        {
            return (flags & 0x20) != 0;
        }

        bool useRandomVolume() const
        {
            return (flags & 0x40) != 0;
        }

        static std::unique_ptr<SoundDetails> readTr1(io::SDLReader& reader)
        {
            std::unique_ptr<SoundDetails> sound_details{new SoundDetails()};
            sound_details->sample = reader.readU16();
            sound_details->volume = reader.readU16();
            sound_details->chance = reader.readU16();
            sound_details->sampleCountAndLoopType = reader.readU8();
            sound_details->flags = reader.readU8();
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
            sound_details->sampleCountAndLoopType = reader.readU8();
            sound_details->flags = reader.readU8();
            return sound_details;
        }
    };
}
