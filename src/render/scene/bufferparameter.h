#pragma once

#include "materialparameter.h"
#include "shaderprogram.h"

#include <boost/log/trivial.hpp>
#include <gsl-lite.hpp>
#include <optional>

namespace render::scene
{
class Node;

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
    m_bufferBinder = [value](const Node& /*node*/, gl::ProgramShaderStorageBlock& shaderStorageBlock) {
      shaderStorageBlock.bind(*value);
    };
  }

  template<class ClassType, typename T>
  void bind(ClassType* classInstance, const gl::ShaderStorageBuffer<T>& (ClassType::*valueMethod)() const)
  {
    m_bufferBinder
      = [classInstance, valueMethod](const Node& /*node*/, gl::ProgramShaderStorageBlock& shaderStorageBlock) {
          shaderStorageBlock.bind((classInstance->*valueMethod)());
        };
  }

  using BufferBinder = void(const Node& node, gl::ShaderStorageBlock& shaderStorageBlock);

  void bind(std::function<BufferBinder>&& setter)
  {
    m_bufferBinder = std::move(setter);
  }

  bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) override;
  void bindBoneTransformBuffer();

private:
  [[nodiscard]] gl::ShaderStorageBlock*
    findShaderStorageBlock(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
  {
    if(const auto block = shaderProgram->findShaderStorageBlock(getName()))
      return block;

    BOOST_LOG_TRIVIAL(warning) << "Shader storage block '" << getName() << "' not found in program '"
                               << shaderProgram->getId() << "'";

    return nullptr;
  }

  std::optional<std::function<BufferBinder>> m_bufferBinder;
};
} // namespace render::scene
