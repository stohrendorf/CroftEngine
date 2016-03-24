#pragma once

#include "world/animation/animids.h"

#include <irrlicht.h>

#include <cstdint>

enum class AxisMovement
{
    Positive,
    Null,
    Negative
};

enum class LaraState : uint16_t
{
    WalkForward = 0,
    RunForward = 1,
    Stop = 2,
    JumpForward = 3,
    Pose = 4,                // Derived from leaked TOMB.MAP
    RunBack = 5,
    TurnRightSlow = 6,
    TurnLeftSlow = 7,
    Death = 8,
    FreeFall = 9,
    Hang = 10,
    Reach = 11,
    //    UNUSED2 12
    UnderwaterStop = 13,
    GrabToFall = 14,
    JumpPrepare = 15,
    WalkBackward = 16,
    UnderwaterForward = 17,
    UnderwaterInertia = 18,
    Climbing = 19,
    TurnFast = 20,
    StepRight = 21,
    StepLeft = 22,
    RollBackward = 23,
    SlideForward = 24,
    JumpBack = 25,
    JumpLeft = 26,
    JumpRight = 27,
    JumpUp = 28,
    FallBackward = 29,
    ShimmyLeft = 30,
    ShimmyRight = 31,
    SlideBackward = 32,
    OnWaterStop = 33,
    OnWaterForward = 34,
    UnderwaterDiving = 35,
    PushablePush = 36,
    PushablePull = 37,
    PushableGrab = 38,
    PickUp = 39,
    SwitchDown = 40,
    SwitchUp = 41,
    InsertKey = 42,
    InsertPuzzle = 43,
    WaterDeath = 44,
    RollForward = 45,
    BoulderDeath = 46,
    OnWaterBackward = 47,
    OnWaterLeft = 48,
    OnWaterRight = 49,
    UseMidas = 50,          //  Derived from leaked TOMB.MAP
    MidasDeath = 51,          //  Derived from leaked TOMB.MAP
    SwandiveBegin = 52,
    SwandiveEnd = 53,
    Handstand = 54,
    OnWaterExit = 55,
    LadderIdle = 56,
    LadderUp = 57,
    LadderLeft = 58,
    //    UNUSED5 59
    LadderRight = 60,
    LadderDown = 61,
    //    UNUSED6 62
    //    UNUSED7 63
    //    UNUSED8 64
    WadeForward = 65,
    UnderwaterTurnAround = 66,
    FlarePickUp = 67,
    JumpRoll = 68,
    //    UNUSED10 69
    ZiplineRide = 70,
    CrouchIdle = 71,
    CrouchRoll = 72,
    Sprint = 73,
    SprintRoll = 74,
    MonkeyswingIdle = 75,
    MonkeyswingForward = 76,
    MonkeyswingLeft = 77,
    MonkeyswingRight = 78,
    MonkeyswingTurnAround = 79,
    CrawlIdle = 80,
    CrawlForward = 81,
    MonkeyswingTurnLeft = 82,
    MonkeyswingTurnRight = 83,
    CrawlTurnLeft = 84,
    CrawlTurnRight = 85,
    CrawlBackward = 86,
    ClimbToCrawl = 87,
    CrawlToClimb = 88,
    MiscControl = 89,
    RopeTurnLeft = 90,
    RopeTurnRight = 91,
    GiantButtonPush = 92,
    TrapdoorFloorOpen = 93,
    //    UNUSED11 94
    RoundHandle = 95,
    CogWheel = 96,
    LeverSwitchPush = 97,
    Hole = 98,
    PoleIdle = 99,
    PoleUp = 100,
    PoleDown = 101,
    PoleTurnLeft = 102,
    PoleTurnRight = 103,
    Pulley = 104,
    CrouchTurnLeft = 105,
    CrouchTurnRight = 106,
    ClimbOuterCornerLeft = 107,
    ClimbOuterCornerRight = 108,
    ClimbInnerCornerLeft = 109,
    ClimbInnerCornerRight = 110,
    RopeIdle = 111,
    RopeClimbUp = 112,
    RopeClimbDown = 113,
    RopeSwing = 114,
    LadderToHands = 115,
    PositionCorrector = 116,
    DoubledoorsPush = 117,
    Dozy = 118,
    TightropeIdle = 119,
    TightropeTurnAround = 120,
    TightropeForward = 121,
    TightropeBalancingLeft = 122,
    TightropeBalancingRight = 123,
    TightropeEnter = 124,
    TightropeExit = 125,
    DoveSwitch = 126,
    TightropeRestoreBalance = 127,
    BarsSwing = 128,
    BarsJump = 129,
    //    UNUSED12 130
    RadioListening = 131,
    RadioOff = 132,
    //    UNUSED13 133
    //    UNUSED14 134
    //    UNUSED15 135
    //    UNUSED16 136
    PickUpFromChest = 137
};

class LaraStateHandler final : public irr::scene::ISceneNodeAnimator
{
private:
    const loader::Level* const m_level;
    std::shared_ptr<loader::DefaultAnimDispatcher> m_dispatcher;
    const std::string m_name;
    irr::u32 m_lastActiveFrame = std::numeric_limits<irr::u32>::max();

    AxisMovement m_xMovement = AxisMovement::Null;
    AxisMovement m_zMovement = AxisMovement::Null;
    bool m_jump = false;
    bool m_moveSlow = false;
    
public:
    LaraStateHandler(const loader::Level* level, const std::shared_ptr<loader::DefaultAnimDispatcher>& dispatcher, const std::string& name)
        : m_level(level), m_dispatcher(dispatcher), m_name(name)
    {
        BOOST_ASSERT(level != nullptr);
        BOOST_ASSERT(dispatcher != nullptr);
        playAnimation(world::animation::TR_ANIMATION_LARA_STAY_IDLE);
    }
    
    ~LaraStateHandler() = default;

    virtual void animateNode(irr::scene::ISceneNode* node, irr::u32 /*timeMs*/) override
    {
        BOOST_ASSERT(node->getType() == irr::scene::ESNT_ANIMATED_MESH);
        irr::scene::IAnimatedMeshSceneNode* animNode = static_cast<irr::scene::IAnimatedMeshSceneNode*>(node);
        
        BOOST_ASSERT(animNode->getFrameNr() >= 0);
        const auto currentFrame = static_cast<irr::u32>(animNode->getFrameNr());
        if(currentFrame == m_lastActiveFrame)
            return;
        
        m_lastActiveFrame = currentFrame;
        switch(static_cast<LaraState>(m_dispatcher->getCurrentState()))
        {
            case LaraState::Stop:
                if(m_jump)
                {
                    setTargetState(LaraState::JumpPrepare);
                }
                else if(m_zMovement == AxisMovement::Positive)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkForward);
                    else
                        setTargetState(LaraState::RunForward);
                }
                else if(m_zMovement == AxisMovement::Negative)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkBackward);
                    else
                        setTargetState(LaraState::RunBack);
                }
                else if(m_xMovement == AxisMovement::Positive)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::StepRight);
                    else
                        setTargetState(LaraState::TurnRightSlow);
                }
                else if(m_xMovement == AxisMovement::Negative)
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::StepLeft);
                    else
                        setTargetState(LaraState::TurnLeftSlow);
                }
                break;
            case LaraState::RunForward:
                if(m_zMovement == AxisMovement::Positive) // continue running
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkForward);
                    else if(m_jump)
                        setTargetState(LaraState::JumpForward);
                    else
                        setTargetState(LaraState::RunForward);
                }
                else
                {
                    setTargetState(LaraState::Stop);
                }
                break;
            case LaraState::RunBack:
                if(m_zMovement == AxisMovement::Negative) // continue running
                {
                    if(m_moveSlow)
                        setTargetState(LaraState::WalkBackward);
                    else if(m_jump)
                        setTargetState(LaraState::JumpBack);
                    else
                        setTargetState(LaraState::RunBack);
                }
                else
                {
                    setTargetState(LaraState::Stop);
                }
                break;
            case LaraState::JumpForward:
                if(m_moveSlow)
                {
                    setTargetState(LaraState::SwandiveBegin);
                }
                else if(m_zMovement == AxisMovement::Positive)
                {
                    setTargetState(LaraState::RunForward);
                }
                else
                {
                    setTargetState(LaraState::Stop);
                }
                break;
            case LaraState::JumpPrepare:
                if(m_zMovement == AxisMovement::Positive)
                {
                    setTargetState(LaraState::JumpForward);
                }
                else if(m_zMovement == AxisMovement::Negative)
                {
                    setTargetState(LaraState::JumpBack);
                }
                else if(m_xMovement == AxisMovement::Positive)
                {
                    setTargetState(LaraState::JumpRight);
                }
                else if(m_xMovement == AxisMovement::Negative)
                {
                    setTargetState(LaraState::JumpLeft);
                }
                break;
            case LaraState::JumpUp:
                setTargetState(LaraState::FreeFall);
                break;
            case LaraState::FreeFall:
                playAnimation(world::animation::TR_ANIMATION_LARA_LANDING_HARD);
                break;
            default:
                BOOST_LOG_TRIVIAL(debug) << "Unhandled state: " << m_dispatcher->getCurrentState();
        }
    }

    virtual ISceneNodeAnimator* createClone(irr::scene::ISceneNode* /*node*/, irr::scene::ISceneManager* /*newManager*/ = nullptr) override
    {
        BOOST_ASSERT(false);
        return nullptr;
    }
    
    void setXAxisMovement(bool left, bool right)
    {
        if(left < right)
            m_xMovement = AxisMovement::Positive;
        else if(left > right)
            m_xMovement = AxisMovement::Negative;
        else
            m_xMovement = AxisMovement::Null;
    }
    
    void setZAxisMovement(bool back, bool forward)
    {
        if(back < forward)
            m_zMovement = AxisMovement::Positive;
        else if(back > forward)
            m_zMovement = AxisMovement::Negative;
        else
            m_zMovement = AxisMovement::Null;
    }
    
    void setJump(bool val)
    {
        m_jump = val;
    }
    
    void setMoveSlow(bool val)
    {
        m_moveSlow = val;
    }
    
private:
    void setTargetState(LaraState st)
    {
        m_dispatcher->setTargetState(static_cast<uint16_t>(st));
    }
    
    void playAnimation(uint16_t anim)
    {
        m_dispatcher->playLocalAnimation(anim);
    }
};
