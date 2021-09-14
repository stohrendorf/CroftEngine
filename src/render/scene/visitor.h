#pragma once

namespace render::scene
{
class RenderContext;
class Node;

class Visitor
{
public:
  static constexpr bool FlushAfterEachRender = false;

  explicit Visitor(RenderContext& context, bool withScissors = true)
      : m_context{context}
      , m_withScissors{withScissors}
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

  [[nodiscard]] bool withScissors() const
  {
    return m_withScissors;
  }

private:
  RenderContext& m_context;
  const bool m_withScissors;
};
} // namespace render::scene
