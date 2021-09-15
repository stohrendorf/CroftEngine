#pragma once

#include "materialparameter.h"

#include <functional>
#include <gl/buffer.h>
#include <gl/program.h>
#include <gsl/gsl-lite.hpp>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>

namespace render::scene
{
class Camera;
class Mesh;
class Node;
class ShaderProgram;

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
    m_valueSetter = [value](const Node& /*node*/, const Mesh& /*mesh*/, gl::Uniform& uniform) { uniform.set(value); };
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod](const Node& /*node*/, gl::Uniform& uniform)
    { uniform.set((classInstance->*valueMethod)()); };
  }

  using UniformValueSetter = void(const Node& node, const Mesh& mesh, gl::Uniform& uniform);

  void bind(std::function<UniformValueSetter>&& setter)
  {
    m_valueSetter = std::move(setter);
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance,
            ValueType (ClassType::*valueMethod)() const,
            std::size_t (ClassType::*countMethod)() const)
  {
    m_valueSetter =
      [classInstance, valueMethod, countMethod](const Node& /*node*/, const Mesh& /*mesh*/, const gl::Uniform& uniform)
    { uniform.set((classInstance->*valueMethod)(), (classInstance->*countMethod)()); };
  }

  bool bind(const Node& node,
            const Mesh& mesh,
            const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) override;

private:
  [[nodiscard]] gl::Uniform* findUniform(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const;

  std::function<UniformValueSetter> m_valueSetter;
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
    m_bufferBinder = [value](const Node& /*node*/, const Mesh& /*mesh*/, gl::UniformBlock& uniformBlock)
    { uniformBlock.bind(*value); };
  }

  template<class ClassType, typename T>
  void bind(ClassType* classInstance, const gl::UniformBuffer<T>& (ClassType::*valueMethod)() const)
  {
    m_bufferBinder
      = [classInstance, valueMethod](const Node& /*node*/, const Mesh& /*mesh*/, gl::UniformBlock& uniformBlock)
    { uniformBlock.bind((classInstance->*valueMethod)()); };
  }

  using BufferBinder = void(const Node& node, const Mesh& mesh, gl::UniformBlock& uniformBlock);

  void bind(std::function<BufferBinder>&& setter)
  {
    m_bufferBinder = std::move(setter);
  }

  bool bind(const Node& node,
            const Mesh& mesh,
            const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) override;

  void bindTransformBuffer();
  void bindCameraBuffer(const gsl::not_null<std::shared_ptr<Camera>>& camera);

private:
  [[nodiscard]] gl::UniformBlock*
    findUniformBlock(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const;

  std::function<BufferBinder> m_bufferBinder;
};
} // namespace render::scene
