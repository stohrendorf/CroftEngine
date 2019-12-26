#pragma once

#include "render/gl/program.h"

#include <boost/container/flat_map.hpp>
#include <filesystem>
#include <map>
#include <memory>
#include <vector>

namespace render::scene
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

  static gsl::not_null<std::shared_ptr<ShaderProgram>> createFromFile(const std::string& id,
                                                                      const std::filesystem::path& vshPath,
                                                                      const std::filesystem::path& fshPath,
                                                                      const std::vector<std::string>& defines);

  [[nodiscard]] const std::string& getId() const
  {
    return m_id;
  }

  [[nodiscard]] const gl::Uniform* findUniform(const std::string& name) const
  {
    return find(m_uniforms, name);
  }

  gl::Uniform* findUniform(const std::string& name)
  {
    return find(m_uniforms, name);
  }

  [[nodiscard]] const gl::ShaderStorageBlock* findShaderStorageBlock(const std::string& name) const
  {
    return find(m_shaderStorageBlocks, name);
  }

  gl::ShaderStorageBlock* findShaderStorageBlock(const std::string& name)
  {
    return find(m_shaderStorageBlocks, name);
  }

  [[nodiscard]] const gl::UniformBlock* findUniformBlock(const std::string& name) const
  {
    return find(m_uniformBlocks, name);
  }

  gl::UniformBlock* findUniformBlock(const std::string& name)
  {
    return find(m_uniformBlocks, name);
  }

  void bind() const
  {
    m_handle.bind();
  }

  [[nodiscard]] const gl::Program& getHandle() const
  {
    return m_handle;
  }

private:
  static std::shared_ptr<ShaderProgram> createFromSource(const std::filesystem::path& vshPath,
                                                         const std::string& vshSource,
                                                         const std::filesystem::path& fshPath,
                                                         const std::string& fshSource,
                                                         const std::vector<std::string>& defines = {});

  std::string m_id;
  gl::Program m_handle;
  std::map<std::string, gl::ProgramInput> m_vertexAttributes;
  std::map<std::string, gl::Uniform> m_uniforms;
  std::map<std::string, gl::ShaderStorageBlock> m_shaderStorageBlocks;
  std::map<std::string, gl::UniformBlock> m_uniformBlocks;

  template<typename T>
  static const T* find(const std::map<std::string, T>& map, const std::string& needle)
  {
    auto it = map.find(needle);
    return it == map.end() ? nullptr : &it->second;
  }

  template<typename T>
  static T* find(std::map<std::string, T>& map, const std::string& needle)
  {
    auto it = map.find(needle);
    return it == map.end() ? nullptr : &it->second;
  }
};
} // namespace render::scene
