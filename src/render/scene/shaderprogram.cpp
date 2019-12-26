#include "shaderprogram.h"

#include "render/gl/shader.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/trivial.hpp>
#include <fstream>
#include <set>

namespace render::scene
{
namespace
{
std::string readAll(const std::filesystem::path& filePath)
{
  // Open file for reading.
  std::ifstream stream(filePath, std::ios::in | std::ios::binary);
  if(!stream.is_open())
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to load file: " << filePath;
    return {};
  }
  stream.seekg(0, std::ios::end);
  const auto size = static_cast<std::size_t>(stream.tellg());
  stream.seekg(0, std::ios::beg);

  // Read entire file contents.
  std::string buffer;
  buffer.resize(size);
  stream.read(&buffer[0], size);
  if(static_cast<std::size_t>(stream.gcount()) != size)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to read complete contents of file '" << filePath
                             << "' (amount read vs. file size: " << stream.gcount() << " < " << size << ").";
    return {};
  }

  return buffer;
}

std::string replaceDefines(const std::vector<std::string>& defines)
{
  std::string out;
  if(!defines.empty())
  {
    out += std::string("\n#define ") + boost::algorithm::join(defines, "\n#define ");
  }

  if(!out.empty())
  {
    out += "\n";
  }

  return out;
}

void replaceIncludes(const std::filesystem::path& filepath,
                     const std::string& source,
                     std::string& out,
                     std::set<std::filesystem::path>& included)
{
  included.emplace(filepath);
  out += "#line 1\n#pragma file \"" + boost::algorithm::replace_all_copy(filepath.string(), "\\", "/") + "\"\n";

  // Replace the #include "foo.bar" with the sourced file contents of "filepath/foo.bar"
  size_t headPos = 0;
  size_t line = 1;
  while(headPos < source.length())
  {
    const auto lastPos = headPos;
    if(headPos == 0)
    {
      // find the first "#include"
      headPos = source.find("#include");
    }
    else
    {
      // find the next "#include"
      headPos = source.find("#include", headPos + 1);
    }

    // If "#include" is found
    if(headPos != std::string::npos)
    {
      // append from our last position for the length (head - last position)
      {
        const auto part = source.substr(lastPos, headPos - lastPos);
        out.append(part);
        line += std::count(part.begin(), part.end(), '\n');
      }

      // find the start quote "
      const size_t startQuote = source.find('"', headPos) + 1;
      if(startQuote == std::string::npos)
      {
        // We have started an "#include" but missing the leading quote "
        BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "' missing leading \".";
        return;
      }
      // find the end quote "
      const size_t endQuote = source.find('"', startQuote);
      if(endQuote == std::string::npos)
      {
        // We have a start quote but missing the trailing quote "
        BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "' missing trailing \".";
        return;
      }

      // jump the head position past the end quote
      headPos = endQuote + 1;

      // File path to include and 'stitch' in the value in the quotes to the file path and source it.
      const size_t len = endQuote - startQuote;
      const auto includePath = filepath.parent_path() / std::filesystem::path{source.substr(startQuote, len)};
      if(included.count(includePath) > 0)
      {
        continue;
      }

      std::string includedSource = readAll(includePath);
      if(includedSource.empty())
      {
        BOOST_LOG_TRIVIAL(error) << "Compile failed for shader '" << filepath << "': failed to include'" << includePath
                                 << "'";
        return;
      }
      // Valid file so lets attempt to see if we need to append anything to it too (recurse...)
      replaceIncludes(includePath, includedSource, out, included);
      out += "#line " + std::to_string(line) + "\n#pragma file \""
             + boost::algorithm::replace_all_copy(filepath.string(), "\\", "/") + "\"\n";
    }
    else
    {
      // Append the remaining
      out.append(source, lastPos);
      break;
    }
  }
}

void writeShaderToErrorFile(const std::filesystem::path& filePath, const std::string& source)
{
  std::ofstream stream{filePath.string() + ".err", std::ios::out | std::ios::binary | std::ios::trunc};
  stream.write(source.c_str(), source.size());
}
} // namespace

ShaderProgram::ShaderProgram() = default;

ShaderProgram::~ShaderProgram() = default;

gsl::not_null<std::shared_ptr<ShaderProgram>> ShaderProgram::createFromFile(const std::string& id,
                                                                            const std::filesystem::path& vshPath,
                                                                            const std::filesystem::path& fshPath,
                                                                            const std::vector<std::string>& defines)
{
  const std::string vshSource = readAll(vshPath);
  if(vshSource.empty())
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to read vertex shader from file '" << vshPath << "'.";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create shader from sources"));
  }
  const std::string fshSource = readAll(fshPath);
  if(fshSource.empty())
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to read fragment shader from file '" << fshPath << "'.";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create shader from sources"));
  }

  std::shared_ptr<ShaderProgram> shaderProgram = createFromSource(vshPath, vshSource, fshPath, fshSource, defines);

  if(shaderProgram == nullptr)
  {
    BOOST_LOG_TRIVIAL(error) << "Failed to create effect from shaders '" << vshPath << "', '" << fshPath << "'.";
    BOOST_THROW_EXCEPTION(std::runtime_error("Failed to create shader from sources"));
  }

  shaderProgram->m_id = id;
  return shaderProgram;
}

std::shared_ptr<ShaderProgram> ShaderProgram::createFromSource(const std::filesystem::path& vshPath,
                                                               const std::string& vshSource,
                                                               const std::filesystem::path& fshPath,
                                                               const std::string& fshSource,
                                                               const std::vector<std::string>& defines)
{
  // Replace all comma separated definitions with #define prefix and \n suffix
  std::string definesStr = replaceDefines(defines);
  definesStr += "\n";

  static constexpr size_t SHADER_SOURCE_LENGTH = 3;
  gsl::czstring shaderSource[SHADER_SOURCE_LENGTH];
  shaderSource[0] = "#version 430\n";
  shaderSource[1] = definesStr.c_str();

  std::string vshSourceStr;
  if(!vshPath.empty())
  {
    // Replace the #include "foo.bar" with the sources that come from file paths
    std::set<std::filesystem::path> included;
    replaceIncludes(vshPath, vshSource, vshSourceStr, included);
    if(!vshSource.empty())
      vshSourceStr += "\n";
  }
  shaderSource[2] = !vshPath.empty() ? vshSourceStr.c_str() : vshSource.c_str();

  gl::VertexShader vertexShader{vshPath.string() + ";" + boost::algorithm::join(defines, ";")};
  vertexShader.setSource(shaderSource, SHADER_SOURCE_LENGTH);
  vertexShader.compile();
  if(!vertexShader.getCompileStatus())
  {
    // Write out the expanded shader file.
    if(!vshPath.empty())
      writeShaderToErrorFile(vshPath, shaderSource[2]);

    BOOST_LOG_TRIVIAL(error) << "Compile failed for vertex shader '" << (vshPath.empty() ? "<none>" : vshPath)
                             << "' with error '" << vertexShader.getInfoLog() << "'.";

    return nullptr;
  }

  // Compile the fragment shader.
  std::string fshSourceStr;
  if(!fshPath.empty())
  {
    // Replace the #include "foo.bar" with the sources that come from file paths
    std::set<std::filesystem::path> included;
    replaceIncludes(std::filesystem::path{fshPath}, fshSource, fshSourceStr, included);
    if(!fshSource.empty())
      fshSourceStr += "\n";
  }
  shaderSource[2] = !fshPath.empty() ? fshSourceStr.c_str() : fshSource.c_str();

  gl::FragmentShader fragmentShader{fshPath.string() + ";" + boost::algorithm::join(defines, ";")};
  fragmentShader.setSource(shaderSource, SHADER_SOURCE_LENGTH);
  fragmentShader.compile();
  if(!fragmentShader.getCompileStatus())
  {
    // Write out the expanded shader file.
    if(!fshPath.empty())
      writeShaderToErrorFile(fshPath, shaderSource[2]);

    BOOST_LOG_TRIVIAL(error) << "Compile failed for fragment shader '" << (fshPath.empty() ? "<none>" : fshPath)
                             << "' with error '" << fragmentShader.getInfoLog() << "'.";

    return nullptr;
  }

  auto shaderProgram = std::make_shared<ShaderProgram>();
  shaderProgram->m_handle.attach(vertexShader);
  shaderProgram->m_handle.attach(fragmentShader);
  shaderProgram->m_handle.link(vshPath.string() + ";" + fshPath.string() + ";" + boost::algorithm::join(defines, ";"));

  if(!shaderProgram->m_handle.getLinkStatus())
  {
    BOOST_LOG_TRIVIAL(error) << "Linking program failed (" << (vshPath.empty() ? "<none>" : vshPath) << ","
                             << (fshPath.empty() ? "<none>" : fshPath) << "): " << shaderProgram->m_handle.getInfoLog();

    return nullptr;
  }

  BOOST_LOG_TRIVIAL(debug) << "Program vertex=" << vshPath << " fragment=" << fshPath
                           << " defines=" << boost::algorithm::join(defines, ";");

  int32_t samplerIndex = 0;
  for(auto&& input : shaderProgram->m_handle.getInputs())
  {
    if(input.getLocation() < 0)
      continue; // only accept directly accessible uniforms

    BOOST_LOG_TRIVIAL(debug) << "  input " << input.getName() << ", location=" << input.getLocation();

    shaderProgram->m_vertexAttributes.insert(make_pair(input.getName(), std::move(input)));
  }

  for(auto&& uniform : shaderProgram->m_handle.getUniforms(samplerIndex))
  {
    if(uniform.getLocation() < 0)
      continue; // only accept directly accessible uniforms

    BOOST_LOG_TRIVIAL(debug) << "  uniform " << uniform.getName() << ", location=" << uniform.getLocation()
                             << ", index=" << uniform.getIndex();

    shaderProgram->m_uniforms.emplace(make_pair(uniform.getName(), std::move(uniform)));
  }

  for(auto&& ub : shaderProgram->m_handle.getUniformBlocks(samplerIndex))
  {
    BOOST_LOG_TRIVIAL(debug) << "  uniform block " << ub.getName() << ", index=" << ub.getIndex()
                             << ", binding=" << ub.getBinding();
    shaderProgram->m_uniformBlocks.emplace(make_pair(ub.getName(), ub));
  }

  for(auto&& ssb : shaderProgram->m_handle.getShaderStorageBlocks())
  {
    BOOST_LOG_TRIVIAL(debug) << "  shader storage block " << ssb.getName() << ", index=" << ssb.getIndex()
                             << ", binding=" << ssb.getBinding();
    shaderProgram->m_shaderStorageBlocks.insert(make_pair(ssb.getName(), std::move(ssb)));
  }

  return shaderProgram;
}

} // namespace render::scene
