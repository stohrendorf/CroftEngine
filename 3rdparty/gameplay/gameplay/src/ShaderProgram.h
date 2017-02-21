#pragma once

#include "Base.h"

#include "gl/program.h"

#include <map>
#include <memory>
#include <vector>

namespace gameplay
{
    /**
     * Defines an effect which can be applied during rendering.
     *
     * An effect essentially wraps an OpenGL program object, which includes the
     * vertex and fragment shader.
     *
     * In the future, this class may be extended to support additional logic that
     * typical effect systems support, such as GPU render state management,
     * techniques and passes.
     */
    class ShaderProgram : public std::enable_shared_from_this<ShaderProgram>
    {
    public:
        explicit ShaderProgram();
        ~ShaderProgram();

        /**
         * Creates an effect using the specified vertex and fragment shader.
         *
         * @param vshPath The path to the vertex shader file.
         * @param fshPath The path to the fragment shader file.
         * @param defines A new-line delimited list of preprocessor defines. May be nullptr.
         *
         * @return The created effect.
         */
        static std::shared_ptr<ShaderProgram> createFromFile(const std::string& vshPath, const std::string& fshPath, const std::vector<std::string>& defines = {});

        /**
         * Creates an effect from the given vertex and fragment shader source code.
         *
         * @param vshSource The vertex shader source code.
         * @param fshSource The fragment shader source code.
         * @param defines A new-line delimited list of preprocessor defines. May be nullptr.
         *
         * @return The created effect.
         */
        static std::shared_ptr<ShaderProgram> createFromSource(const std::string& vshSource, const std::string& fshSource, const std::vector<std::string>& defines = {});

        /**
         * Returns the unique string identifier for the effect, which is a concatenation of
         * the shader paths it was loaded from.
         */
        const std::string& getId() const;

        /**
         * Returns the vertex attribute handle for the vertex attribute with the specified name.
         *
         * @param name The name of the vertex attribute to return.
         *
         * @return The vertex attribute, or -1 if no such vertex attribute exists.
         */
        const gl::Program::ActiveAttribute* getVertexAttribute(const std::string& name) const;

        /**
         * Returns the uniform handle for the uniform with the specified name.
         *
         * @param name The name of the uniform to return.
         *
         * @return The uniform, or nullptr if no such uniform exists.
         */
        gl::Program::ActiveUniform* getUniform(const std::string& name) const;

        /**
         * Returns the specified active uniform.
         *
         * @param index The index of the uniform to return.
         *
         * @return The uniform, or nullptr if index is invalid.
         */
        gl::Program::ActiveUniform* getUniform(size_t index) const;

        /**
         * Returns the number of active uniforms in this effect.
         *
         * @return The number of active uniforms.
         */
        size_t getUniformCount() const;

        /**
         * Binds this effect to make it the currently active effect for the rendering system.
         */
        void bind();

        const gl::Program& getHandle() const
        {
            return m_handle;
        }

    private:

        ShaderProgram(const ShaderProgram&) = delete;
        ShaderProgram& operator=(const ShaderProgram&) = delete;

        static std::shared_ptr<ShaderProgram> createFromSource(const std::string& vshPath, const std::string& vshSource, const std::string& fshPath, const std::string& fshSource, const std::vector<std::string>& defines = {});

        std::string _id;
        gl::Program m_handle;
        std::map<std::string, gl::Program::ActiveAttribute> _vertexAttributes;
        mutable std::map<std::string, gl::Program::ActiveUniform> _uniforms;
    };
}
