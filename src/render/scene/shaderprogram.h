#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/static_string.hpp>
#include <filesystem>
#include <gl/program.h>
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

  [[nodiscard]] const gl::Uniform* findUniform(const gsl::not_null<gsl::czstring>& name) const
  {
    return find(m_uniforms, name);
  }

  gl::Uniform* findUniform(const gsl::not_null<gsl::czstring>& name)
  {
    return find(m_uniforms, name);
  }

  [[nodiscard]] const gl::ShaderStorageBlock* findShaderStorageBlock(const gsl::not_null<gsl::czstring>& name) const
  {
    return find(m_shaderStorageBlocks, name);
  }

  gl::ShaderStorageBlock* findShaderStorageBlock(const gsl::not_null<gsl::czstring>& name)
  {
    return find(m_shaderStorageBlocks, name);
  }

  [[nodiscard]] const gl::UniformBlock* findUniformBlock(const gsl::not_null<gsl::czstring>& name) const
  {
    return find(m_uniformBlocks, name);
  }

  gl::UniformBlock* findUniformBlock(const gsl::not_null<gsl::czstring>& name)
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

  using IdentifierString = boost::static_string<64>;
  boost::container::flat_map<IdentifierString, gl::ProgramInput> m_vertexAttributes;
  boost::container::flat_map<IdentifierString, gl::Uniform> m_uniforms;
  boost::container::flat_map<IdentifierString, gl::ShaderStorageBlock> m_shaderStorageBlocks;
  boost::container::flat_map<IdentifierString, gl::UniformBlock> m_uniformBlocks;

  template<typename T>
  static const T* find(const boost::container::flat_map<IdentifierString, T>& map,
                       const gsl::not_null<gsl::czstring>& needle)
  {
    auto it = map.find(needle.get());
    return it == map.end() ? nullptr : &it->second;
  }

  template<typename T>
  static T* find(boost::container::flat_map<IdentifierString, T>& map, const gsl::not_null<gsl::czstring>& needle)
  {
    auto it = map.find(needle.get());
    return it == map.end() ? nullptr : &it->second;
  }
};
} // namespace render::scene
