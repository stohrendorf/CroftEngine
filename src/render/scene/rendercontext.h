#pragma once

#include "render/gl/renderstate.h"

#include "gsl-lite.hpp"

#include <stack>

namespace render
{
namespace scene
{
class Node;

class RenderContext final
{
public:
    explicit RenderContext()
    {
        m_renderStates.push( render::gl::RenderState() );
    }

    Node* getCurrentNode() const noexcept
    {
        return m_currentNode;
    }

    void setCurrentNode(Node* n) noexcept
    {
        m_currentNode = n;
    }

    void pushState(const render::gl::RenderState& state)
    {
        auto tmp = m_renderStates.top();
        tmp.merge( state );
        m_renderStates.emplace( tmp );
    }

    void bindState()
    {
        Expects( !m_renderStates.empty() );
        m_renderStates.top().apply();
    }

    void popState()
    {
        m_renderStates.pop();
    }

private:
    Node* m_currentNode = nullptr;
    std::stack<render::gl::RenderState> m_renderStates{};
};
}
}
