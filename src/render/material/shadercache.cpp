#include "shadercache.h"

#include "shaderprogram.h"

#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/log/trivial.hpp>
#include <filesystem>
#include <gl/shader.h>
#include <gsl/gsl-lite.hpp>
#include <gslu.h>
#include <string>
#include <vector>

namespace render::material
{
namespace
{
std::string makeId(const std::filesystem::path& vshPath,
                   const std::filesystem::path& fshPath,
                   const std::filesystem::path& geomPath,
                   std::vector<std::string> defines)
{
  std::string id = vshPath.string();
  id += ';';
  id += fshPath.string();
  id += ';';
  id += geomPath.string();
  id += ';';
  std::sort(defines.begin(), defines.end());
  id += boost::algorithm::join(defines, ";");
  return id;
}
std::string
  makeId(const std::filesystem::path& vshPath, const std::filesystem::path& fshPath, std::vector<std::string> defines)
{
  std::string id = vshPath.string();
  id += ';';
  id += fshPath.string();
  id += ';';
  std::sort(defines.begin(), defines.end());
  id += boost::algorithm::join(defines, ";");
  return id;
}
std::string makeId(const std::filesystem::path& shaderPath, std::vector<std::string> defines)
{
  std::string id = shaderPath.string();
  id += ';';
  std::sort(defines.begin(), defines.end());
  id += boost::algorithm::join(defines, ";");
  return id;
}
} // namespace

gslu::nn_shared<ShaderProgram> ShaderCache::get(const std::filesystem::path& vshPath,
                                                const std::filesystem::path& fshPath,
                                                const std::vector<std::string>& defines)
{
  const auto programId = makeId(vshPath, fshPath, defines);
  const auto it = m_programs.find(programId);
  if(it != m_programs.end())
    return it->second;

  BOOST_LOG_TRIVIAL(debug) << "Loading shader program " << programId;
  auto vert = gl::VertexShader::create(m_root / vshPath, defines, makeId(vshPath, defines));
  auto frag = gl::FragmentShader::create(m_root / fshPath, defines, makeId(fshPath, defines));
  auto shader = gsl::make_shared<ShaderProgram>(programId, vert, frag);
  m_programs.emplace(programId, shader);
  return shader;
}

gslu::nn_shared<ShaderProgram> ShaderCache::get(const std::filesystem::path& vshPath,
                                                const std::filesystem::path& fshPath,
                                                const std::filesystem::path& geomPath,
                                                const std::vector<std::string>& defines)
{
  const auto programId = makeId(vshPath, fshPath, geomPath, defines);
  const auto it = m_programs.find(programId);
  if(it != m_programs.end())
    return it->second;

  BOOST_LOG_TRIVIAL(debug) << "Loading shader program " << programId;
  auto vert = gl::VertexShader::create(m_root / vshPath, defines, makeId(vshPath, defines));
  auto frag = gl::FragmentShader::create(m_root / fshPath, defines, makeId(fshPath, defines));
  auto geom = gl::GeometryShader::create(m_root / geomPath, defines, makeId(geomPath, defines));
  auto shader = gsl::make_shared<ShaderProgram>(programId, vert, frag, geom);
  m_programs.emplace(programId, shader);
  return shader;
}
} // namespace render::material
