#pragma once

#include "VertexFormat.h"


namespace gameplay
{
    class Mesh;
    class Effect;


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
    class VertexAttributeBinding : std::enable_shared_from_this<VertexAttributeBinding>
    {
    public:
        explicit VertexAttributeBinding();

        ~VertexAttributeBinding();

        /**
         * Creates a new VertexAttributeBinding between the given Mesh and Effect.
         *
         * If a VertexAttributeBinding matching the specified Mesh and Effect already
         * exists, it will be returned. Otherwise, a new VertexAttributeBinding will
         * be returned. If OpenGL VAOs are enabled, the a new VAO will be created and
         * stored in the returned VertexAttributeBinding, otherwise a client-side
         * array of vertex attribute bindings will be stored.
         *
         * @param mesh The mesh.
         * @param effect The effect.
         *
         * @return A VertexAttributeBinding for the requested parameters.
         * @script{create}
         */
        static std::shared_ptr<VertexAttributeBinding> create(const std::shared_ptr<Mesh>& mesh, const std::shared_ptr<Effect>& effect);

        /**
         * Creates a client-side vertex attribute binding.
         *
         * This method creates a client-side vertex attribute binding, which is simply a cached
         * set of parameters that need to be passed to the renderer to setup vertex attribute
         * bindings between a vertex buffer and a vertex shader. The specified vertexPointer is
         * a client-side block of memory that contains the vertices to be send to the renderer,
         * formatted as indicated in the specified vertexFormat parameter.
         *
         * @param vertexFormat The vertex format.
         * @param vertexPointer Pointer to beginning of client-side vertex array.
         * @param effect The effect.
         *
         * @return A VertexAttributeBinding for the requested parameters.
         * @script{ignore}
         */
        static std::shared_ptr<VertexAttributeBinding> create(const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<Effect>& effect);

        /**
         * Binds this vertex array object.
         */
        void bind();

        /**
         * Unbinds this vertex array object.
         */
        void unbind();

    private:

        class VertexAttribute
        {
        public:
            bool enabled;
            int size;
            GLenum type;
            bool normalized;
            unsigned int stride;
            void* pointer;
        };


        VertexAttributeBinding& operator=(const VertexAttributeBinding&) = delete;

        static std::shared_ptr<VertexAttributeBinding> create(const std::shared_ptr<Mesh>& mesh, const VertexFormat& vertexFormat, void* vertexPointer, const std::shared_ptr<Effect>& effect);

        void setVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalize, GLsizei stride, void* pointer);

        GLuint _handle;
        std::vector<VertexAttribute> _attributes;
        std::shared_ptr<Mesh> _mesh;
        std::shared_ptr<Effect> _effect;
    };
}
