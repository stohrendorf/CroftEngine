#pragma once

#include <gsl/gsl-lite.hpp>
#include <gslu.h>

namespace render::scene
{
class Mesh;
class Node;
} // namespace render::scene

namespace render::material
{
class ShaderProgram;

class MaterialParameter
{
public:
  explicit MaterialParameter(std::string name)
      : m_name{std::move(name)}
  {
  }

  virtual ~MaterialParameter() = default;

  virtual bool bind(const scene::Node* node, const scene::Mesh& mesh, ShaderProgram& shaderProgram) = 0;

  [[nodiscard]] const std::string& getName() const
  {
    return m_name;
  }

private:
  const std::string m_name;
};
} // namespace render::material
