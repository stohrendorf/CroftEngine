#pragma once

#include "vertexattribute.h"

#include "gl/vertexbuffer.h"
#include "gl/program.h"

#include <gsl/gsl>

#include <map>


namespace gameplay
{
    namespace ext
    {
        class StructuredVertexBuffer : public gl::VertexBuffer
        {
        public:
            using AttributeMapping = std::map<std::string, VertexAttribute>;


            explicit StructuredVertexBuffer(const AttributeMapping& mapping, bool dynamic, const std::string& label = {})
                : VertexBuffer{label}
                , m_mapping{mapping}
                , m_dynamic{dynamic}
                , m_vertexCount{0}
            {
                BOOST_ASSERT(!mapping.empty());

                m_size = -1;
                for( const auto& attrib : mapping )
                {
                    auto tmp = attrib.second.getStride();
                    if( m_size != -1 && m_size != tmp )
                    {
                        BOOST_THROW_EXCEPTION(std::runtime_error("Inconsistent stride in vertex attribute mapping"));
                    }

                    m_size = tmp;
                }

                BOOST_ASSERT(m_size > 0);
            }


            void bind(const gl::Program& program) const
            {
                VertexBuffer::bind();

                for( const auto& attrib : program.getActiveAttributes() )
                {
                    auto it = m_mapping.find(attrib.getName());
                    if( it == m_mapping.end() )
                        continue;

                    it->second.bind(gsl::narrow<GLuint>(attrib.getLocation()));
                }
            }


            template<typename T>
            void assignSub(const gsl::not_null<const T*>& vertexData, size_t vertexStart, size_t vertexCount)
            {
                if( sizeof(T) != m_size )
                {
                    BOOST_THROW_EXCEPTION(std::invalid_argument("Trying to assign vertex data which has a different size than specified in the format"));
                }

                VertexBuffer::bind();

                if( vertexCount == 0 )
                {
                    vertexCount = m_vertexCount - vertexStart;
                }

                glBufferSubData(GL_ARRAY_BUFFER, vertexStart * m_size, vertexCount * m_size, vertexData);
                gl::checkGlError();
            }


            template<typename T>
            void assign(const gsl::not_null<const T*>& vertexData, size_t vertexCount)
            {
                if( sizeof(T) != m_size )
                {
                    BOOST_THROW_EXCEPTION(std::invalid_argument("Trying to assign vertex data which has a different size than specified in the format"));
                }

                VertexBuffer::bind();

                if( vertexCount != 0 )
                    m_vertexCount = vertexCount;

                glBufferData(GL_ARRAY_BUFFER, m_size * m_vertexCount, vertexData, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
                gl::checkGlError();
            }


            template<typename T>
            void assignRaw(const gsl::not_null<const T*>& vertexData, size_t vertexCount)
            {
                VertexBuffer::bind();

                if( vertexCount != 0 )
                    m_vertexCount = vertexCount;

                glBufferData(GL_ARRAY_BUFFER, m_size * m_vertexCount, vertexData, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
                gl::checkGlError();
            }


            template<typename T>
            void assign(const std::vector<T>& data)
            {
                assign<T>(data.data(), data.size());
            }


            template<typename T>
            void assignRaw(const std::vector<T>& data, size_t vertexCount)
            {
                assignRaw<T>(data.data(), vertexCount);
            }


            void reserve(size_t n)
            {
                m_vertexCount = n;
                glBufferData(GL_ARRAY_BUFFER, m_size * m_vertexCount, nullptr, m_dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
            }


            template<typename T>
            T* mapTypedRw()
            {
                if( sizeof(T) != m_size )
                {
                    BOOST_THROW_EXCEPTION(std::invalid_argument("Trying to map vertex data which has a different size than specified in the format"));
                }

                return static_cast<T*>(mapRw());
            }


            template<typename T>
            const T* mapTyped()
            {
                if( sizeof(T) != m_size )
                {
                    BOOST_THROW_EXCEPTION(std::invalid_argument("Trying to map vertex data which has a different size than specified in the format"));
                }

                return static_cast<T*>(map());
            }


            size_t getVertexCount() const noexcept
            {
                return m_vertexCount;
            }


            bool isDynamic() const noexcept
            {
                return m_dynamic;
            }


            GLsizei getVertexSize() const noexcept
            {
                return m_size;
            }


            const AttributeMapping& getAttributeMapping() const
            {
                return m_mapping;
            }


        private:
            const AttributeMapping m_mapping;

            GLsizei m_size = -1;

            bool m_dynamic;

            size_t m_vertexCount;
        };
    }
}
