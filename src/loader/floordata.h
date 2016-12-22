#pragma once

#include <vector>


namespace loader
{
    using FloorData = std::vector<uint16_t>;


    //! @brief Native TR floor data functions
    //! @ingroup native
    enum class FloorDataChunkType : uint8_t
    {
        PortalSector = 0x01,
        FloorSlant = 0x02,
        CeilingSlant = 0x03,
        CommandSequence = 0x04,
        Death = 0x05,
        Climb = 0x06,
        FloorTriangleNW = 0x07, //  [_\_]
        FloorTriangleNE = 0x08, //  [_/_]
        CeilingTriangleNW = 0x09, //  [_/_]
        CeilingTriangleNE = 0x0A, //  [_\_]
        FloorTriangleNWPortalSW = 0x0B, //  [P\_]
        FloorTriangleNWPortalNE = 0x0C, //  [_\P]
        FloorTriangleNEPortalSE = 0x0D, //  [_/P]
        FloorTriangleNEPortalNW = 0x0E, //  [P/_]
        CeilingTriangleNWPortalSW = 0x0F, //  [P\_]
        CeilingTriangleNWPortalNE = 0x10, //  [_\P]
        CeilingTriangleNEPortalNW = 0x11, //  [P/_]
        CeilingTriangleNEPortalSE = 0x12, //  [_/P]
        Monkey = 0x13,
        MinecartLeft = 0x14, // In TR3 only. Function changed in TR4+.
        MinecartRight = 0x15 // In TR3 only. Function changed in TR4+.
    };


    //! @brief Native trigger types.
    //! @ingroup native
    //! @see FloorDataChunkType::Always
    //! @see Command
    enum class SequenceCondition
    {
        Always = 0x00, //!< If Lara is in sector, run (any case).
        Pad = 0x01, //!< If Lara is in sector, run (land case).
        Switch = 0x02, //!< If item is activated, run, else stop.
        Key = 0x03, //!< If item is activated, run.
        Pickup = 0x04, //!< If item is picked up, run.
        Heavy = 0x05, //!< If item is in sector, run, else stop.
        AntiPad = 0x06, //!< If Lara is in sector, stop (land case).
        Combat = 0x07, //!< If Lara is in combat state, run (any case).
        Dummy = 0x08, //!< If Lara is in sector, run (air case).
        AntiTrigger = 0x09, //!< TR2-5 only: If Lara is in sector, stop (any case).
        HeavySwitch = 0x0A, //!< TR3-5 only: If item is activated by item, run.
        HeavyAntiTrigger = 0x0B, //!< TR3-5 only: If item is activated by item, stop.
        Monkey = 0x0C, //!< TR3-5 only: If Lara is monkey-swinging, run.
        Skeleton = 0x0D, //!< TR5 only: Activated by skeleton only?
        TightRope = 0x0E, //!< TR5 only: If Lara is on tightrope, run.
        CrawlDuck = 0x0F, //!< TR5 only: If Lara is crawling, run.
        Climb = 0x10, //!< TR5 only: If Lara is climbing, run.
    };


    //! @brief Native trigger function types.
    //! @ingroup native
    //! @see FloorDataChunkType::Always
    //! @see SequenceCondition
    enum class Command
    {
        Activate = 0x00,
        SwitchCamera = 0x01,
        UnderwaterCurrent = 0x02,
        FlipMap = 0x03,
        FlipOn = 0x04,
        FlipOff = 0x05,
        LookAt = 0x06,
        EndLevel = 0x07,
        PlayTrack = 0x08,
        FlipEffect = 0x09,
        Secret = 0x0A,
        ClearBodies = 0x0B, // Unused in TR4
        FlyBy = 0x0C,
        CutScene = 0x0D,
        Command_E,
        Command_F
    };


    struct FloorDataChunkHeader
    {
        explicit FloorDataChunkHeader(FloorData::value_type fd)
            : isLast{extractIsLast(fd)}
            , sequenceCondition{extractSequenceCondition(fd)}
            , type{extractType(fd)}
        {
        }


        bool isLast;
        SequenceCondition sequenceCondition;
        FloorDataChunkType type;


        static FloorDataChunkType extractType(FloorData::value_type data)
        {
            return gsl::narrow_cast<FloorDataChunkType>(data & 0xff);
        }


    private:
        static SequenceCondition extractSequenceCondition(FloorData::value_type data)
        {
            return gsl::narrow_cast<SequenceCondition>((data & 0x3f00) >> 8);
        }


        static constexpr bool extractIsLast(FloorData::value_type data)
        {
            return (data & 0x8000) != 0;
        }
    };


    struct FloorDataCommandSequenceHeader
    {
        static constexpr const uint16_t Oneshot = 0x100;
        static constexpr const uint16_t ActivationMask = 0x3e00;
        static constexpr const uint16_t InvertedActivation = 0x4000;
        static constexpr const uint16_t Locked = 0x8000;


        explicit FloorDataCommandSequenceHeader(FloorData::value_type fd)
            : timeout{gsl::narrow_cast<uint8_t>(fd & 0xff)}
            , oneshot{(fd & Oneshot) != 0}
            , inverted{(fd & InvertedActivation) != 0}
            , locked{(fd & Locked) != 0}
            , activationMask{gsl::narrow_cast<uint16_t>(fd & ActivationMask)}
        {
        }


        const uint8_t timeout;
        const bool oneshot;
        const bool inverted;
        const bool locked;
        const uint16_t activationMask;
    };


    struct FloorDataCameraParameters
    {
        explicit FloorDataCameraParameters(FloorData::value_type fd)
            : timeout{gsl::narrow_cast<uint8_t>(fd & 0xff)}
            , oneshot{(fd & 0x100) != 0}
            , isLast{(fd & 0x8000) != 0}
            , smoothness{gsl::narrow_cast<uint8_t>((fd >> 8) & 0x3e)}
        {
        }


        const uint8_t timeout;
        const bool oneshot;
        const bool isLast;
        const uint8_t smoothness;
    };


    struct FloorDataCommandHeader
    {
        explicit FloorDataCommandHeader(FloorData::value_type fd)
            : isLast{extractIsLast(fd)}
            , command{extractCommand(fd)}
            , parameter{extractParameter(fd)}
        {
        }


        mutable bool isLast;
        Command command;
        uint16_t parameter;

    private:
        static Command extractCommand(FloorData::value_type data)
        {
            return gsl::narrow_cast<Command>((data >> 10) & 0x0f);
        }


        static constexpr uint16_t extractParameter(FloorData::value_type data)
        {
            return data & 0x3ff;
        }


        static constexpr bool extractIsLast(FloorData::value_type data)
        {
            return (data & 0x8000) != 0;
        }
    };
}
