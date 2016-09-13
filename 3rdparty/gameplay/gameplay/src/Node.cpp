#include "Base.h"
#include "Node.h"
#include "Scene.h"
#include "Game.h"
#include "Drawable.h"
#include "Joint.h"

// Node dirty flags
#define NODE_DIRTY_WORLD 1
#define NODE_DIRTY_BOUNDS 2
#define NODE_DIRTY_HIERARCHY 4
#define NODE_DIRTY_ALL (NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS | NODE_DIRTY_HIERARCHY)


namespace gameplay
{
    Node::Node(const std::string& id)
        : _scene(nullptr)
        , _id(id)
        , _parent()
        , _enabled(true)
        , _tags(nullptr)
        , _drawable(nullptr)
        , _camera(nullptr)
        , _light(nullptr)
        , _agent(nullptr)
        , _userObject(nullptr)
        , _dirtyBits(NODE_DIRTY_ALL)
    {
    }


    Node::~Node()
    {
        removeAllChildren();
        if( _drawable )
            _drawable->setNode(nullptr);
        SAFE_DELETE(_tags);
        setAgent(nullptr);
    }


    const std::string& Node::getId() const
    {
        return _id;
    }


    void Node::setId(const std::string& id)
    {
        _id = id;
    }


    Node::Type Node::getType() const
    {
        return Node::NODE;
    }


    void Node::addChild(const std::shared_ptr<Node>& child)
    {
        GP_ASSERT(child);

        if( child->_parent.lock() == shared_from_this() )
        {
            // This node is already present in our hierarchy
            return;
        }

        // If the item belongs to another hierarchy, remove it first.
        if( !child->_parent.expired() )
        {
            child->_parent.lock()->removeChild(child);
        }
        else if( child->_scene )
        {
            child->_scene->removeNode(child);
        }

        _children.push_back(child);
        child->_parent = std::static_pointer_cast<Node>(shared_from_this());
        setBoundsDirty();

        if( _dirtyBits & NODE_DIRTY_HIERARCHY )
        {
            hierarchyChanged();
        }
    }


    void Node::removeChild(const std::shared_ptr<Node>& child)
    {
        if( child == nullptr || child->_parent.lock() != shared_from_this() )
        {
            // The child is not in our hierarchy.
            return;
        }
        // Call remove on the child.
        child->remove();
    }


    void Node::removeAllChildren()
    {
        _dirtyBits &= ~NODE_DIRTY_HIERARCHY;
        while( !_children.empty() )
        {
            removeChild(_children.back());
        }
        _dirtyBits |= NODE_DIRTY_HIERARCHY;
        hierarchyChanged();
    }


    void Node::remove()
    {
        // Update our parent.
        auto parent = _parent;
        if( !parent.expired() )
        {
            auto p = parent.lock();
            p->_children.erase(std::find(p->_children.begin(), p->_children.end(), shared_from_this()));
        }
        _parent.reset();

        if( !parent.expired() && parent.lock()->_dirtyBits & NODE_DIRTY_HIERARCHY )
        {
            parent.lock()->hierarchyChanged();
        }
    }


    const std::weak_ptr<Node>& Node::getParent() const
    {
        return _parent;
    }


    size_t Node::getChildCount() const
    {
        return _children.size();
    }


    Node* Node::getRootNode() const
    {
        Node* n = const_cast<Node*>(this);
        while( !n->getParent().expired() )
        {
            n = n->getParent().lock().get();
        }
        return n;
    }


    std::shared_ptr<Node> Node::findNode(const char* id, bool recursive, bool exactMatch) const
    {
        GP_ASSERT(id);

        // If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
        std::shared_ptr<Node> rootNode;
        auto model = std::dynamic_pointer_cast<Model>(_drawable);
        if( model )
        {
            if( model->getSkin() != nullptr && (rootNode = model->getSkin()->_rootNode) != nullptr )
            {
                if( (exactMatch && rootNode->_id == id) || (!exactMatch && rootNode->_id.find(id) == 0) )
                    return rootNode;

                auto match = rootNode->findNode(id, true, exactMatch);
                if( match )
                {
                    return match;
                }
            }
        }
        // Search immediate children first.
        for( const auto& child : _children )
        {
            // Does this child's ID match?
            if( (exactMatch && child->_id == id) || (!exactMatch && child->_id.find(id) == 0) )
            {
                return child;
            }
        }
        // Recurse.
        if( recursive )
        {
            for( const auto& child : _children )
            {
                auto match = child->findNode(id, true, exactMatch);
                if( match )
                {
                    return match;
                }
            }
        }
        return nullptr;
    }


    unsigned int Node::findNodes(const char* id, Node::List& nodes, bool recursive, bool exactMatch) const
    {
        GP_ASSERT(id);

        // If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
        unsigned int count = 0;
        std::shared_ptr<Node> rootNode;
        auto model = std::dynamic_pointer_cast<Model>(_drawable);
        if( model )
        {
            if( model->getSkin() != nullptr && (rootNode = model->getSkin()->_rootNode) != nullptr )
            {
                if( (exactMatch && rootNode->_id == id) || (!exactMatch && rootNode->_id.find(id) == 0) )
                {
                    nodes.push_back(rootNode);
                    ++count;
                }
                count += rootNode->findNodes(id, nodes, true, exactMatch);
            }
        }
        // Search immediate children first.
        for( const auto& child : _children )
        {
            // Does this child's ID match?
            if( (exactMatch && child->_id == id) || (!exactMatch && child->_id.find(id) == 0) )
            {
                nodes.push_back(child);
                ++count;
            }
        }
        // Recurse.
        if( recursive )
        {
            for( const auto& child : _children )
            {
                count += child->findNodes(id, nodes, true, exactMatch);
            }
        }

        return count;
    }


    Scene* Node::getScene() const
    {
        if( _scene )
            return _scene;

        // Search our parent for the scene
        if( !_parent.expired() )
        {
            Scene* scene = _parent.lock()->getScene();
            if( scene )
                return scene;
        }
        return nullptr;
    }


    bool Node::hasTag(const char* name) const
    {
        GP_ASSERT(name);
        return (_tags ? _tags->find(name) != _tags->end() : false);
    }


    const char* Node::getTag(const char* name) const
    {
        GP_ASSERT(name);

        if( !_tags )
            return nullptr;

        std::map<std::string, std::string>::const_iterator itr = _tags->find(name);
        return (itr == _tags->end() ? nullptr : itr->second.c_str());
    }


    void Node::setTag(const char* name, const char* value)
    {
        GP_ASSERT(name);

        if( value == nullptr )
        {
            // Removing tag
            if( _tags )
            {
                _tags->erase(name);
                if( _tags->size() == 0 )
                {
                    SAFE_DELETE(_tags);
                }
            }
        }
        else
        {
            // Setting tag
            if( _tags == nullptr )
            {
                _tags = new std::map<std::string, std::string>();
            }
            (*_tags)[name] = value;
        }
    }


    void Node::setEnabled(bool enabled)
    {
        _enabled = enabled;
    }


    bool Node::isEnabled() const
    {
        return _enabled;
    }


    bool Node::isEnabledInHierarchy() const
    {
        if( !_enabled )
            return false;

        auto node = _parent;
        while( !node.expired() )
        {
            if( !node.lock()->_enabled )
            {
                return false;
            }
            node = node.lock()->_parent;
        }
        return true;
    }


    void Node::update(float elapsedTime)
    {
        for( const auto& child : _children )
        {
            if( child->isEnabled() )
            {
                child->update(elapsedTime);
            }
        }
    }


    bool Node::isStatic() const
    {
        return false;
    }


    const Matrix& Node::getWorldMatrix() const
    {
        if( _dirtyBits & NODE_DIRTY_WORLD )
        {
            // Clear our dirty flag immediately to prevent this block from being entered if our
            // parent calls our getWorldMatrix() method as a result of the following calculations.
            _dirtyBits &= ~NODE_DIRTY_WORLD;

            if( !isStatic() )
            {
                // If we have a parent, multiply our parent world transform by our local
                // transform to obtain our final resolved world transform.
                auto parent = getParent();
                if( !parent.expired() )
                {
                    Matrix::multiply(parent.lock()->getWorldMatrix(), getMatrix(), &_world);
                }
                else
                {
                    _world = getMatrix();
                }

                // Our world matrix was just updated, so call getWorldMatrix() on all child nodes
                // to force their resolved world matrices to be updated.
                for( const auto& child : _children )
                {
                    child->getWorldMatrix();
                }
            }
        }
        return _world;
    }


    const Matrix& Node::getWorldViewMatrix() const
    {
        static Matrix worldView;
        Matrix::multiply(getViewMatrix(), getWorldMatrix(), &worldView);
        return worldView;
    }


    const Matrix& Node::getInverseTransposeWorldViewMatrix() const
    {
        static Matrix invTransWorldView;
        Matrix::multiply(getViewMatrix(), getWorldMatrix(), &invTransWorldView);
        invTransWorldView.invert();
        invTransWorldView.transpose();
        return invTransWorldView;
    }


    const Matrix& Node::getInverseTransposeWorldMatrix() const
    {
        static Matrix invTransWorld;
        invTransWorld = getWorldMatrix();
        invTransWorld.invert();
        invTransWorld.transpose();
        return invTransWorld;
    }


    const Matrix& Node::getViewMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getViewMatrix();
        }
        else
        {
            return Matrix::identity();
        }
    }


    const Matrix& Node::getInverseViewMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getInverseViewMatrix();
        }
        else
        {
            return Matrix::identity();
        }
    }


    const Matrix& Node::getProjectionMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getProjectionMatrix();
        }
        else
        {
            return Matrix::identity();
        }
    }


    const Matrix& Node::getViewProjectionMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getViewProjectionMatrix();
        }
        else
        {
            return Matrix::identity();
        }
    }


    const Matrix& Node::getInverseViewProjectionMatrix() const
    {
        Scene* scene = getScene();
        auto camera = scene ? scene->getActiveCamera() : nullptr;
        if( camera )
        {
            return camera->getInverseViewProjectionMatrix();
        }
        return Matrix::identity();
    }


    const Matrix& Node::getWorldViewProjectionMatrix() const
    {
        // Always re-calculate worldViewProjection matrix since it's extremely difficult
        // to track whether the camera has changed (it may frequently change every frame).
        static Matrix worldViewProj;
        Matrix::multiply(getViewProjectionMatrix(), getWorldMatrix(), &worldViewProj);
        return worldViewProj;
    }


    Vector3 Node::getTranslationWorld() const
    {
        Vector3 translation;
        getWorldMatrix().getTranslation(&translation);
        return translation;
    }


    Vector3 Node::getTranslationView() const
    {
        Vector3 translation;
        getWorldMatrix().getTranslation(&translation);
        getViewMatrix().transformPoint(&translation);
        return translation;
    }


    Vector3 Node::getForwardVectorWorld() const
    {
        Vector3 vector;
        getWorldMatrix().getForwardVector(&vector);
        return vector;
    }


    Vector3 Node::getForwardVectorView() const
    {
        Vector3 vector;
        getWorldMatrix().getForwardVector(&vector);
        getViewMatrix().transformVector(&vector);
        return vector;
    }


    Vector3 Node::getRightVectorWorld() const
    {
        Vector3 vector;
        getWorldMatrix().getRightVector(&vector);
        return vector;
    }


    Vector3 Node::getUpVectorWorld() const
    {
        Vector3 vector;
        getWorldMatrix().getUpVector(&vector);
        return vector;
    }


    Vector3 Node::getActiveCameraTranslationWorld() const
    {
        Scene* scene = getScene();
        if( scene )
        {
            auto camera = scene->getActiveCamera();
            if( camera )
            {
                auto cameraNode = camera->getNode();
                if( cameraNode )
                {
                    return cameraNode->getTranslationWorld();
                }
            }
        }
        return Vector3::zero();
    }


    Vector3 Node::getActiveCameraTranslationView() const
    {
        Scene* scene = getScene();
        if( scene )
        {
            auto camera = scene->getActiveCamera();
            if( camera )
            {
                auto cameraNode = camera->getNode();
                if( cameraNode )
                {
                    return cameraNode->getTranslationView();
                }
            }
        }
        return Vector3::zero();
    }


    void Node::hierarchyChanged()
    {
        // When our hierarchy changes our world transform is affected, so we must dirty it.
        _dirtyBits |= NODE_DIRTY_HIERARCHY;
        transformChanged();
    }


    void Node::transformChanged()
    {
        // Our local transform was changed, so mark our world matrices dirty.
        _dirtyBits |= NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS;

        // Notify our children that their transform has also changed (since transforms are inherited).
        for( const auto& child : _children )
        {
            if( Transform::isTransformChangedSuspended() )
            {
                // If the DIRTY_NOTIFY bit is not set
                if( !child->isDirty(Transform::DIRTY_NOTIFY) )
                {
                    child->transformChanged();
                    suspendTransformChange(child);
                }
            }
            else
            {
                child->transformChanged();
            }
        }
        Transform::transformChanged();
    }


    // ReSharper disable once CppMemberFunctionMayBeConst
    void Node::setBoundsDirty()
    {
        // Mark ourself and our parent nodes as dirty
        _dirtyBits |= NODE_DIRTY_BOUNDS;

        // Mark our parent bounds as dirty as well
        if( !_parent.expired() )
            _parent.lock()->setBoundsDirty();
    }


    const std::shared_ptr<Camera>& Node::getCamera() const
    {
        return _camera;
    }


    void Node::setCamera(const std::shared_ptr<Camera>& camera)
    {
        if( _camera == camera )
            return;

        if( _camera )
        {
            _camera->setNode(nullptr);
        }

        _camera = camera;

        if( _camera )
        {
            _camera->setNode(std::static_pointer_cast<Node>(shared_from_this()));
        }
    }


    const std::shared_ptr<Light>& Node::getLight() const
    {
        return _light;
    }


    void Node::setLight(const std::shared_ptr<Light>& light)
    {
        if( _light == light )
            return;

        if( _light )
        {
            _light->setNode(nullptr);
        }

        _light = light;

        if( _light )
        {
            _light->setNode(this);
        }

        setBoundsDirty();
    }


    const std::shared_ptr<Drawable>& Node::getDrawable() const
    {
        return _drawable;
    }


    void Node::setDrawable(const std::shared_ptr<Drawable>& drawable)
    {
        if( _drawable != drawable )
        {
            if( _drawable )
            {
                _drawable->setNode(nullptr);
            }

            _drawable = drawable;

            if( _drawable )
            {
                _drawable->setNode(this);
            }
        }
        setBoundsDirty();
    }


    const BoundingSphere& Node::getBoundingSphere() const
    {
        if( _dirtyBits & NODE_DIRTY_BOUNDS )
        {
            _dirtyBits &= ~NODE_DIRTY_BOUNDS;

            const Matrix& worldMatrix = getWorldMatrix();

            // Start with our local bounding sphere
            // TODO: Incorporate bounds from entities other than mesh (i.e. particleemitters, audiosource, etc)
            bool empty = true;
            auto model = std::dynamic_pointer_cast<Model>(_drawable);
            if( model && model->getMesh() )
            {
                if( empty )
                {
                    _bounds.set(model->getMesh()->getBoundingSphere());
                    empty = false;
                }
                else
                {
                    _bounds.merge(model->getMesh()->getBoundingSphere());
                }
            }
            if( _light )
            {
                switch( _light->getLightType() )
                {
                    case Light::POINT:
                        if( empty )
                        {
                            _bounds.set(Vector3::zero(), _light->getRange());
                            empty = false;
                        }
                        else
                        {
                            _bounds.merge(BoundingSphere(Vector3::zero(), _light->getRange()));
                        }
                        break;
                    case Light::SPOT:
                        // TODO: Implement spot light bounds
                        break;
                }
            }
            if( empty )
            {
                // Empty bounding sphere, set the world translation with zero radius
                worldMatrix.getTranslation(&_bounds.center);
                _bounds.radius = 0;
            }

            // Transform the sphere (if not empty) into world space.
            if( !empty )
            {
                bool applyWorldTransform = true;
                if( model && model->getSkin() )
                {
                    // Special case: If the root joint of our mesh skin is parented by any nodes,
                    // multiply the world matrix of the root joint's parent by this node's
                    // world matrix. This computes a final world matrix used for transforming this
                    // node's bounding volume. This allows us to store a much smaller bounding
                    // volume approximation than would otherwise be possible for skinned meshes,
                    // since joint parent nodes that are not in the matrix palette do not need to
                    // be considered as directly transforming vertices on the GPU (they can instead
                    // be applied directly to the bounding volume transformation below).
                    GP_ASSERT(model->getSkin()->getRootJoint());
                    auto jointParent = model->getSkin()->getRootJoint()->getParent().lock();
                    if( jointParent )
                    {
                        // TODO: Should we protect against the case where joints are nested directly
                        // in the node hierachy of the model (this is normally not the case)?
                        Matrix boundsMatrix;
                        Matrix::multiply(getWorldMatrix(), jointParent->getWorldMatrix(), &boundsMatrix);
                        _bounds.transform(boundsMatrix);
                        applyWorldTransform = false;
                    }
                }
                if( applyWorldTransform )
                {
                    _bounds.transform(getWorldMatrix());
                }
            }

            // Merge this world-space bounding sphere with our childrens' bounding volumes.
            for( const auto& child : _children )
            {
                const BoundingSphere& childSphere = child->getBoundingSphere();
                if( !childSphere.isEmpty() )
                {
                    if( empty )
                    {
                        _bounds.set(childSphere);
                        empty = false;
                    }
                    else
                    {
                        _bounds.merge(childSphere);
                    }
                }
            }
        }

        return _bounds;
    }


    std::shared_ptr<AIAgent> Node::getAgent() const
    {
        // Lazily create a new Agent for this Node if we don't have one yet.
        // Basically, all Nodes by default can have an Agent, we just won't
        // waste the memory unless they request one.
        if( !_agent )
        {
            _agent = AIAgent::create();
            _agent->_node = const_cast<Node*>(this);
            Game::getInstance()->getAIController()->addAgent(_agent);
        }

        return _agent;
    }


    void Node::setAgent(const std::shared_ptr<AIAgent>& agent)
    {
        if( agent == _agent )
            return;

        if( _agent )
        {
            Game::getInstance()->getAIController()->removeAgent(_agent);
            _agent->setNode(nullptr);
        }

        _agent = agent;

        if( _agent )
        {
            _agent->setNode(this);
            Game::getInstance()->getAIController()->addAgent(_agent);
        }
    }


    void* Node::getUserObject() const
    {
        return _userObject;
    }


    void Node::setUserObject(void* obj)
    {
        _userObject = obj;
    }
}
