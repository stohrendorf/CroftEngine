#pragma once

#include "VertexFormat.h"

#include "gl/vertexarray.h"

#include <memory>

namespace gameplay
{
    class Mesh;
    class ShaderProgram;


    /**
     * Defines a binding between the vertex layout of a Mesh and the vertex
     * input attributes of a vertex shader (Effect).
     *
     * In a perfect world, this class would always be a binding directly between
     * a unique VertexFormat and an Effect, where the VertexFormat is simply the
     * definition of the layout of any anonymous vertex buffer. However, the OpenGL
     * mechanism for setting up these bindings is Vertex Array Objects (VAOs).
     * OpenGL requires a separate VAO per vertex buffer object (VBO), rather than per
     * vertex layout definition. Therefore, although we would like to define this
     * binding between a VertexFormat and Effect, we are specifying the binding
     * between a Mesh and Effect to satisfy the OpenGL requirement of one VAO per VBO.
     *
     * Note that this class still does provide a binding between a VertexFormat
     * and an Effect, however this binding is actually a client-side binding and
     * should only be used when writing custom code that use client-side vertex
     * arrays, since it is slower than the server-side VAOs used by OpenGL
     * (when creating a VertexAttributeBinding between a Mesh and Effect).
     */
    class VertexAttributeBinding : public gl::VertexArray
    {
    public:
        explicit VertexAttributeBinding(Mesh* mesh, const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<ShaderProgram>& shaderProgram);

        explicit VertexAttributeBinding(Mesh* mesh, const std::shared_ptr<ShaderProgram>& shaderProgram);


        explicit VertexAttributeBinding(const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<ShaderProgram>& shaderProgram)
            : VertexAttributeBinding{nullptr, vertexFormat, vertexPointer, shaderProgram}
        {
        }


        ~VertexAttributeBinding();

    private:

        VertexAttributeBinding& operator=(const VertexAttributeBinding&) = delete;

        void setVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalize, GLsizei stride, void* pointer);
    };
}
