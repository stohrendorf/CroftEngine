#pragma once

#include "Model.h"
#include "Light.h"

#include <glm/gtc/matrix_transform.hpp>

namespace gameplay
{
    class Drawable;
    class Light;
    class Scene;


    /**
     * Defines a hierarchical structure of objects in 3D transformation spaces.
     *
     * This object allow you to attach components to a scene such as:
     * Drawable's(Model, Camera, Light, PhysicsCollisionObject, AudioSource, etc.
     *
     * @see http://gameplay3d.github.io/GamePlay/docs/file-formats.html#wiki-Node
     */
    class Node : public std::enable_shared_from_this<Node>
    {
        friend class Light;
        friend class Scene;

    public:
        using List = std::vector<std::shared_ptr<Node>>;

        explicit Node(const std::string& id);
        virtual ~Node();

        /**
         * Gets the identifier for the node.
         *
         * @return The node identifier.
         */
        const std::string& getId() const;

        /**
         * Sets the identifier for the node.
         *
         * @param id The identifier to set for the node.
         */
        void setId(const std::string& id);

        /**
         * Adds a child node.
         *
         * @param child The child to add.
         */
        void addChild(const std::shared_ptr<Node>& child);

        /**
         * Removes a child node.
         *
         * @param child The child to remove.
         */
        void removeChild(const std::shared_ptr<Node>& child);

        /**
         * Removes all child nodes.
         */
        void removeAllChildren();

        /**
         * Returns the parent of this node.
         *
         * @return The parent.
         */
        const std::weak_ptr<Node>& getParent() const;

        /**
         * Returns the number of direct children of this item.
         *
         * @return The number of children.
         */
        size_t getChildCount() const;

        /**
         * Gets the top level node in this node's parent hierarchy.
         */
        Node* getRootNode() const;

        /**
         * Returns the first child node that matches the given ID.
         *
         * This method checks the specified ID against its immediate child nodes
         * but does not check the ID against itself.
         * If recursive is true, it also traverses the Node's hierarchy with a breadth first search.
         *
         * @param id The ID of the child to find.
         * @param recursive True to search recursively all the node's children, false for only direct children.
         * @param exactMatch true if only nodes whose ID exactly matches the specified ID are returned,
         *        or false if nodes that start with the given ID are returned.
         *
         * @return The Node found or NULL if not found.
         */
        std::shared_ptr<Node> findNode(const std::string& id, bool recursive = true, bool exactMatch = true) const;

        /**
         * Returns all child nodes that match the given ID.
         *
         * @param id The ID of the node to find.
         * @param nodes A vector of nodes to be populated with matches.
         * @param recursive true if a recursive search should be performed, false otherwise.
         * @param exactMatch true if only nodes whose ID exactly matches the specified ID are returned,
         *        or false if nodes that start with the given ID are returned.
         *
         * @return The number of matches found.
         * @script{ignore}
         */
        size_t findNodes(const std::string& id, Node::List& nodes, bool recursive = true, bool exactMatch = true) const;

        /**
         * Gets the scene this node is currenlty within.
         *
         * @return The scene.
         */
        virtual Scene* getScene() const;

        /**
         * Sets if the node is enabled in the scene.
         *
         * @param enabled if the node is enabled in the scene.
         */
        void setEnabled(bool enabled);

        /**
         * Gets if the node is enabled in the scene.
         *
         * @return if the node is enabled in the scene.
         */
        bool isEnabled() const;

        /**
         * Gets if the node inherently enabled.
         *
         * @return if components attached on this node should be running.
         */
        bool isEnabledInHierarchy() const;

        /**
         * Gets the world matrix corresponding to this node.
         *
         * @return The world matrix of this node.
         */
        virtual const glm::mat4& getWorldMatrix() const;

        /**
         * Gets the world view matrix corresponding to this node.
         *
         * @return The world view matrix of this node.
         */
        glm::mat4 getWorldViewMatrix() const;

        /**
         * Gets the inverse transpose world matrix corresponding to this node.
         *
         * This matrix is typically used to transform normal vectors into world space.
         *
         * @return The inverse world matrix of this node.
         */
        glm::mat4 getInverseTransposeWorldMatrix() const;

        /**
         * Gets the inverse transpose world view matrix corresponding to this node.
         *
         * This matrix is typically used to transform normal vectors into view space.
         *
         * @return The inverse world view matrix of this node.
         */
        glm::mat4 getInverseTransposeWorldViewMatrix() const;

        /**
         * Gets the view matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The view matrix of this node.
         */
        const glm::mat4& getViewMatrix() const;

        /**
         * Gets the inverse view matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The inverse view matrix of this node.
         */
        const glm::mat4& getInverseViewMatrix() const;

        /**
         * Gets the projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The projection matrix of this node.
         */
        const glm::mat4& getProjectionMatrix() const;

        /**
         * Gets the view * projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The view * projection matrix of this node.
         */
        const glm::mat4& getViewProjectionMatrix() const;

        /**
         * Gets the inverse view * projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The inverse view * projection matrix of this node.
         */
        const glm::mat4& getInverseViewProjectionMatrix() const;

        /**
         * Gets the world * view * projection matrix corresponding to this node based
         * on the scene's active camera.
         *
         * @return The world * view * projection matrix of this node.
         */
        glm::mat4 getWorldViewProjectionMatrix() const;

        /**
         * Gets the translation vector (or position) of this Node in world space.
         *
         * @return The world translation vector.
         */
        glm::vec3 getTranslationWorld() const;

        /**
         * Gets the translation vector (or position) of this Node in view space.
         *
         * @return The view space translation vector.
         */
        glm::vec3 getTranslationView() const;

        /**
         * Returns the translation vector of the currently active camera for this node's scene.
         *
         * @return The translation vector of the scene's active camera.
         */
        glm::vec3 getActiveCameraTranslationWorld() const;

        /**
         * Gets the drawable object attached to this node.
         *
         * @return The drawable component attached to this node.
         */
        const std::shared_ptr<Drawable>& getDrawable() const;

        /**
         * Set the drawable object to be attached to this node
         *
         * This is typically a Model, ParticleEmiiter, Form, Terrrain, Sprite, TileSet or Text.
         *
         * This will increase the reference count of the new drawble and decrease
         * the reference count of the old drawable.
         *
         * @param drawable The new drawable component. May be NULL.
         */
        void setDrawable(const std::shared_ptr<Drawable>& drawable);

        /**
         * Get the light attached to this node.
         *
         * @return The light attached to this node.
         */
        const std::shared_ptr<Light>& getLight() const;

        /**
         * Attaches a light to this node.
         *
         * This will increase the reference count of the new light and decrease
         * the reference count of the old light.
         *
         * @param light The new light. May be NULL.
         */
        void setLight(const std::shared_ptr<Light>& light);

        const List& getChildren() const
        {
            return _children;
        }

        const glm::mat4& getLocalMatrix() const
        {
            return m_localMatrix;
        }

        void setLocalMatrix(const glm::mat4& m)
        {
            m_localMatrix = m;
            transformChanged();
        }

    protected:

        /**
         * Removes this node from its parent.
         */
        void remove();

        /**
         * Called when this Node's transform changes.
         */
        void transformChanged();

        /**
         * Called when this Node's hierarchy changes.
         */
        void hierarchyChanged();

        /**
         * Marks the bounding volume of the node as dirty.
         */
        void setBoundsDirty();

    private:

        Node(const Node& copy) = delete;

        Node& operator=(const Node&) = delete;

    protected:

        /** The scene this node is attached to. */
        Scene* _scene = nullptr;
        /** The nodes id. */
        std::string _id;

        List _children;

        /** The nodes parent. */
        std::weak_ptr<Node> _parent{};

        /** If this node is enabled. Maybe different if parent is enabled/disabled. */
        bool _enabled = true;
        /** The drawble component attached to this node. */
        std::shared_ptr<Drawable> _drawable = nullptr;
        /** The light component attached to this node. */
        std::shared_ptr<Light> _light = nullptr;
        /** The dirty bits used for optimization. */
        mutable int _dirtyBits;

        glm::mat4 m_localMatrix{ 1.0f };
        mutable glm::mat4 m_worldMatrix{ 1.0f };
    };
}
