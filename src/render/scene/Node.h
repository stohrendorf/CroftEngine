#pragma once

#include "Visitor.h"
#include "bufferparameter.h"
#include "model.h"
#include "uniformparameter.h"

#include <boost/container/flat_map.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace render
{
namespace scene
{
class Renderable;

class Scene;

class Node
{
    friend class Scene;

public:
    Node(const Node&) = delete;

    Node(Node&&) = delete;

    Node& operator=(Node&&) = delete;

    Node& operator=(const Node&) = delete;

    using List = std::vector<gsl::not_null<std::shared_ptr<Node>>>;

    explicit Node(std::string id)
        : m_id(std::move(id))
    {
    }

    virtual ~Node();

    const std::string& getId() const
    {
        return m_id;
    }

    const std::weak_ptr<Node>& getParent() const
    {
        return m_parent;
    }

    virtual Scene* getScene() const;

    void setVisible(bool visible)
    {
        m_visible = visible;
    }

    bool isVisible() const
    {
        return m_visible;
    }

    virtual const glm::mat4& getModelMatrix() const;

    glm::mat4 getModelViewMatrix() const
    {
        return getViewMatrix() * getModelMatrix();
    }

    const glm::mat4& getViewMatrix() const;

    const glm::mat4& getInverseViewMatrix() const;

    const glm::mat4& getProjectionMatrix() const;

    const glm::mat4& getViewProjectionMatrix() const;

    const glm::mat4& getInverseViewProjectionMatrix() const;

    glm::vec3 getTranslationWorld() const
    {
        return glm::vec3(getModelMatrix()[3]);
    }

    const std::shared_ptr<Renderable>& getDrawable() const
    {
        return m_drawable;
    }

    void setDrawable(const std::shared_ptr<Renderable>& drawable)
    {
        m_drawable = drawable;
    }

    const List& getChildren() const
    {
        return m_children;
    }

    void removeAllChildren()
    {
        for(auto& child : m_children)
            child->m_parent.reset();
        m_children.clear();
    }

    const gsl::not_null<std::shared_ptr<Node>>& getChild(const size_t idx) const
    {
        BOOST_ASSERT(idx < m_children.size());
        return m_children[idx];
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

    void accept(Visitor& visitor)
    {
        for(const auto& node : m_children)
            visitor.visit(*node);
    }

    void addUniformSetter(const std::string& name, const std::function<UniformParameter::UniformValueSetter>& setter)
    {
        m_uniformSetters[name] = setter;
    }

    void addUniformSetter(const std::string& name, std::function<UniformParameter::UniformValueSetter>&& setter)
    {
        m_uniformSetters[name] = std::move(setter);
    }

    void addBufferBinder(const std::string& name, const std::function<BufferParameter::BufferBinder>& binder)
    {
        m_bufferBinders[name] = binder;
    }

    void addBufferBinder(const std::string& name, std::function<BufferParameter::BufferBinder>& binder)
    {
        m_bufferBinders[name] = std::move(binder);
    }

    const std::function<UniformParameter::UniformValueSetter>* findUniformSetter(const std::string& name) const
    {
        const auto it = m_uniformSetters.find(name);
        if(it != m_uniformSetters.end())
            return &it->second;

        if(auto p = getParent().lock())
            return p->findUniformSetter(name);

        return nullptr;
    }

    const std::function<BufferParameter::BufferBinder>* findShaderStorageBlockBinder(const std::string& name) const
    {
        const auto it = m_bufferBinders.find(name);
        if(it != m_bufferBinders.end())
            return &it->second;

        if(auto p = getParent().lock())
            return p->findShaderStorageBlockBinder(name);

        return nullptr;
    }

protected:
    void transformChanged();

private:
    Scene* m_scene = nullptr;

    std::string m_id;

    List m_children;

    std::weak_ptr<Node> m_parent{};

    bool m_visible = true;

    std::shared_ptr<Renderable> m_drawable = nullptr;

    glm::mat4 m_localMatrix{1.0f};

    mutable glm::mat4 m_modelMatrix{1.0f};

    mutable bool m_dirty = false;

    boost::container::flat_map<std::string, std::function<UniformParameter::UniformValueSetter>> m_uniformSetters;
    boost::container::flat_map<std::string, std::function<BufferParameter::BufferBinder>> m_bufferBinders;

    friend void setParent(const gsl::not_null<std::shared_ptr<Node>>& node, const std::shared_ptr<Node>& parent);
};

inline void setParent(const gsl::not_null<std::shared_ptr<Node>>& node, const std::shared_ptr<Node>& parent)
{
    // first remove from hierarchy
    if(auto p = node->m_parent.lock())
    {
        const auto it = std::find(p->m_children.begin(), p->m_children.end(), node);
        BOOST_ASSERT(it != p->m_children.end());
        node->m_parent.reset();
        p->m_children.erase(it);
    }

    // then add to hierarchy again
    node->m_parent = parent;

    if(parent != nullptr)
        parent->m_children.emplace_back(node);

    node->transformChanged();
}

inline void addChild(const gsl::not_null<std::shared_ptr<Node>>& node,
                     const gsl::not_null<std::shared_ptr<Node>>& child)
{
    if(child->getParent().lock() == node.get())
    {
        // This node is already present in our hierarchy
        return;
    }

    setParent(child, node);
}
} // namespace scene
} // namespace render
