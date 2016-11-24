#pragma once

#include "Node.h"
#include "Light.h"
#include "Model.h"
#include "Drawable.h"


namespace gameplay
{
    class Camera;

    /**
     * Defines the root container for a hierarchy of Node objects.
     *
     * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Scene
     */
    class Scene
    {
    public:
        explicit Scene();
        virtual ~Scene();

        /**
         * Adds the specified node to the scene.
         *
         * @param node The node to be added to the scene.
         */
        void addNode(const std::shared_ptr<Node>& node);

        /**
         * Removes the specified node from the scene.
         *
         * @param node The node to remove.
         */
        void removeNode(const std::shared_ptr<Node>& node);

        /**
         * Removes all nodes from the scene.
         */
        void removeAllNodes();

        /**
         * Returns the number of nodes at the root level of the scene.
         *
         * @return The node count.
         */
        size_t getNodeCount() const;

        /**
         * Gets the active camera for the scene.
         *
         * @return The active camera for the scene.
         * @see VisibleSet#getActiveCamera
         */
        const std::shared_ptr<Camera>& getActiveCamera() const;

        /**
         * Sets the active camera on the scene.
         *
         * @param camera The active camera to be set on the scene.
         */
        void setActiveCamera(const std::shared_ptr<Camera>& camera);

        void accept(Visitor& visitor)
        {
            for(auto& node : _nodes)
                visitor.visit(*node);
        }
    private:

        /**
         * Hidden copy constructor.
         */
        Scene(const Scene& copy) = delete;

        Scene& operator=(const Scene&) = delete;

        std::shared_ptr<Camera> _activeCamera = nullptr;
        Node::List _nodes;
    };
}
