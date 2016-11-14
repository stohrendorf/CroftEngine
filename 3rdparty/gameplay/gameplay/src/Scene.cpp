#include "Base.h"
#include "Scene.h"


namespace gameplay
{
    Scene::Scene() = default;


    Scene::~Scene()
    {
        removeAllNodes();
    }


    std::shared_ptr<Node> Scene::findNode(const std::string& id, bool recursive, bool exactMatch) const
    {
        // Search immediate children first.
        for( const auto& child : _nodes )
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
            for( const auto& child : _nodes )
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


    size_t Scene::findNodes(const std::string& id, Node::List& nodes, bool recursive, bool exactMatch) const
    {
        size_t count = 0;

        // Search immediate children first.
        for( const auto& child : _nodes )
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
            for( const auto& child : _nodes )
            {
                count += child->findNodes(id, nodes, true, exactMatch);
            }
        }

        return count;
    }


    void Scene::visitNode(const std::shared_ptr<Node>& node, const char* visitMethod) const
    {
        // Recurse for all children.
        for( const auto& child : _nodes )
        {
            visitNode(child, visitMethod);
        }
    }


    std::shared_ptr<Node> Scene::addNode(const std::string& id)
    {
        auto node = std::make_shared<Node>(id);
        BOOST_ASSERT(node);
        addNode(node);

        return node;
    }


    void Scene::addNode(const std::shared_ptr<Node>& node)
    {
        BOOST_ASSERT(node);

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

        _nodes.push_back(node);
        node->_scene = this;

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
        BOOST_ASSERT(node);

        if( node->_scene != this )
            return;

        node->remove();
        node->_scene = nullptr;
    }


    void Scene::removeAllNodes()
    {
        while( !_nodes.empty() )
        {
            removeNode(_nodes.back());
        }
    }


    size_t Scene::getNodeCount() const
    {
        return _nodes.size();
    }


    const std::shared_ptr<Camera>& Scene::getActiveCamera() const
    {
        return _activeCamera;
    }


    void Scene::setActiveCamera(const std::shared_ptr<Camera>& camera)
    {
        _activeCamera = camera;
    }


    const glm::vec3& Scene::getAmbientColor() const
    {
        return _ambientColor;
    }


    void Scene::setAmbientColor(float red, float green, float blue)
    {
        _ambientColor = {red, green, blue};
    }
}
