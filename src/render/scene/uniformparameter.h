#pragma once

#include "materialparameter.h"
#include "render/gl/program.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <gsl-lite.hpp>
#include <optional>

namespace render::scene
{
class Node;
class Camera;

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
    m_valueSetter = [value](const Node& /*node*/, gl::Uniform& uniform) { uniform.set(value); };
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod](const Node& /*node*/, gl::Uniform& uniform) {
      uniform.set((classInstance->*valueMethod)());
    };
  }

  using UniformValueSetter = void(const Node& node, gl::Uniform& uniform);

  void bind(std::function<UniformValueSetter>&& setter)
  {
    m_valueSetter = std::move(setter);
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance,
            ValueType (ClassType::*valueMethod)() const,
            std::size_t (ClassType::*countMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod, countMethod](const Node& /*node*/, const gl::Uniform& uniform) {
      uniform.set((classInstance->*valueMethod)(), (classInstance->*countMethod)());
    };
  }

  bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) override;

private:
  [[nodiscard]] gl::Uniform* findUniform(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
  {
    if(const auto uniform = shaderProgram->findUniform(getName()))
      return uniform;

    BOOST_LOG_TRIVIAL(warning) << "Uniform '" << getName() << "' not found in program '" << shaderProgram->getId()
                               << "'";

    return nullptr;
  }

  std::optional<std::function<UniformValueSetter>> m_valueSetter;
};

class UniformBlockParameter : public MaterialParameter
{
public:
  explicit UniformBlockParameter(std::string name)
      : MaterialParameter{std::move(name)}
  {
  }

  UniformBlockParameter(const UniformBlockParameter&&) = delete;

  UniformBlockParameter& operator=(const UniformBlockParameter&) = delete;

  UniformBlockParameter& operator=(UniformBlockParameter&&) = delete;

  template<typename T>
  void set(const std::shared_ptr<gl::UniformBuffer<T>>& value)
  {
    m_bufferBinder = [value](const Node& /*node*/, gl::UniformBlock& uniformBlock) { uniformBlock.bind(*value); };
  }

  template<class ClassType, typename T>
  void bind(ClassType* classInstance, const gl::UniformBuffer<T>& (ClassType::*valueMethod)() const)
  {
    m_bufferBinder = [classInstance, valueMethod](const Node& /*node*/, gl::UniformBlock& uniformBlock) {
      uniformBlock.bind((classInstance->*valueMethod)());
    };
  }

  using BufferBinder = void(const Node& node, gl::UniformBlock& uniformBlock);

  void bind(std::function<BufferBinder>&& setter)
  {
    m_bufferBinder = std::move(setter);
  }

  bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) override;

  void bindTransformBuffer();
  void bindCameraBuffer(const gsl::not_null<std::shared_ptr<Camera>>& camera);

private:
  [[nodiscard]] gl::UniformBlock*
    findUniformBlock(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
  {
    if(const auto block = shaderProgram->findUniformBlock(getName()))
      return block;

    BOOST_LOG_TRIVIAL(warning) << "Uniform block '" << getName() << "' not found in program '" << shaderProgram->getId()
                               << "'";

    return nullptr;
  }

  std::optional<std::function<BufferBinder>> m_bufferBinder;
};

} // namespace render::scene
