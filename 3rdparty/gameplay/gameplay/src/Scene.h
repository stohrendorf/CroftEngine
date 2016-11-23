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

        /**
         * Visits each node in the scene and calls the specified method pointer.
         *
         * Calling this method invokes the specified method pointer for each node
         * in the scene hierarchy.
         *
         * The visitMethod parameter must be a pointer to a method that has a bool
         * return type and accepts a single parameter of type Node*.
         *
         * A depth-first traversal of the scene continues while the visit method
         * returns true. Returning false will stop traversing further children for
         * the given node and the traversal will continue at the next sibling.
         *
         * @param instance The pointer to an instance of the object that contains visitMethod.
         * @param visitMethod The pointer to the class method to call for each node in the scene.
         */
        void visit(RenderContext& context, const std::function<bool(RenderContext&)>& callback)
        {
            for( const auto& node : _nodes )
            {
                context.setCurrentNode(node.get());
                visitNode(context, callback);
            }

            context.setCurrentNode(nullptr);
        }

    private:

        /**
         * Hidden copy constructor.
         */
        Scene(const Scene& copy) = delete;

        Scene& operator=(const Scene&) = delete;

        void visitNode(RenderContext& context, const std::function<bool(RenderContext&)>& callback)
        {
            if( !callback(context) )
                return;

            // Recurse for all children.
            for( const auto& child : context.getCurrentNode()->getChildren() )
            {
                context.setCurrentNode(child.get());
                visitNode(context, callback);
            }

            context.setCurrentNode(nullptr);
        }

        std::shared_ptr<Camera> _activeCamera = nullptr;
        Node::List _nodes;
    };
}
