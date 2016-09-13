#include "Base.h"
#include "AIStateMachine.h"
#include "AIAgent.h"
#include "AIMessage.h"
#include "Game.h"


namespace gameplay
{
    AIStateMachine::AIStateMachine(AIAgent* agent)
        : _agent(agent)
    {
        GP_ASSERT(agent);
        if( !AIState::_empty )
            AIState::_empty = std::make_shared<AIState>("");
        _currentState = AIState::_empty;
    }


    AIStateMachine::~AIStateMachine() = default;


    AIAgent* AIStateMachine::getAgent() const
    {
        return _agent;
    }


    std::shared_ptr<AIState> AIStateMachine::addState(const char* id)
    {
        auto state = AIState::create(id);
        _states.push_back(state);
        return state;
    }


    void AIStateMachine::addState(const std::shared_ptr<AIState>& state)
    {
        _states.push_back(state);
    }


    void AIStateMachine::removeState(const std::shared_ptr<AIState>& state)
    {
        auto itr = std::find(_states.begin(), _states.end(), state);
        if( itr != _states.end() )
        {
            _states.erase(itr);
        }
    }


    std::shared_ptr<AIState> AIStateMachine::getState(const char* id) const
    {
        GP_ASSERT(id);

        for(auto itr = _states.begin(); itr != _states.end(); ++itr)
        {
            auto state = (*itr);

            if( strcmp(id, state->getId()) == 0 )
                return state;
        }

        return nullptr;
    }


    const std::shared_ptr<AIState>& AIStateMachine::getActiveState() const
    {
        return _currentState;
    }


    bool AIStateMachine::hasState(const std::shared_ptr<AIState>& state) const
    {
        GP_ASSERT(state);

        return (std::find(_states.begin(), _states.end(), state) != _states.end());
    }


    std::shared_ptr<AIState> AIStateMachine::setState(const char* id)
    {
        auto state = getState(id);
        if( state )
            sendChangeStateMessage(state);
        return state;
    }


    bool AIStateMachine::setState(const std::shared_ptr<AIState>& state)
    {
        if( hasState(state) )
        {
            sendChangeStateMessage(state);
            return true;
        }

        return false;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void AIStateMachine::sendChangeStateMessage(const std::shared_ptr<AIState>& newState)
    {
        auto message = AIMessage::create(0, _agent->getId(), _agent->getId(), 1);
        message->_messageType = AIMessage::MESSAGE_TYPE_STATE_CHANGE;
        message->setString(0, newState->getId());
        Game::getInstance()->getAIController()->sendMessage(message);
    }


    void AIStateMachine::setStateInternal(const std::shared_ptr<AIState>& state)
    {
        GP_ASSERT(hasState(state));

        // Fire the exit event for the current state
        _currentState->exit(this);

        // Set the new state
        _currentState = state;

        // Fire the enter event for the new state
        _currentState->enter(this);
    }


    void AIStateMachine::update(const std::chrono::microseconds& elapsedTime)
    {
        _currentState->update(this, elapsedTime);
    }
}
