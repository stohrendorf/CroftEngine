#include "Base.h"
#include "Scene.h"


namespace gameplay
{
    Scene::Scene() = default;


    Scene::~Scene() = default;


    void Scene::addNode(const std::shared_ptr<Node>& node)
    {
        BOOST_ASSERT(node);

        if( node->_scene == this )
        {
            // The node is already a member of this scene.
            return;
        }

        _nodes.push_back(node);
        node->_scene = this;
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
