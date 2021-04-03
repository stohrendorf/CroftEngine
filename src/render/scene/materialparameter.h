#pragma once

#include <gsl/gsl-lite.hpp>

namespace render::scene
{
class Mesh;
class Node;
class ShaderProgram;

class MaterialParameter
{
public:
  explicit MaterialParameter(std::string name)
      : m_name{std::move(name)}
  {
  }

  virtual ~MaterialParameter() = default;

  virtual bool
    bind(const Node& node, const Mesh& mesh, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram)
    = 0;

  [[nodiscard]] const std::string& getName() const
  {
    return m_name;
  }

private:
  const std::string m_name;
};
} // namespace render::scene
