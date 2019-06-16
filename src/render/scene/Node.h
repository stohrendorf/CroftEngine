#pragma once

#include "MaterialParameter.h"
#include "Visitor.h"
#include "model.h"

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

    explicit Node(std::string id);

    virtual ~Node();

    const std::string& getId() const;

    const std::weak_ptr<Node>& getParent() const;

    virtual Scene* getScene() const;

    void setVisible(bool visible);

    bool isVisible() const;

    virtual const glm::mat4& getModelMatrix() const;

    glm::mat4 getModelViewMatrix() const;

    const glm::mat4& getViewMatrix() const;

    const glm::mat4& getInverseViewMatrix() const;

    const glm::mat4& getProjectionMatrix() const;

    const glm::mat4& getViewProjectionMatrix() const;

    const glm::mat4& getInverseViewProjectionMatrix() const;

    glm::vec3 getTranslationWorld() const;

    const std::shared_ptr<Renderable>& getDrawable() const;

    void setDrawable(const std::shared_ptr<Renderable>& drawable);

    const List& getChildren() const
    {
        return m_children;
    }

    void removeAllChildren()
    {
        while(!m_children.empty())
            setParent(m_children[0], nullptr);
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

    void addMaterialParameterSetter(const std::string& name,
                                    const std::function<MaterialParameter::UniformValueSetter>& setter)
    {
        m_materialParameterSetters[name] = setter;
    }

    void addMaterialParameterSetter(const std::string& name,
                                    std::function<MaterialParameter::UniformValueSetter>&& setter)
    {
        m_materialParameterSetters[name] = std::move(setter);
    }

    const std::function<MaterialParameter::UniformValueSetter>*
        findMaterialParameterSetter(const std::string& name) const
    {
        const auto it = m_materialParameterSetters.find(name);
        if(it != m_materialParameterSetters.end())
            return &it->second;

        if(auto p = getParent().lock())
            return p->findMaterialParameterSetter(name);

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

    boost::container::flat_map<std::string, std::function<MaterialParameter::UniformValueSetter>>
        m_materialParameterSetters;

    friend void setParent(gsl::not_null<std::shared_ptr<Node>> node, const std::shared_ptr<Node>& parent);
};

inline void setParent(gsl::not_null<std::shared_ptr<Node>> node, const std::shared_ptr<Node>& parent)
{
    if(auto p = node->getParent().lock())
    {
        const auto it = std::find(p->m_children.begin(), p->m_children.end(), node);
        BOOST_ASSERT(it != p->m_children.end());
        node->getParent().lock()->m_children.erase(it);
    }

    if(auto p = node->getParent().lock())
    {
        const auto it = std::find(p->m_children.begin(), p->m_children.end(), node);
        if(it != p->m_children.end())
            p->m_children.erase(it);
    }

    node->m_parent = parent;

    if(parent != nullptr)
        parent->m_children.emplace_back(node);

    node->transformChanged();
}

inline void swapChildren(const gsl::not_null<std::shared_ptr<Node>>& a, const gsl::not_null<std::shared_ptr<Node>>& b)
{
    auto aChildren = a->getChildren();
    for(auto& child : aChildren)
        setParent(child, nullptr);
    BOOST_ASSERT(a->getChildren().empty());

    auto bChildren = b->getChildren();
    for(auto& child : bChildren)
        setParent(child, nullptr);
    BOOST_ASSERT(b->getChildren().empty());

    for(auto& child : bChildren)
        setParent(child, a);

    for(auto& child : aChildren)
        setParent(child, b);
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
