#include "shadermanager.h"

#include "shaderprogram.h"

#include <boost/algorithm/string/join.hpp>

namespace render::scene
{
std::string ShaderManager::makeId(const std::filesystem::path& vshPath,
                                  const std::filesystem::path& fshPath,
                                  const std::vector<std::string>& defines)
{
  std::string id = vshPath.string();
  id += ';';
  id += fshPath.string();
  id += ';';
  id += boost::algorithm::join(defines, ";");
  return id;
}

gsl::not_null<std::shared_ptr<ShaderProgram>> ShaderManager::get(const std::filesystem::path& vshPath,
                                                                 const std::filesystem::path& fshPath,
                                                                 const std::vector<std::string>& defines)
{
  const auto id = makeId(m_root / vshPath, m_root / fshPath, defines);
  const auto it = m_programs.find(id);
  if(it != m_programs.end())
    return it->second;

  auto shader = ShaderProgram::createFromFile(id, m_root / vshPath, m_root / fshPath, defines);
  m_programs.emplace(id, shader);
  return shader;
}
} // namespace render::scene
