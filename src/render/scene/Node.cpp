#include "Node.h"

#include "Scene.h"
#include "camera.h"

namespace render
{
namespace scene
{
Node::~Node()
{
    if(auto p = m_parent.lock())
    {
        const auto it = std::find_if(
            p->m_children.begin(), p->m_children.end(), [this](const gsl::not_null<std::shared_ptr<Node>>& node) {
                return node.get().get() == this;
            });
        if(it != p->m_children.end())
            p->m_children.erase(it);
    }

    m_parent.reset();

    transformChanged();
}

Scene* Node::getScene() const
{
    if(m_scene)
        return m_scene;

    // Search our parent for the scene
    if(const auto p = getParent().lock())
    {
        const auto scene = p->getScene();
        if(scene)
            return scene;
    }
    return nullptr;
}

const glm::mat4& Node::getModelMatrix() const
{
    if(m_dirty)
    {
        // Clear our dirty flag immediately to prevent this block from being entered if our
        // parent calls our getModelMatrix() method as a result of the following calculations.
        m_dirty = false;

        // If we have a parent, multiply our parent world transform by our local
        // transform to obtain our final resolved world transform.
        if(const auto p = getParent().lock())
        {
            m_modelMatrix = p->getModelMatrix() * getLocalMatrix();
        }
        else
        {
            m_modelMatrix = getLocalMatrix();
        }
    }
    return m_modelMatrix;
}

const glm::mat4& Node::getViewMatrix() const
{
    Scene* scene = getScene();
    const auto camera = scene ? scene->getActiveCamera() : nullptr;
    if(camera)
    {
        return camera->getViewMatrix();
    }
    else
    {
        static const glm::mat4 identity{1.0f};
        return identity;
    }
}

const glm::mat4& Node::getInverseViewMatrix() const
{
    Scene* scene = getScene();
    const auto camera = scene ? scene->getActiveCamera() : nullptr;
    if(camera)
    {
        return camera->getInverseViewMatrix();
    }
    else
    {
        static const glm::mat4 identity{1.0f};
        return identity;
    }
}

const glm::mat4& Node::getProjectionMatrix() const
{
    const auto scene = getScene();
    const auto camera = scene != nullptr ? scene->getActiveCamera() : nullptr;
    if(camera != nullptr)
    {
        return camera->getProjectionMatrix();
    }
    else
    {
        static const glm::mat4 identity{1.0f};
        return identity;
    }
}

const glm::mat4& Node::getViewProjectionMatrix() const
{
    Scene* scene = getScene();
    const auto camera = scene ? scene->getActiveCamera() : nullptr;
    if(camera)
    {
        return camera->getViewProjectionMatrix();
    }
    else
    {
        static const glm::mat4 identity{1.0f};
        return identity;
    }
}

const glm::mat4& Node::getInverseViewProjectionMatrix() const
{
    Scene* scene = getScene();
    const auto camera = scene ? scene->getActiveCamera() : nullptr;
    if(camera)
    {
        return camera->getInverseViewProjectionMatrix();
    }
    static const glm::mat4 identity{1.0f};
    return identity;
}

void Node::transformChanged()
{
    m_dirty = true;

    // Notify our children that their transform has also changed (since transforms are inherited).
    for(const auto& child : m_children)
    {
        child->transformChanged();
    }
}
} // namespace scene
} // namespace render
