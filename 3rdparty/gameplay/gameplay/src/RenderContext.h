#pragma once

#include "RenderState.h"

#include <gsl/gsl>

#include <stack>

namespace gameplay
{
class Node;


class RenderContext
{
public:
    explicit RenderContext()
    {
        m_renderStates.push( RenderState() );
    }

    Node* getCurrentNode() const noexcept
    {
        return m_currentNode;
    }

    void setCurrentNode(Node* n) noexcept
    {
        m_currentNode = n;
    }

    void pushState(const RenderState& state)
    {
        auto tmp = m_renderStates.top();
        tmp.merge( state );
        m_renderStates.emplace( tmp );
    }

    void bindState()
    {
        Expects( !m_renderStates.empty() );
        m_renderStates.top().bindState();
    }

    void popState()
    {
        m_renderStates.pop();
    }

private:
    Node* m_currentNode = nullptr;
    std::stack<RenderState> m_renderStates{};
};
}
