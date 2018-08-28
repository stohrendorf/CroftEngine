set( GAMEPLAY_SRC
     src/Base.h
     src/Camera.cpp
     src/Camera.h
     src/Drawable.h
     src/ShaderProgram.cpp
     src/ShaderProgram.h
     src/Game.cpp
     src/Game.h
     src/Material.cpp
     src/Material.h
     src/MaterialParameter.cpp
     src/MaterialParameter.h
     src/Mesh.cpp
     src/Mesh.h
     src/MeshPart.cpp
     src/MeshPart.h
     src/Model.cpp
     src/Model.h
     src/Node.cpp
     src/Node.h
     src/Rectangle.h
     src/RenderContext.h
     src/RenderState.cpp
     src/RenderState.h
     src/Scene.h
     src/ScreenOverlay.cpp
     src/ScreenOverlay.h
     src/Sprite.cpp
     src/Sprite.h
     src/Visitor.h
     src/Visitor.cpp
     src/gl/util.h
     src/gl/shader.h
     src/gl/program.h
     src/gl/bindableresource.h
     src/gl/framebuffer.h
     src/gl/indexbuffer.h
     src/gl/pixel.h
     src/gl/renderbuffer.h
     src/gl/rendertarget.h
     src/gl/texture.h
     src/gl/typetraits.h
     src/gl/vertexarray.h
     src/gl/vertexbuffer.h
     src/gl/debuggroup.h
     src/gl/font.cpp
     src/gl/font.h
     src/gl/image.h
     src/gl/structuredvertexbuffer.h
     src/gl/vertexattribute.h
     src/gsl_util.h )

add_library( gameplay STATIC ${GAMEPLAY_SRC} )
if( NOT MSVC )
    target_compile_options( gameplay PRIVATE -Wall -Wextra )
endif()
target_include_directories( gameplay PUBLIC src )

target_link_libraries( gameplay PUBLIC GLEW::GLEW glfw Boost::boost OpenGL glm Freetype::Freetype )

GROUP_FILES( ${GAMEPLAY_SRC} )
