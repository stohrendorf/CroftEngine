#pragma once

#include "materialparameter.h"

#include <functional>
#include <gl/buffer.h>
#include <gl/program.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <memory>
#include <string>
#include <utility>

namespace render::scene
{
class Mesh;
class Node;
} // namespace render::scene

namespace render::material
{
class ShaderProgram;

class BufferParameter : public MaterialParameter
{
public:
  explicit BufferParameter(std::string name)
      : MaterialParameter{std::move(name)}
  {
  }

  BufferParameter(const BufferParameter&&) = delete;
  BufferParameter& operator=(const BufferParameter&) = delete;
  BufferParameter& operator=(BufferParameter&&) = delete;

  template<typename T>
  void set(const std::shared_ptr<gl::ShaderStorageBuffer<T>>& value)
  {
    m_bufferBinder
      = [value](const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    {
      shaderStorageBlock.bind(*value);
    };
  }

  template<class ClassType, typename T>
  void bind(ClassType* classInstance, const gl::ShaderStorageBuffer<T>& (ClassType::*valueMethod)() const)
  {
    m_bufferBinder
      = [classInstance, valueMethod](
          const scene::Node& /*node*/, const scene::Mesh& /*mesh*/, gl::ShaderStorageBlock& shaderStorageBlock)
    {
      shaderStorageBlock.bind((classInstance->*valueMethod)());
    };
  }

  using BufferBinder
    = void(const scene::Node* node, const scene::Mesh& mesh, gl::ShaderStorageBlock& shaderStorageBlock);

  void bind(std::function<BufferBinder>&& setter)
  {
    m_bufferBinder = std::move(setter);
  }

  bool bind(const scene::Node* node,
            const scene::Mesh& mesh,
            const gslu::nn_shared<ShaderProgram>& shaderProgram) override;
  void bindBoneTransformBuffer(std::function<bool()> smooth);

private:
  [[nodiscard]] gl::ShaderStorageBlock*
    findShaderStorageBlock(const gslu::nn_shared<ShaderProgram>& shaderProgram) const;

  std::function<BufferBinder> m_bufferBinder;
};
} // namespace render::material
