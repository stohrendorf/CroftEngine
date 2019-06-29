#pragma once

#include "ShaderProgram.h"
#include "gsl-lite.hpp"

#include <boost/optional.hpp>
#include <glm/glm.hpp>

namespace render
{
namespace scene
{
class Node;

class BufferParameter
{
public:
    explicit BufferParameter(std::string name)
        : m_name{std::move(name)}
    {
    }

    ~BufferParameter() = default;

    BufferParameter(const BufferParameter&&) = delete;

    BufferParameter& operator=(const BufferParameter&) = delete;

    BufferParameter& operator=(BufferParameter&&) = delete;

    const std::string& getName() const
    {
        return m_name;
    }

    void set(const std::shared_ptr<gl::ShaderStorageBuffer>& value)
    {
        m_bufferBinder = [value](const Node& /*node*/, gl::ProgramShaderStorageBlock& shaderStorageBlock) {
            shaderStorageBlock.bind(*value);
        };
    }

    template<class ClassType>
    void bind(ClassType* classInstance, const gl::ShaderStorageBuffer& (ClassType::*valueMethod)() const)
    {
        m_bufferBinder
            = [classInstance, valueMethod](const Node& /*node*/, gl::ProgramShaderStorageBlock& shaderStorageBlock) {
                  shaderStorageBlock.bind((classInstance->*valueMethod)());
              };
    }

    using BufferBinder = void(const Node& node, gl::ProgramShaderStorageBlock& shaderStorageBlock);

    void bind(std::function<BufferBinder>&& setter)
    {
        m_bufferBinder = std::move(setter);
    }

    bool bind(const Node& node, const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram);

private:
    gl::ProgramShaderStorageBlock*
        findShaderStorageBlock(const gsl::not_null<std::shared_ptr<ShaderProgram>>& shaderProgram) const
    {
        if(const auto block = shaderProgram->findShaderStorageBlock(m_name))
            return block;

        // This parameter was not found in the specified effect, so do nothing.
        BOOST_LOG_TRIVIAL(warning) << "Shader storage block '" << m_name << "' not found in program '"
                                   << shaderProgram->getId() << "'";

        return nullptr;
    }

    const std::string m_name;

    boost::optional<std::function<BufferBinder>> m_bufferBinder;
};
} // namespace scene
} // namespace render
