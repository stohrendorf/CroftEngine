#include "glassert.h"

#include "api/gl_api_provider.hpp"

namespace render
{
namespace gl
{
#ifndef NDEBUG
void checkGlError(const char* code)
{
    const auto error = glGetError();
    if(error == GL_NO_ERROR)
        return;

    BOOST_LOG_TRIVIAL(error) << "OpenGL error " << error << " after evaluation of '" << code
                             << "': " << (const char*)gluErrorString(error);
    BOOST_LOG_TRIVIAL(error) << "Stacktrace:\n" << boost::stacktrace::stacktrace();
    BOOST_ASSERT_MSG(false, code);
}
#endif
} // namespace gl
} // namespace render
