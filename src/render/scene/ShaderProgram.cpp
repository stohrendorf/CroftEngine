#include "names.h"
#include "ShaderProgram.h"
#include "renderer.h"

#include "render/gl/shader.h"

#include <fstream>
#include <set>

#include <boost/log/trivial.hpp>
#include <boost/algorithm/string.hpp>

namespace render
{
namespace scene
{
namespace
{
std::string readAll(const std::string& filePath)
{
    // Open file for reading.
    std::ifstream stream( filePath, std::ios::in | std::ios::binary );
    if( !stream.is_open() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to load file: " << filePath;
        return {};
    }
    stream.seekg( 0, std::ios::end );
    const auto size = static_cast<std::size_t>(stream.tellg());
    stream.seekg( 0, std::ios::beg );

    // Read entire file contents.
    std::string buffer;
    buffer.resize( size );
    stream.read( &buffer[0], size );
    if( static_cast<std::size_t>(stream.gcount()) != size )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to read complete contents of file '" << filePath
                                   << "' (amount read vs. file size: " << stream.gcount() << " < " << size
                                   << ").";
        return {};
    }

    return buffer;
}

std::string replaceDefines(const std::vector<std::string>& defines)
{
    std::string out;
    if( !defines.empty() )
    {
        out += std::string( "\n#define " ) + boost::algorithm::join( defines, "\n#define " );
    }

    if( !out.empty() )
    {
        out += "\n";
    }

    return out;
}

void replaceIncludes(const std::string& filepath,
                     const std::string& source,
                     std::string& out,
                     std::set<std::string>& included)
{
    included.emplace( filepath );
    out += "#line 1\n#pragma file \"" + filepath + "\"\n";

    // Replace the #include "xxxx.xxx" with the sourced file contents of "filepath/xxxx.xxx"
    size_t headPos = 0;
    size_t line = 1;
    while( headPos < source.length() )
    {
        const auto lastPos = headPos;
        if( headPos == 0 )
        {
            // find the first "#include"
            headPos = source.find( "#include" );
        }
        else
        {
            // find the next "#include"
            headPos = source.find( "#include", headPos + 1 );
        }

        // If "#include" is found
        if( headPos != std::string::npos )
        {
            // append from our last position for the length (head - last position)
            {
                const auto part = source.substr( lastPos, headPos - lastPos );
                out.append( part );
                line += std::count( part.begin(), part.end(), '\n' );
            }

            // find the start quote "
            const size_t startQuote = source.find( '"', headPos ) + 1;
            if( startQuote == std::string::npos )
            {
                // We have started an "#include" but missing the leading quote "
                BOOST_LOG_TRIVIAL( error ) << "Compile failed for shader '" << filepath
                                           << "' missing leading \".";
                return;
            }
            // find the end quote "
            const size_t endQuote = source.find( '"', startQuote );
            if( endQuote == std::string::npos )
            {
                // We have a start quote but missing the trailing quote "
                BOOST_LOG_TRIVIAL( error ) << "Compile failed for shader '" << filepath
                                           << "' missing trailing \".";
                return;
            }

            // jump the head position past the end quote
            headPos = endQuote + 1;

            // File path to include and 'stitch' in the value in the quotes to the file path and source it.
            std::string filepathStr = filepath;
            std::string directoryPath = filepathStr.substr( 0, filepathStr.rfind( '/' ) + 1 );
            const size_t len = endQuote - startQuote;
            std::string includeStr = source.substr( startQuote, len );
            directoryPath.append( includeStr );
            if( included.count( directoryPath ) > 0 )
            {
                continue;
            }

            std::string includedSource = readAll( directoryPath );
            if( includedSource.empty() )
            {
                BOOST_LOG_TRIVIAL( error ) << "Compile failed for shader '" << filepathStr
                                           << "' invalid filepath.";
                return;
            }
            // Valid file so lets attempt to see if we need to append anything to it too (recurse...)
            replaceIncludes( directoryPath, includedSource, out, included );
            out += "#line " + std::to_string( line ) + "\n#pragma file \"" + filepath + "\"\n";
        }
        else
        {
            // Append the remaining
            out.append( source, lastPos );
            break;
        }
    }
}

void writeShaderToErrorFile(const std::string& filePath, const std::string& source)
{
    std::ofstream stream{filePath + ".err", std::ios::out | std::ios::binary | std::ios::trunc};
    stream.write( source.c_str(), source.size() );
}
}

ShaderProgram::ShaderProgram() = default;

ShaderProgram::~ShaderProgram()
{
    m_uniforms.clear();
}

std::shared_ptr<ShaderProgram> ShaderProgram::createFromFile(const std::string& vshPath,
                                                             const std::string& fshPath,
                                                             const std::vector<std::string>& defines)
{
    // Search the effect cache for an identical effect that is already loaded.
    std::string uniqueId = vshPath;
    uniqueId += ';';
    uniqueId += fshPath;
    uniqueId += ';';
    uniqueId += boost::algorithm::join( defines, ";" );

    // Read source from file.
    std::string vshSource = readAll( vshPath );
    if( vshSource.empty() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to read vertex shader from file '" << vshPath << "'.";
        return nullptr;
    }
    std::string fshSource = readAll( fshPath );
    if( fshSource.empty() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to read fragment shader from file '" << fshPath << "'.";
        return nullptr;
    }

    std::shared_ptr<ShaderProgram> shaderProgram = createFromSource( vshPath, vshSource, fshPath, fshSource,
                                                                     defines );

    if( shaderProgram == nullptr )
    {
        BOOST_LOG_TRIVIAL( error ) << "Failed to create effect from shaders '" << vshPath << "', '" << fshPath
                                   << "'.";
    }
    else
    {
        // Store this effect in the cache.
        shaderProgram->m_id = uniqueId;
    }

    return shaderProgram;
}

std::shared_ptr<ShaderProgram> ShaderProgram::createFromSource(const std::string& vshPath, const std::string& vshSource,
                                                               const std::string& fshPath, const std::string& fshSource,
                                                               const std::vector<std::string>& defines)
{
    // Replace all comma separated definitions with #define prefix and \n suffix
    std::string definesStr = replaceDefines( defines );
    definesStr += "\n";

    const size_t SHADER_SOURCE_LENGTH = 3;
    const ::gl::GLchar* shaderSource[SHADER_SOURCE_LENGTH];
    shaderSource[0] = "#version 410\n";
    shaderSource[1] = definesStr.c_str();

    std::string vshSourceStr;
    if( !vshPath.empty() )
    {
        // Replace the #include "xxxxx.xxx" with the sources that come from file paths
        std::set<std::string> included;
        replaceIncludes( vshPath, vshSource, vshSourceStr, included );
        if( !vshSource.empty() )
            vshSourceStr += "\n";
    }
    shaderSource[2] = !vshPath.empty() ? vshSourceStr.c_str() : vshSource.c_str();

    gl::Shader vertexShader{::gl::GL_VERTEX_SHADER, vshPath + ";" + boost::algorithm::join( defines, ";" )};
    vertexShader.setSource( shaderSource, SHADER_SOURCE_LENGTH );
    vertexShader.compile();
    if( !vertexShader.getCompileStatus() )
    {
        // Write out the expanded shader file.
        if( !vshPath.empty() )
            writeShaderToErrorFile( vshPath, shaderSource[2] );

        BOOST_LOG_TRIVIAL( error ) << "Compile failed for vertex shader '" << (vshPath.empty() ? "<none>" : vshPath)
                                   << "' with error '" << vertexShader.getInfoLog() << "'.";

        return nullptr;
    }

    // Compile the fragment shader.
    std::string fshSourceStr;
    if( !fshPath.empty() )
    {
        // Replace the #include "xxxxx.xxx" with the sources that come from file paths
        std::set<std::string> included;
        replaceIncludes( fshPath, fshSource, fshSourceStr, included );
        if( !fshSource.empty() )
            fshSourceStr += "\n";
    }
    shaderSource[2] = !fshPath.empty() ? fshSourceStr.c_str() : fshSource.c_str();

    gl::Shader fragmentShader{::gl::GL_FRAGMENT_SHADER, fshPath + ";" + boost::algorithm::join( defines, ";" )};
    fragmentShader.setSource( shaderSource, SHADER_SOURCE_LENGTH );
    fragmentShader.compile();
    if( !fragmentShader.getCompileStatus() )
    {
        // Write out the expanded shader file.
        if( !fshPath.empty() )
            writeShaderToErrorFile( fshPath, shaderSource[2] );

        BOOST_LOG_TRIVIAL( error ) << "Compile failed for fragment shader '"
                                   << (fshPath.empty() ? "<none>" : fshPath) << "' with error '"
                                   << fragmentShader.getInfoLog() << "'.";

        return nullptr;
    }

    // Create and return the new Effect.
    auto shaderProgram = std::make_shared<ShaderProgram>();
    shaderProgram->m_handle.attach( vertexShader );
    shaderProgram->m_handle.attach( fragmentShader );
    shaderProgram->m_handle.link( vshPath + ";" + fshPath + ";" + boost::algorithm::join( defines, ";" ) );

    if( !shaderProgram->m_handle.getLinkStatus() )
    {
        BOOST_LOG_TRIVIAL( error ) << "Linking program failed (" << (vshPath.empty() ? "<none>" : vshPath) << ","
                                   << (fshPath.empty() ? "<none>" : fshPath) << "): "
                                   << shaderProgram->m_handle.getInfoLog();

        return nullptr;
    }

    for( auto&& attrib : shaderProgram->m_handle.getActiveAttributes() )
    {
        shaderProgram->m_vertexAttributes.insert( make_pair( attrib.getName(), std::move( attrib ) ) );
    }

    for( auto&& uniform : shaderProgram->m_handle.getActiveUniforms() )
    {
        shaderProgram->m_uniforms.emplace( make_pair( uniform.getName(), std::move( uniform ) ) );
    }

    return shaderProgram;
}

const std::string& ShaderProgram::getId() const
{
    return m_id;
}

const gl::Program::ActiveAttribute* ShaderProgram::getVertexAttribute(const std::string& name) const
{
    const auto it = m_vertexAttributes.find( name );
    return it == m_vertexAttributes.end() ? nullptr : &it->second;
}

const gl::Program::ActiveUniform* ShaderProgram::getUniform(const std::string& name) const
{
    const auto it = m_uniforms.find( name );
    return it == m_uniforms.end() ? nullptr : &it->second;
}

gl::Program::ActiveUniform* ShaderProgram::getUniform(const std::string& name)
{
    auto it = m_uniforms.find( name );
    return it == m_uniforms.end() ? nullptr : &it->second;
}

void ShaderProgram::bind() const
{
    m_handle.bind();
}
}
}
