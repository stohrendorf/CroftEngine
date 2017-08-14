#pragma once

#include "Node.h"


namespace gameplay
{
    class Camera;


    /**
     * Defines the root container for a hierarchy of Node objects.
     */
    class Scene final
    {
    public:
        explicit Scene() = default;
        ~Scene() = default;


        /**
         * Adds the specified node to the scene.
         *
         * @param node The node to be added to the scene.
         */
        void addNode(const std::shared_ptr<Node>& node)
        {
            BOOST_ASSERT(node);

            if( node->m_scene == this )
            {
                // The node is already a member of this scene.
                return;
            }

            _nodes.push_back(node);
            node->m_scene = this;
        }


        /**
         * Returns the number of nodes at the root level of the scene.
         *
         * @return The node count.
         */
        size_t getNodeCount() const
        {
            return _nodes.size();
        }


        /**
         * Gets the active camera for the scene.
         *
         * @return The active camera for the scene.
         */
        const std::shared_ptr<Camera>& getActiveCamera() const
        {
            return _activeCamera;
        }


        /**
         * Sets the active camera on the scene.
         *
         * @param camera The active camera to be set on the scene.
         */
        void setActiveCamera(const std::shared_ptr<Camera>& camera)
        {
            _activeCamera = camera;
        }


        void accept(Visitor& visitor)
        {
            for( auto& node : _nodes )
                visitor.visit(*node);
        }


    private:

        Scene(const Scene& copy) = delete;

        Scene& operator=(const Scene&) = delete;

        std::shared_ptr<Camera> _activeCamera = nullptr;
        Node::List _nodes;
    };
}
