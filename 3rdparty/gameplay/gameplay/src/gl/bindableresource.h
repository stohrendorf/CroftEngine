#pragma once

#include "util.h"

#include <functional>


namespace gameplay
{
    namespace gl
    {
        class BindableResource
        {
        public:
            void bind() const
            {
                if( m_handle == 0 )
                    return;

                m_binder(m_handle);
                checkGlError();
            }


            void unbind() const
            {
                m_binder(0);
                checkGlError();
            }


            GLuint getHandle() const
            {
                BOOST_ASSERT(m_handle != 0);
                return m_handle;
            }


        protected:
            using Allocator = void(GLsizei, GLuint*);
            using Binder = void(GLuint);
            using Deleter = void(GLsizei, GLuint*);


            explicit BindableResource(const std::function<Allocator>& allocator, const std::function<Binder>& binder, const std::function<Deleter>& deleter)
                : BindableResource{allocator, binder, deleter, GLenum(-1), {}}
            {
            }


            explicit BindableResource(const std::function<Allocator>& allocator, const std::function<Binder>& binder, const std::function<Deleter>& deleter, GLenum identifier, const std::string& label)
                : m_allocator{allocator}
                , m_binder{binder}
                , m_deleter{deleter}
            {
                BOOST_ASSERT(allocator != nullptr);
                BOOST_ASSERT(binder != nullptr);
                BOOST_ASSERT(deleter != nullptr);

                m_allocator(1, &m_handle);
                checkGlError();

                BOOST_ASSERT(m_handle != 0);

                if( !label.empty() )
                {
                    // An object must be created (not only reserved) to be able to have a label assigned;
                    // for certain types of resources, this may fail, e.g. programs which must be linked
                    // before they are considered "created".
                    bind();
                    setLabel(identifier, label);
                    unbind();
                }
            }


            explicit BindableResource(BindableResource&& rhs)
                : m_handle{std::move(rhs.m_handle)}
                , m_allocator{move(rhs.m_allocator)}
                , m_binder{move(rhs.m_binder)}
                , m_deleter{move(rhs.m_deleter)}
            {
                rhs.m_handle = 0;
            }


            virtual ~BindableResource()
            {
                if( m_handle == 0 )
                    return;

                unbind();
                m_deleter(1, &m_handle);
                checkGlError();
            }


            void setLabel(GLenum identifier, const std::string& label)
            {
                GLint maxLabelLength = 0;
                glGetIntegerv(GL_MAX_LABEL_LENGTH, &maxLabelLength);
                checkGlError();
                BOOST_ASSERT(maxLabelLength > 0);

                glObjectLabel(identifier, m_handle, -1, label.empty() ? nullptr : label.substr(0, static_cast<size_t>(maxLabelLength)).c_str());
                checkGlError();
            }


        private:
            GLuint m_handle = 0;

            std::function<Allocator> m_allocator;

            std::function<Binder> m_binder;

            std::function<Deleter> m_deleter;

            BindableResource(const BindableResource&) = delete;

            BindableResource& operator=(const BindableResource&) = delete;
        };
    }
}
