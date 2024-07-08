#pragma once

#include "materialparameter.h"

#include <cstddef>
#include <functional>
#include <gl/buffer.h>
#include <gl/program.h>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace render::scene
{
class Camera;
class Mesh;
class Node;
} // namespace render::scene

namespace render::material
{
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
    m_valueSetter = [value](const scene::Node* /*node*/, const scene::Mesh& /*mesh*/, gl::Uniform& uniform)
    {
      uniform.set(value);
    };
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance, ValueType (ClassType::*valueMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod](const scene::Node& /*node*/, gl::Uniform& uniform)
    {
      uniform.set((classInstance->*valueMethod)());
    };
  }

  using UniformValueSetter = void(const scene::Node* node, const scene::Mesh& mesh, gl::Uniform& uniform);

  void bind(std::function<UniformValueSetter>&& setter)
  {
    m_valueSetter = std::move(setter);
  }

  template<class ClassType, class ValueType>
  void bind(ClassType* classInstance,
            ValueType (ClassType::*valueMethod)() const,
            std::size_t (ClassType::*countMethod)() const)
  {
    m_valueSetter = [classInstance, valueMethod, countMethod](
                      const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, const gl::Uniform& uniform)
    {
      uniform.set((classInstance->*valueMethod)(), (classInstance->*countMethod)());
    };
  }

  bool bind(const scene::Node* node, const scene::Mesh& mesh, ShaderProgram& shaderProgram) override;

private:
  [[nodiscard]] gl::Uniform* findUniform(ShaderProgram& shaderProgram) const;

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
    m_bufferBinder = [value](const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, gl::UniformBlock& uniformBlock)
    {
      uniformBlock.bind(*value);
    };
  }

  template<class ClassType, typename T>
  void bind(ClassType* classInstance, const gl::UniformBuffer<T>& (ClassType::*valueMethod)() const)
  {
    m_bufferBinder = [classInstance, valueMethod](
                       const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, gl::UniformBlock& uniformBlock)
    {
      uniformBlock.bind((classInstance->*valueMethod)());
    };
  }

  using BufferBinder = void(const scene::Node* node, const scene::Mesh& mesh, gl::UniformBlock& uniformBlock);

  void bind(std::function<BufferBinder>&& setter)
  {
    m_bufferBinder = std::move(setter);
  }

  bool bind(const scene::Node* node, const scene::Mesh& mesh, ShaderProgram& shaderProgram) override;

  void bindTransformBuffer();
  void bindCameraBuffer(const gslu::nn_shared<scene::Camera>& camera);

private:
  [[nodiscard]] gl::UniformBlock* findUniformBlock(ShaderProgram& shaderProgram) const;

  std::function<BufferBinder> m_bufferBinder;
};
} // namespace render::material
