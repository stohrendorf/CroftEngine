#pragma once

#include "gsl-lite.hpp"

namespace render
{
namespace scene
{
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

  virtual bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) = 0;

  const std::string& getName() const
  {
    return m_name;
  }

  private:
  const std::string m_name;
};
} // namespace scene
} // namespace render
