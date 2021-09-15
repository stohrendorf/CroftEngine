include( get_freetype )
include( get_boost )
include( get_glew )
include( get_glfw )
include( get_glm )

add_custom_target( soglb-gen
        COMMENT "Generating structured OpenGL bindings"
        COMMAND ${Python3_EXECUTABLE} soglb_gen.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/gl
        BYPRODUCTS
        api/gl.hpp
        api/gl.cpp
        api/gles1.hpp
        api/gles1.cpp
        api/gles2.hpp
        api/gles2.cpp
        api/glsc2.hpp
        api/glsc2.cpp
        api/gl.xml
        )

add_library( soglb STATIC
        gl/bindableresource.h
        gl/buffer.h
        gl/debuggroup.h
        gl/framebuffer.h
        gl/image.h
        gl/pixel.h
        gl/program.h
        gl/program.cpp
        gl/shader.h
        gl/shader.cpp
        gl/vertexbuffer.h
        gl/texture.h
        gl/texture2d.h
        gl/texture2darray.h
        gl/texturedepth.h
        gl/typetraits.h
        gl/vertexarray.h
        gl/glassert.h
        gl/glassert.cpp
        gl/font.h
        gl/font.cpp
        gl/renderstate.h
        gl/renderstate.cpp
        gl/glew_init.h
        gl/glew_init.cpp
        gl/api/gl.cpp
        gl/window.h
        gl/window.cpp
        gl/cimgwrapper.h
        gl/cimgwrapper.cpp
        )

set_property(
        SOURCE gl/cimgwrapper.cpp
        PROPERTY COMPILE_DEFINITIONS NDEBUG
)

target_link_libraries( soglb PRIVATE CImg::CImg )
target_link_libraries(
        soglb
        PUBLIC
        GLEW::GLEW
        glfw
        gsl-lite::gsl-lite
        Freetype::Freetype
        Boost::log
        Boost::disable_autolinking
        utf8cpp::utf8cpp
        glm::glm
)
target_include_directories( soglb PUBLIC . )
