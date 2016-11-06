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
        BOOST_ASSERT(id);

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
            for(const auto& child : _nodes)
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


    size_t Scene::findNodes(const char* id, Node::List& nodes, bool recursive, bool exactMatch) const
    {
        BOOST_ASSERT(id);

        size_t count = 0;

        // Search immediate children first.
        for(const auto& child : _nodes)
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
            for(const auto& child : _nodes)
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
        for(const auto& child : _nodes)
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
        _ambientColor = { red, green, blue };
    }


    void Scene::update(float elapsedTime)
    {
        for(const auto& child : _nodes)
        {
            if( child->isEnabled() )
                child->update(elapsedTime);
        }
    }
}
