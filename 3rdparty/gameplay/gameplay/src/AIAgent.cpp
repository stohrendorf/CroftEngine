#include "Base.h"
#include "AIAgent.h"
#include "Node.h"


namespace gameplay
{
    AIAgent::AIAgent()
        : _stateMachine(nullptr)
        , _node(nullptr)
        , _enabled(true)
        , _listener(nullptr)
    {
        _stateMachine = new AIStateMachine(this);
    }


    AIAgent::~AIAgent()
    {
        SAFE_DELETE(_stateMachine);
    }


    std::shared_ptr<AIAgent> AIAgent::create()
    {
        return std::make_shared<AIAgent>();
    }


    const std::string& AIAgent::getId() const
    {
        if( _node )
            return _node->getId();

        static const std::string empty;

        return empty;
    }


    Node* AIAgent::getNode() const
    {
        return _node;
    }


    void AIAgent::setNode(Node* node)
    {
        _node = node;
    }


    AIStateMachine* AIAgent::getStateMachine() const
    {
        return _stateMachine;
    }


    bool AIAgent::isEnabled() const
    {
        return (_node && _enabled);
    }


    void AIAgent::setEnabled(bool enabled)
    {
        _enabled = enabled;
    }


    void AIAgent::setListener(Listener* listener)
    {
        _listener = listener;
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void AIAgent::update(const std::chrono::microseconds& elapsedTime)
    {
        _stateMachine->update(elapsedTime);
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    bool AIAgent::processMessage(const std::shared_ptr<AIMessage>& message)
    {
        // Handle built-in message types.
        switch( message->_messageType )
        {
            case AIMessage::MESSAGE_TYPE_STATE_CHANGE:
            {
                // Change state message
                const char* stateId = message->getString(0);
                if( stateId )
                {
                    auto state = _stateMachine->getState(stateId);
                    if( state )
                        _stateMachine->setStateInternal(state);
                }
            }
                break;
            case AIMessage::MESSAGE_TYPE_CUSTOM:
                break;
        }

        // Dispatch message to registered listener.
        if( _listener && _listener->messageReceived(message) )
            return true;

        return false;
    }
}
