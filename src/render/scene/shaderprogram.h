#pragma once

#include <boost/container/flat_map.hpp>
#include <boost/container/vector.hpp>
#include <boost/log/trivial.hpp>
#include <boost/throw_exception.hpp>
#include <filesystem>
#include <gl/program.h>
#include <gsl/gsl-lite.hpp>
#include <iosfwd>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace render::scene
{
class ShaderProgram
{
public:
  template<gl::api::ShaderType... Types>
  explicit ShaderProgram(const std::string_view& label, const gl::Shader<Types>&... shaders)
      : m_handle{label, shaders...}
      , m_id{label}
  {
    static_assert(sizeof...(Types) > 0);

    if(const auto log = m_handle.getInfoLog(); !log.empty())
      BOOST_LOG_TRIVIAL(debug) << "Shader program info log: " << log;

    if(!m_handle.getLinkStatus())
    {
      BOOST_LOG_TRIVIAL(error) << "Failed to link program";
      BOOST_THROW_EXCEPTION(std::runtime_error("Failed to link program"));
    }

    initInterface();
  }

  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram(ShaderProgram&&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;
  ShaderProgram& operator=(ShaderProgram&&) = delete;

  ~ShaderProgram();

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

  void bind() const;

  [[nodiscard]] const gl::Program& getHandle() const
  {
    return m_handle;
  }

private:
  static gsl::not_null<std::shared_ptr<ShaderProgram>> createFromSource(const std::string& programId,
                                                                        const std::string& vshId,
                                                                        const std::filesystem::path& vshPath,
                                                                        const std::string& vshSource,
                                                                        const std::string& fshId,
                                                                        const std::filesystem::path& fshPath,
                                                                        const std::string& fshSource,
                                                                        const std::vector<std::string>& defines = {});

  gl::Program m_handle;
  const std::string m_id;

  boost::container::flat_map<std::string, gl::ProgramInput> m_vertexAttributes;
  boost::container::flat_map<std::string, gl::Uniform> m_uniforms;
  boost::container::flat_map<std::string, gl::ShaderStorageBlock> m_shaderStorageBlocks;
  boost::container::flat_map<std::string, gl::UniformBlock> m_uniformBlocks;

  void initInterface();

  template<typename T>
  static const T* find(const boost::container::flat_map<std::string, T>& map, const std::string& needle)
  {
    auto it = map.find(needle);
    return it == map.end() ? nullptr : &it->second;
  }

  template<typename T>
  static T* find(boost::container::flat_map<std::string, T>& map, const std::string& needle)
  {
    auto it = map.find(needle);
    return it == map.end() ? nullptr : &it->second;
  }
};
} // namespace render::scene
