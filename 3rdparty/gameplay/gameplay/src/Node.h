#pragma once

#include "Transform.h"
#include "Model.h"
#include "Camera.h"
#include "Light.h"


namespace gameplay
{
    class Camera;
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
    class Node : public Transform
    {
        friend class Light;
        friend class MeshSkin;
        friend class Scene;

    public:
        using List = std::vector<std::shared_ptr<Node>>;

        explicit Node(const std::string& id);
        virtual ~Node();


        /**
         * Defines the types of nodes.
         */
        enum Type
        {
            NODE = 1,
            JOINT
        };


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
        unsigned int findNodes(const std::string& id, Node::List& nodes, bool recursive = true, bool exactMatch = true) const;

        /**
         * Gets the scene this node is currenlty within.
         *
         * @return The scene.
         */
        virtual Scene* getScene() const;

        /**
         * Sets a tag on this Node.
         *
         * tags can be used for a variety of purposes within a game. For example,
         * a tag called "transparent" can be added to nodes, to indicate which nodes in
         * a scene are transparent. This tag can then be read during rendering to sort
         * transparent and opaque objects for correct drawing order.
         *
         * Setting a tag to NULL removes the tag from the Node.
         *
         * @param name Name of the tag to set.
         * @param value Optional value of the tag (empty string by default).
         */
        void setTag(const std::string& name, const std::string& value = {});
        void clearTag(const std::string& name);

        /**
         * Returns the value of the custom tag with the given name.
         *
         * @param name Name of the tag to return.
         *
         * @return The value of the given tag, or NULL if the tag is not set.
         */
        const char* getTag(const std::string& name) const;

        /**
         * Determines if a custom tag with the specified name is set.
         *
         * @param name Name of the tag to query.
         *
         * @return true if the tag is set, false otherwise.
         */
        bool hasTag(const std::string& name) const;

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
         * Called to update the state of this Node.
         *
         * This method is called by Scene::update(float) to update the state of all active
         * nodes in a scene. A Node is considered active if Node::isActive() returns true.
         *
         * @param elapsedTime Elapsed time in milliseconds.
         */
        void update(float elapsedTime);

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
         * Returns the view-space translation vector of the currently active camera for this node's scene.
         *
         * @return The translation vector of the scene's active camera, in view-space.
         */
        glm::vec3 getActiveCameraTranslationView() const;

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
         * Gets the camera attached to this node.
         *
         * @return Gets the camera attached to this node.
         */
        const std::shared_ptr<Camera>& getCamera() const;

        /**
         * Attaches a camera to this node.
         *
         * This will increase the reference count of the new camera and decrease
         * the reference count of the old camera.
         *
         * @param camera The new camera. May be NULL.
         */
        void setCamera(const std::shared_ptr<Camera>& camera);

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

        /**
         * Gets the user object assigned to this node.
         *
         * @return The user object assigned object to this node.
         */
        void* getUserObject() const;

        /**
        * Sets a user object to be assigned object to this node.
        *
        * @param obj The user object assigned object to this node.
        */
        void setUserObject(void* obj);


        void resetRotationPatch()
        {
            _rotationPatch = glm::quat();
            dirty(DIRTY_ROTATION);
        }


        void setRotationPatch(const glm::quat& q)
        {
            _rotationPatch = q;
            dirty(DIRTY_ROTATION);
        }


        const List& getChildren() const
        {
            return _children;
        }


    protected:

        /**
         * Removes this node from its parent.
         */
        void remove();

        /**
         * Called when this Node's transform changes.
         */
        void transformChanged() override;

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
        Scene* _scene;
        /** The nodes id. */
        std::string _id;

        List _children;

        /** The nodes parent. */
        std::weak_ptr<Node> _parent;

        /** If this node is enabled. Maybe different if parent is enabled/disabled. */
        bool _enabled;
        /** Tags assigned to this node. */
        std::map<std::string, std::string> _tags;
        /** The drawble component attached to this node. */
        std::shared_ptr<Drawable> _drawable;
        /** The camera component attached to this node. */
        std::shared_ptr<Camera> _camera;
        /** The light component attached to this node. */
        std::shared_ptr<Light> _light;
        /** The user object component attached to this node. */
        void* _userObject;
        /** The world matrix for this node. */
        mutable glm::mat4 _world;
        /** The dirty bits used for optimization. */
        mutable int _dirtyBits;
    };
}
