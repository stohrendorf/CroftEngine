set( GAMEPLAY_SRC
     src/Base.h
     src/BoundingBox.cpp
     src/BoundingBox.h
     src/Camera.cpp
     src/Camera.h
     src/Drawable.cpp
     src/Drawable.h
     src/ShaderProgram.cpp
     src/ShaderProgram.h
     src/FileSystem.cpp
     src/FileSystem.h
     src/Font.cpp
     src/Font.h
     src/Frustum.cpp
     src/Frustum.h
     src/Game.cpp
     src/Game.h
     src/Image.cpp
     src/Image.h
     src/Material.cpp
     src/Material.h
     src/MaterialParameter.cpp
     src/MaterialParameter.h
     src/Mesh.cpp
     src/Mesh.h
     src/MeshBatch.cpp
     src/MeshBatch.h
     src/MeshPart.cpp
     src/MeshPart.h
     src/Model.cpp
     src/Model.h
     src/Node.cpp
     src/Node.h
     src/ParticleEmitter.cpp
     src/ParticleEmitter.h
     src/Plane.cpp
     src/Plane.h
     src/Ray.cpp
     src/Ray.h
     src/Rectangle.cpp
     src/Rectangle.h
     src/RenderContext.h
     src/RenderState.cpp
     src/RenderState.h
     src/Scene.cpp
     src/Scene.h
     src/ScreenOverlay.cpp
     src/ScreenOverlay.h
     src/Sprite.cpp
     src/Sprite.h
     src/SpriteBatch.cpp
     src/SpriteBatch.h
     src/VertexAttributeBinding.cpp
     src/VertexAttributeBinding.h
     src/VertexFormat.cpp
     src/VertexFormat.h
     src/Visitor.h
     src/Visitor.cpp
     src/RenderContext.h
     src/gl/util.h
     src/gl/shader.h
     src/gl/program.h
     src/gl/bindableresource.h
     src/gl/framebuffer.h
     src/gl/indexbuffer.h
     src/gl/renderbuffer.h
     src/gl/rendertarget.h
     src/gl/texture.h
     src/gl/vertexarray.h
     src/gl/vertexbuffer.h
)

find_package(OpenGL REQUIRED)

add_library(gameplay STATIC
    ${GAMEPLAY_SRC}
)
if(NOT MSVC)
    target_compile_options(gameplay PRIVATE -Wall -Wextra)
endif()
target_include_directories(gameplay PUBLIC src)

set(GAMEPLAY_VERSION 3.0.0)

# debug
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG" )
    target_compile_definitions(gameplay PUBLIC -D_DEBUG -DNOMINMAX)
endif()

target_compile_definitions(gameplay PUBLIC -DGLM_FORCE_RADIANS)

target_link_libraries(gameplay PUBLIC GLEW GLFW Boost OpenGL glm freetype GSL)

set_target_properties(gameplay PROPERTIES
    OUTPUT_NAME "gameplay"
    CLEAN_DIRECT_OUTPUT 1
    VERSION ${GAMEPLAY_VERSION}
)

GROUP_FILES(${GAMEPLAY_SRC})
