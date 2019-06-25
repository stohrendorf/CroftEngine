#pragma once

#include "render/gl/program.h"

#include <boost/container/flat_map.hpp>
#include <map>
#include <memory>
#include <vector>

namespace render
{
namespace scene
{
class ShaderProgram
{
public:
    explicit ShaderProgram();

    ShaderProgram(const ShaderProgram&) = delete;

    ShaderProgram(ShaderProgram&&) = delete;

    ShaderProgram& operator=(const ShaderProgram&) = delete;

    ShaderProgram& operator=(ShaderProgram&&) = delete;

    ~ShaderProgram();

    static std::shared_ptr<ShaderProgram> createFromFile(const std::string& vshPath,
                                                         const std::string& fshPath,
                                                         const std::vector<std::string>& defines = {});

    const std::string& getId() const
    {
        return m_id;
    }

    const gl::ProgramUniform* findUniform(const std::string& name) const
    {
        auto it = m_uniforms.find(name);
        return it == m_uniforms.end() ? nullptr : &it->second;
    }

    gl::ProgramUniform* findUniform(const std::string& name)
    {
        auto it = m_uniforms.find(name);
        return it == m_uniforms.end() ? nullptr : &it->second;
    }

    void bind() const
    {
        m_handle.bind();
    }

    const gl::Program& getHandle() const
    {
        return m_handle;
    }

private:
    static std::shared_ptr<ShaderProgram> createFromSource(const std::string& vshPath,
                                                           const std::string& vshSource,
                                                           const std::string& fshPath,
                                                           const std::string& fshSource,
                                                           const std::vector<std::string>& defines = {});

    std::string m_id;

    gl::Program m_handle;

    boost::container::flat_map<std::string, gl::ProgramInput> m_vertexAttributes;

    std::map<std::string, gl::ProgramUniform> m_uniforms;
};
} // namespace scene
} // namespace render
