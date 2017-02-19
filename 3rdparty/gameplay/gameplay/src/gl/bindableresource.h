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
            }


            virtual ~BindableResource()
            {
                unbind();
                m_deleter(1, &m_handle);
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
