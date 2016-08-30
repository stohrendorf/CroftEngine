#include "Base.h"
#include "AIController.h"
#include "Game.h"


namespace gameplay
{
    AIController::AIController()
        : _paused(false)
        , _firstMessage(nullptr)
        , _firstAgent(nullptr)
    {
    }


    AIController::~AIController()
    {
    }


    void AIController::initialize()
    {
    }


    void AIController::finalize()
    {
        // Remove all agents
        AIAgent* agent = _firstAgent;
        while( agent )
        {
            AIAgent* temp = agent;
            agent = agent->_next;
            SAFE_RELEASE(temp);
        }
        _firstAgent = nullptr;

        // Remove all messages
        AIMessage* message = _firstMessage;
        while( message )
        {
            AIMessage* temp = message;
            message = message->_next;
            AIMessage::destroy(temp);
        }
        _firstMessage = nullptr;
    }


    void AIController::pause()
    {
        _paused = true;
    }


    void AIController::resume()
    {
        _paused = false;
    }


    void AIController::sendMessage(AIMessage* message, float delay)
    {
        if( delay <= 0 )
        {
            // Send instantly
            if( message->getReceiver() == nullptr || strlen(message->getReceiver()) == 0 )
            {
                // Broadcast message to all agents
                AIAgent* agent = _firstAgent;
                while( agent )
                {
                    if( agent->processMessage(message) )
                        break; // message consumed by this agent - stop bubbling
                    agent = agent->_next;
                }
            }
            else
            {
                // Single recipient
                AIAgent* agent = findAgent(message->getReceiver());
                if( agent )
                {
                    agent->processMessage(message);
                }
                else
                {
                    GP_WARN("Failed to locate AIAgent for message recipient: %s", message->getReceiver());
                }
            }

            // Delete the message, since it is finished being processed
            AIMessage::destroy(message);
        }
        else
        {
            // Queue for later delivery
            if( _firstMessage )
                message->_next = _firstMessage;
            _firstMessage = message;
        }
    }


    void AIController::update(const std::chrono::microseconds& elapsedTime)
    {
        if( _paused )
            return;

        static Game* game = Game::getInstance();

        // Send all pending messages that have expired
        AIMessage* prevMsg = nullptr;
        AIMessage* msg = _firstMessage;
        while( msg )
        {
            // If the message delivery time has expired, send it (this also deletes it)
            if( msg->getDeliveryTime() >= game->getGameTime() )
            {
                // Link the message out of our list
                if( prevMsg )
                    prevMsg->_next = msg->_next;

                AIMessage* temp = msg;
                msg = msg->_next;
                temp->_next = nullptr;
                sendMessage(temp);
            }
            else
            {
                prevMsg = msg;
                msg = msg->_next;
            }
        }

        // Update all enabled agents
        AIAgent* agent = _firstAgent;
        while( agent )
        {
            if( agent->isEnabled() )
                agent->update(elapsedTime);

            agent = agent->_next;
        }
    }


    void AIController::addAgent(AIAgent* agent)
    {
        agent->addRef();

        if( _firstAgent )
            agent->_next = _firstAgent;

        _firstAgent = agent;
    }


    void AIController::removeAgent(AIAgent* agent)
    {
        // Search our linked list of agents and link this agent out.
        AIAgent* prevAgent = nullptr;
        AIAgent* itr = _firstAgent;
        while( itr )
        {
            if( itr == agent )
            {
                if( prevAgent )
                    prevAgent->_next = agent->_next;
                else
                    _firstAgent = agent->_next;

                agent->_next = nullptr;
                agent->release();
                break;
            }

            prevAgent = itr;
            itr = itr->_next;
        }
    }


    AIAgent* AIController::findAgent(const std::string& id) const
    {
        AIAgent* agent = _firstAgent;
        while( agent )
        {
            if( id == agent->getId() )
                return agent;

            agent = agent->_next;
        }

        return nullptr;
    }
}
