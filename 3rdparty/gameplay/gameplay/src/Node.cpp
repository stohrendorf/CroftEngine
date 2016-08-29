#include "Base.h"
#include "Node.h"
#include "Scene.h"
#include "Game.h"
#include "Drawable.h"
#include "Ref.h"
#include "Joint.h"

// Node dirty flags
#define NODE_DIRTY_WORLD 1
#define NODE_DIRTY_BOUNDS 2
#define NODE_DIRTY_HIERARCHY 4
#define NODE_DIRTY_ALL (NODE_DIRTY_WORLD | NODE_DIRTY_BOUNDS | NODE_DIRTY_HIERARCHY)


namespace gameplay
{
    Node::Node(const char* id)
        : _scene(nullptr)
        , _firstChild(nullptr)
        , _nextSibling(nullptr)
        , _prevSibling(nullptr)
        , _parent(nullptr)
        , _childCount(0)
        , _enabled(true)
        , _tags(nullptr)
        , _drawable(nullptr)
        , _camera(nullptr)
        , _light(nullptr)
        , _agent(nullptr)
        , _userObject(nullptr)
        , _dirtyBits(NODE_DIRTY_ALL)
    {
        if( id )
        {
            _id = id;
        }
    }


    Node::~Node()
    {
        removeAllChildren();
        if( _drawable )
            _drawable->setNode(nullptr);
        Ref* ref = dynamic_cast<Ref*>(_drawable);
        SAFE_RELEASE(ref);
        SAFE_RELEASE(_camera);
        SAFE_RELEASE(_light);
        SAFE_RELEASE(_userObject);
        SAFE_DELETE(_tags);
        setAgent(nullptr);
    }


    Node* Node::create(const char* id)
    {
        return new Node(id);
    }


    const char* Node::getId() const
    {
        return _id.c_str();
    }


    void Node::setId(const char* id)
    {
        if( id )
        {
            _id = id;
        }
    }


    Node::Type Node::getType() const
    {
        return Node::NODE;
    }


    void Node::addChild(Node* child)
    {
        GP_ASSERT(child);

        if( child->_parent == this )
        {
            // This node is already present in our hierarchy
            return;
        }
        child->addRef();

        // If the item belongs to another hierarchy, remove it first.
        if( child->_parent )
        {
            child->_parent->removeChild(child);
        }
        else if( child->_scene )
        {
            child->_scene->removeNode(child);
        }
        // Add child to the end of the list.
        // NOTE: This is different than the original behavior which inserted nodes
        // into the beginning of the list. Although slightly slower to add to the
        // end of the list, it makes scene traversal and drawing order more
        // predictable, so I've changed it.
        if( _firstChild )
        {
            Node* n = _firstChild;
            while( n->_nextSibling )
                n = n->_nextSibling;
            n->_nextSibling = child;
            child->_prevSibling = n;
        }
        else
        {
            _firstChild = child;
        }
        child->_parent = this;
        ++_childCount;
        setBoundsDirty();

        if( _dirtyBits & NODE_DIRTY_HIERARCHY )
        {
            hierarchyChanged();
        }
    }


    void Node::removeChild(Node* child)
    {
        if( child == nullptr || child->_parent != this )
        {
            // The child is not in our hierarchy.
            return;
        }
        // Call remove on the child.
        child->remove();
        SAFE_RELEASE(child);
    }


    void Node::removeAllChildren()
    {
        _dirtyBits &= ~NODE_DIRTY_HIERARCHY;
        while( _firstChild )
        {
            removeChild(_firstChild);
        }
        _dirtyBits |= NODE_DIRTY_HIERARCHY;
        hierarchyChanged();
    }


    void Node::remove()
    {
        // Re-link our neighbours.
        if( _prevSibling )
        {
            _prevSibling->_nextSibling = _nextSibling;
        }
        if( _nextSibling )
        {
            _nextSibling->_prevSibling = _prevSibling;
        }
        // Update our parent.
        Node* parent = _parent;
        if( parent )
        {
            if( this == parent->_firstChild )
            {
                parent->_firstChild = _nextSibling;
            }
            --parent->_childCount;
        }
        _nextSibling = nullptr;
        _prevSibling = nullptr;
        _parent = nullptr;

        if( parent && parent->_dirtyBits & NODE_DIRTY_HIERARCHY )
        {
            parent->hierarchyChanged();
        }
    }


    Node* Node::getFirstChild() const
    {
        return _firstChild;
    }


    Node* Node::getNextSibling() const
    {
        return _nextSibling;
    }


    Node* Node::getPreviousSibling() const
    {
        return _prevSibling;
    }


    Node* Node::getParent() const
    {
        return _parent;
    }


    unsigned int Node::getChildCount() const
    {
        return _childCount;
    }


    Node* Node::getRootNode() const
    {
        Node* n = const_cast<Node*>(this);
        while( n->getParent() )
        {
            n = n->getParent();
        }
        return n;
    }


    Node* Node::findNode(const char* id, bool recursive, bool exactMatch) const
    {
        GP_ASSERT(id);

        // If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
        Node* rootNode = nullptr;
        Model* model = dynamic_cast<Model*>(_drawable);
        if( model )
        {
            if( model->getSkin() != nullptr && (rootNode = model->getSkin()->_rootNode) != nullptr )
            {
                if( (exactMatch && rootNode->_id == id) || (!exactMatch && rootNode->_id.find(id) == 0) )
                    return rootNode;

                Node* match = rootNode->findNode(id, true, exactMatch);
                if( match )
                {
                    return match;
                }
            }
        }
        // Search immediate children first.
        for( Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling() )
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
            for( Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling() )
            {
                Node* match = child->findNode(id, true, exactMatch);
                if( match )
                {
                    return match;
                }
            }
        }
        return nullptr;
    }


    unsigned int Node::findNodes(const char* id, std::vector<Node*>& nodes, bool recursive, bool exactMatch) const
    {
        GP_ASSERT(id);

        // If the drawable is a model with a mesh skin, search the skin's hierarchy as well.
        unsigned int count = 0;
        Node* rootNode = nullptr;
        Model* model = dynamic_cast<Model*>(_drawable);
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
        for( Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling() )
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
            for( Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling() )
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
        if( _parent )
        {
            Scene* scene = _parent->getScene();
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

        Node* node = _parent;
        while( node )
        {
            if( !node->_enabled )
            {
                return false;
            }
            node = node->_parent;
        }
        return true;
    }


    void Node::update(float elapsedTime)
    {
        for( Node* node = _firstChild; node != nullptr; node = node->_nextSibling )
        {
            if( node->isEnabled() )
            {
                node->update(elapsedTime);
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
                Node* parent = getParent();
                if( parent )
                {
                    Matrix::multiply(parent->getWorldMatrix(), getMatrix(), &_world);
                }
                else
                {
                    _world = getMatrix();
                }

                // Our world matrix was just updated, so call getWorldMatrix() on all child nodes
                // to force their resolved world matrices to be updated.
                for( Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling() )
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
        Camera* camera = scene ? scene->getActiveCamera() : nullptr;
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
        Camera* camera = scene ? scene->getActiveCamera() : nullptr;
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
        Camera* camera = scene ? scene->getActiveCamera() : nullptr;
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
        Camera* camera = scene ? scene->getActiveCamera() : nullptr;
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
        Camera* camera = scene ? scene->getActiveCamera() : nullptr;
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
            Camera* camera = scene->getActiveCamera();
            if( camera )
            {
                Node* cameraNode = camera->getNode();
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
            Camera* camera = scene->getActiveCamera();
            if( camera )
            {
                Node* cameraNode = camera->getNode();
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
        for( Node* n = getFirstChild(); n != nullptr; n = n->getNextSibling() )
        {
            if( Transform::isTransformChangedSuspended() )
            {
                // If the DIRTY_NOTIFY bit is not set
                if( !n->isDirty(Transform::DIRTY_NOTIFY) )
                {
                    n->transformChanged();
                    suspendTransformChange(n);
                }
            }
            else
            {
                n->transformChanged();
            }
        }
        Transform::transformChanged();
    }


    void Node::setBoundsDirty()
    {
        // Mark ourself and our parent nodes as dirty
        _dirtyBits |= NODE_DIRTY_BOUNDS;

        // Mark our parent bounds as dirty as well
        if( _parent )
            _parent->setBoundsDirty();
    }


    Animation* Node::getAnimation(const char* id) const
    {
        Animation* animation = ((AnimationTarget*)this)->getAnimation(id);
        if( animation )
            return animation;

        // See if this node has a model, then drill down.
        Model* model = dynamic_cast<Model*>(_drawable);
        if( model )
        {
            // Check to see if there's any animations with the ID on the joints.
            MeshSkin* skin = model->getSkin();
            if( skin )
            {
                Node* rootNode = skin->_rootNode;
                if( rootNode )
                {
                    animation = rootNode->getAnimation(id);
                    if( animation )
                        return animation;
                }
            }

            // Check to see if any of the model's material parameter's has an animation
            // with the given ID.
            Material* material = model->getMaterial();
            if( material )
            {
                // How to access material parameters? hidden on the Material::RenderState.
                std::vector<MaterialParameter*>::iterator itr = material->_parameters.begin();
                for( ; itr != material->_parameters.end(); ++itr )
                {
                    GP_ASSERT(*itr);
                    animation = static_cast<MaterialParameter*>(*itr)->getAnimation(id);
                    if( animation )
                        return animation;
                }
            }
        }

        // Look through this node's children for an animation with the specified ID.
        for( Node* child = getFirstChild(); child != nullptr; child = child->getNextSibling() )
        {
            animation = child->getAnimation(id);
            if( animation )
                return animation;
        }

        return nullptr;
    }


    Camera* Node::getCamera() const
    {
        return _camera;
    }


    void Node::setCamera(Camera* camera)
    {
        if( _camera == camera )
            return;

        if( _camera )
        {
            _camera->setNode(nullptr);
            SAFE_RELEASE(_camera);
        }

        _camera = camera;

        if( _camera )
        {
            _camera->addRef();
            _camera->setNode(this);
        }
    }


    Light* Node::getLight() const
    {
        return _light;
    }


    void Node::setLight(Light* light)
    {
        if( _light == light )
            return;

        if( _light )
        {
            _light->setNode(nullptr);
            SAFE_RELEASE(_light);
        }

        _light = light;

        if( _light )
        {
            _light->addRef();
            _light->setNode(this);
        }

        setBoundsDirty();
    }


    Drawable* Node::getDrawable() const
    {
        return _drawable;
    }


    void Node::setDrawable(Drawable* drawable)
    {
        if( _drawable != drawable )
        {
            if( _drawable )
            {
                _drawable->setNode(nullptr);
                Ref* ref = dynamic_cast<Ref*>(_drawable);
                if( ref )
                    ref->release();
            }

            _drawable = drawable;

            if( _drawable )
            {
                Ref* ref = dynamic_cast<Ref*>(_drawable);
                if( ref )
                    ref->addRef();
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
            Model* model = dynamic_cast<Model*>(_drawable);
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
                    Node* jointParent = model->getSkin()->getRootJoint()->getParent();
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
            for( Node* n = getFirstChild(); n != nullptr; n = n->getNextSibling() )
            {
                const BoundingSphere& childSphere = n->getBoundingSphere();
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


    AIAgent* Node::getAgent() const
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


    void Node::setAgent(AIAgent* agent)
    {
        if( agent == _agent )
            return;

        if( _agent )
        {
            Game::getInstance()->getAIController()->removeAgent(_agent);
            _agent->setNode(nullptr);
            SAFE_RELEASE(_agent);
        }

        _agent = agent;

        if( _agent )
        {
            _agent->addRef();
            _agent->setNode(this);
            Game::getInstance()->getAIController()->addAgent(_agent);
        }
    }


    Ref* Node::getUserObject() const
    {
        return _userObject;
    }


    void Node::setUserObject(Ref* obj)
    {
        _userObject = obj;
    }
}
