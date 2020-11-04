find_package( Freetype REQUIRED )
find_package( Boost COMPONENTS system log REQUIRED )
find_package( GLEW REQUIRED )
find_package( glfw3 3.3 REQUIRED )
target_compile_definitions( glfw INTERFACE -DGLFW_INCLUDE_NONE )

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
        gl/shader.h
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

target_link_libraries( soglb PRIVATE PRIVATE CImg::CImg )
target_link_libraries( soglb PUBLIC GLEW::GLEW glfw )
target_link_libraries( soglb PUBLIC gsl-lite Freetype::Freetype ${Boost_LIBRARIES} )
target_include_directories( soglb PUBLIC . ${Boost_INCLUDE_DIR} )
target_compile_definitions( soglb PUBLIC -DBOOST_LOG_DYN_LINK=0 -DBOOST_ALL_NO_LIB )
