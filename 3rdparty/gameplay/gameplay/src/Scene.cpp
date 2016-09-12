#include "Base.h"
#include "Scene.h"
#include "MeshSkin.h"
#include "Joint.h"


namespace gameplay
{
    // Global list of active scenes
    static std::vector<Scene*> __sceneList;


    Scene::Scene()
        : _id("")
        , _activeCamera(nullptr)
        , _firstNode(nullptr)
        , _lastNode(nullptr)
        , _nodeCount(0)
        , _nextItr(nullptr)
        , _nextReset(true)
    {
        __sceneList.push_back(this);
    }


    Scene::~Scene()
    {
        // Remove all nodes from the scene
        removeAllNodes();

        // Remove the scene from global list
        auto itr = std::find(__sceneList.begin(), __sceneList.end(), this);
        if( itr != __sceneList.end() )
            __sceneList.erase(itr);
    }


    Scene* Scene::create(const char* id)
    {
        Scene* scene = new Scene();
        scene->setId(id);
        return scene;
    }


    Scene* Scene::getScene(const char* id)
    {
        if( id == nullptr )
            return !__sceneList.empty() ? __sceneList[0] : nullptr;

        for( size_t i = 0, count = __sceneList.size(); i < count; ++i )
        {
            if( __sceneList[i]->_id == id )
                return __sceneList[i];
        }

        return nullptr;
    }


    const char* Scene::getId() const
    {
        return _id.c_str();
    }


    void Scene::setId(const char* id)
    {
        _id = id ? id : "";
    }


    std::shared_ptr<Node> Scene::findNode(const char* id, bool recursive, bool exactMatch) const
    {
        GP_ASSERT(id);

        // Search immediate children first.
        for( auto child = getFirstNode(); child != nullptr; child = child->getNextSibling() )
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
            for( auto child = getFirstNode(); child != nullptr; child = child->getNextSibling() )
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


    unsigned int Scene::findNodes(const char* id, std::vector<std::shared_ptr<Node>>& nodes, bool recursive, bool exactMatch) const
    {
        GP_ASSERT(id);

        unsigned int count = 0;

        // Search immediate children first.
        for( auto child = getFirstNode(); child != nullptr; child = child->getNextSibling() )
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
            for( auto child = getFirstNode(); child != nullptr; child = child->getNextSibling() )
            {
                count += child->findNodes(id, nodes, true, exactMatch);
            }
        }

        return count;
    }


    void Scene::visitNode(const std::shared_ptr<Node>& node, const char* visitMethod) const
    {
        // If this node has a model with a mesh skin, visit the joint hierarchy within it
        // since we don't add joint hierarcies directly to the scene. If joints are never
        // visited, it's possible that nodes embedded within the joint hierarchy that contain
        // models will never get visited (and therefore never get drawn).
        auto model = std::dynamic_pointer_cast<Model>(node->getDrawable());
        if( model && model->_skin && model->_skin->_rootNode )
        {
            visitNode(model->_skin->_rootNode, visitMethod);
        }

        // Recurse for all children.
        for( auto child = node->getFirstChild(); child != nullptr; child = child->getNextSibling() )
        {
            visitNode(child, visitMethod);
        }
    }


    std::shared_ptr<Node> Scene::addNode(const std::string& id)
    {
        auto node = std::make_shared<Node>(id);
        GP_ASSERT(node);
        addNode(node);

        return node;
    }


    void Scene::addNode(const std::shared_ptr<Node>& node)
    {
        GP_ASSERT(node);

        if( node->_scene == this )
        {
            // The node is already a member of this scene.
            return;
        }

        // If the node is part of another scene, remove it.
        if( node->_scene && node->_scene != this )
        {
            node->_scene->removeNode(node);
        }

        // If the node is part of another node hierarchy, remove it.
        if( !node->getParent().expired() )
        {
            node->getParent().lock()->removeChild(node);
        }

        // Link the new node into the end of our list.
        if( _lastNode )
        {
            _lastNode->_nextSibling = node;
            node->_prevSibling = _lastNode;
            _lastNode = node;
        }
        else
        {
            _firstNode = _lastNode = node;
        }

        node->_scene = this;

        ++_nodeCount;

        // If we don't have an active camera set, then check for one and set it.
        if( _activeCamera == nullptr )
        {
            auto camera = node->getCamera();
            if( camera )
            {
                setActiveCamera(camera);
            }
        }
    }


    void Scene::removeNode(const std::shared_ptr<Node>& node)
    {
        GP_ASSERT(node);

        if( node->_scene != this )
            return;

        if( node == _firstNode )
        {
            _firstNode = node->_nextSibling;
        }
        if( node == _lastNode )
        {
            _lastNode = node->_prevSibling;
        }

        node->remove();
        node->_scene = nullptr;

        --_nodeCount;
    }


    void Scene::removeAllNodes()
    {
        while( _lastNode )
        {
            removeNode(_lastNode);
        }
    }


    unsigned int Scene::getNodeCount() const
    {
        return _nodeCount;
    }


    const std::shared_ptr<Node>& Scene::getFirstNode() const
    {
        return _firstNode;
    }


    const std::shared_ptr<Camera>& Scene::getActiveCamera() const
    {
        return _activeCamera;
    }


    void Scene::setActiveCamera(const std::shared_ptr<Camera>& camera)
    {
        _activeCamera = camera;
    }


    const Vector3& Scene::getAmbientColor() const
    {
        return _ambientColor;
    }


    void Scene::setAmbientColor(float red, float green, float blue)
    {
        _ambientColor.set(red, green, blue);
    }


    void Scene::update(float elapsedTime)
    {
        for( auto node = _firstNode; node != nullptr; node = node->_nextSibling )
        {
            if( node->isEnabled() )
                node->update(elapsedTime);
        }
    }


    void Scene::reset()
    {
        _nextItr = nullptr;
        _nextReset = true;
    }


    std::shared_ptr<Node> Scene::getNext()
    {
        if( _nextReset )
        {
            _nextItr = findNextVisibleSibling(getFirstNode());
            _nextReset = false;
        }
        else if( _nextItr )
        {
            auto node = findNextVisibleSibling(_nextItr->getFirstChild());
            if( node == nullptr )
            {
                node = findNextVisibleSibling(_nextItr->getNextSibling());
                if( node == nullptr )
                {
                    // Find first parent with a sibling
                    node = _nextItr->getParent().lock();
                    while( node && (!findNextVisibleSibling(node->getNextSibling())) )
                    {
                        node = node->getParent().lock();
                    }
                    if( node )
                    {
                        node = findNextVisibleSibling(node->getNextSibling());
                    }
                }
            }
            _nextItr = node;
        }
        return _nextItr;
    }


    std::shared_ptr<Node> Scene::findNextVisibleSibling(const std::shared_ptr<Node>& node) const
    {
        auto n = node;
        while( n != nullptr && !isNodeVisible(n) )
        {
            n = n->getNextSibling();
        }
        return n;
    }


    bool Scene::isNodeVisible(const std::shared_ptr<Node>& node) const
    {
        if( !node->isEnabled() )
            return false;

        if( node->getDrawable() || node->getLight() || node->getCamera() )
        {
            return true;
        }
        else
        {
            return node->getBoundingSphere().intersects(_activeCamera->getFrustum());
        }
    }
}
