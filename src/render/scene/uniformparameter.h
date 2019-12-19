#pragma once

#include "gsl-lite.hpp"
#include "materialparameter.h"
#include "render/gl/program.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <optional>

namespace render::scene
{
class Node;

class UniformParameter final : public MaterialParameter
{
public:
  explicit UniformParameter(std::string name)
      : MaterialParameter{std::move(name)}
  {
  }

  UniformParameter(const UniformParameter&&) = delete;

  UniformParameter& operator=(const UniformParameter&) = delete;

  UniformParameter& operator=(UniformParameter&&) = delete;

  template<typename T>
  void set(const T& value)
  {
    m_valueSetter = [value](const Node& /*node*/, gl::ProgramUniform& uniform) { uniform.set(value); };
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod](const Node& /*node*/, gl::ProgramUniform& uniform) {
      uniform.set((classInstance->*valueMethod)());
    };
  }

  using UniformValueSetter = void(const Node& node, gl::ProgramUniform& uniform);

  void bind(std::function<UniformValueSetter>&& setter)
  {
    m_valueSetter = std::move(setter);
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance,
            ValueType (ClassType::*valueMethod)() const,
            std::size_t (ClassType::*countMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod, countMethod](const Node& /*node*/, const gl::ProgramUniform& uniform) {
      uniform.set((classInstance->*valueMethod)(), (classInstance->*countMethod)());
    };
  }

  void bindModelMatrix();

  void bindViewMatrix();

  void bindModelViewMatrix();

  void bindProjectionMatrix();

  void bindLightModelViewProjection();

  bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) override;

private:
  [[nodiscard]] gl::ProgramUniform*
    findUniform(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
  {
    if(const auto uniform = shaderProgram->findUniform(getName()))
      return uniform;

    // This parameter was not found in the specified effect, so do nothing.
    BOOST_LOG_TRIVIAL(warning) << "Uniform '" << getName() << "' not found in program '" << shaderProgram->getId()
                               << "'";

    return nullptr;
  }

  std::optional<std::function<UniformValueSetter>> m_valueSetter;
};
} // namespace render::scene
