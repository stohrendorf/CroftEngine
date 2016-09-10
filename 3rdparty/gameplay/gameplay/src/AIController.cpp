#include "Base.h"
#include "AIController.h"
#include "Game.h"


namespace gameplay
{
    AIController::AIController()
        : _paused(false)
        , _firstMessage()
        , _firstAgent()
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
        _firstAgent.clear();

        // Remove all messages
        _firstMessage.clear();
    }


    void AIController::pause()
    {
        _paused = true;
    }


    void AIController::resume()
    {
        _paused = false;
    }


    void AIController::sendMessage(const std::shared_ptr<AIMessage>& message, float delay)
    {
        if( delay <= 0 )
        {
            // Send instantly
            if( message->getReceiver() == nullptr || strlen(message->getReceiver()) == 0 )
            {
                // Broadcast message to all agents
                for(const auto& agent : _firstAgent)
                {
                    if(agent->processMessage(message))
                        break; // message consumed by this agent - stop bubbling
                }
            }
            else
            {
                // Single recipient
                auto agent = findAgent(message->getReceiver());
                if( agent )
                {
                    agent->processMessage(message);
                }
                else
                {
                    GP_WARN("Failed to locate AIAgent for message recipient: %s", message->getReceiver());
                }
            }
        }
        else
        {
            // Queue for later delivery
            _firstMessage.push_front(message);
        }
    }


    void AIController::update(const std::chrono::microseconds& elapsedTime)
    {
        if( _paused )
            return;

        static Game* game = Game::getInstance();

        // Send all pending messages that have expired
        for(auto msg = _firstMessage.begin(); msg != _firstMessage.end(); ++msg)
        {
            // If the message delivery time has expired, send it (this also deletes it)
            if( (*msg)->getDeliveryTime() >= game->getGameTime() )
            {
                sendMessage(*msg);
                _firstMessage.erase(msg);
            }
        }

        // Update all enabled agents
        for( const auto& agent : _firstAgent )
        {
            if( agent->isEnabled() )
                agent->update(elapsedTime);
        }
    }


    void AIController::addAgent(const std::shared_ptr<AIAgent>& agent)
    {
        _firstAgent.push_front(agent);
    }


    void AIController::removeAgent(const std::shared_ptr<AIAgent>& agent)
    {
        auto itr = std::find(_firstAgent.begin(), _firstAgent.end(), agent);
        if(itr != _firstAgent.end())
            _firstAgent.erase(itr);
    }


    std::shared_ptr<AIAgent> AIController::findAgent(const std::string& id) const
    {
        for( const auto& agent : _firstAgent )
        {
            if( id == agent->getId() )
                return agent;
        }

        return nullptr;
    }
}
