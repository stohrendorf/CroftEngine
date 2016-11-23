#include "Base.h"
#include "Scene.h"


namespace gameplay
{
    Scene::Scene() = default;


    Scene::~Scene()
    {
        removeAllNodes();
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
}
