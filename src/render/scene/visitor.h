#pragma once

namespace render::scene
{
class RenderContext;

class Node;

class Visitor
{
public:
  static constexpr bool FlushAfterEachRender = false;

  explicit Visitor(RenderContext& context)
      : m_context{context}
  {
  }

  Visitor(const Visitor&) = delete;
  Visitor(Visitor&&) = delete;
  Visitor& operator=(Visitor&&) = delete;
  Visitor& operator=(const Visitor&) = delete;

  virtual ~Visitor() = default;

  virtual void visit(Node& node);

  RenderContext& getContext()
  {
    return m_context;
  }

private:
  RenderContext& m_context;
};
} // namespace render::scene
