#pragma once

#include "Node.h"

namespace gameplay
{
    class Camera;


    class Scene final
    {
    public:
        Scene(const Scene& copy) = delete;

        Scene& operator=(const Scene&) = delete;

        explicit Scene() = default;

        ~Scene() = default;

        void addNode(const std::shared_ptr<Node>& node)
        {
            BOOST_ASSERT( node );

            if( node->m_scene == this )
            {
                // The node is already a member of this scene.
                return;
            }

            m_nodes.push_back( node );
            node->m_scene = this;
        }

        size_t getNodeCount() const
        {
            return m_nodes.size();
        }

        const std::shared_ptr<Camera>& getActiveCamera() const
        {
            return m_activeCamera;
        }

        void setActiveCamera(const std::shared_ptr<Camera>& camera)
        {
            m_activeCamera = camera;
        }

        void accept(Visitor& visitor)
        {
            for( auto& node : m_nodes )
                visitor.visit( *node );
        }

    private:
        std::shared_ptr<Camera> m_activeCamera = nullptr;
        Node::List m_nodes;
    };
}
