set(GAMEPLAY_SRC
    src/AIAgent.cpp
    src/AIAgent.h
    src/AIController.cpp
    src/AIController.h
    src/AIMessage.cpp
    src/AIMessage.h
    src/AIState.cpp
    src/AIState.h
    src/AIStateMachine.cpp
    src/AIStateMachine.h
    src/Animation.cpp
    src/Animation.h
    src/AnimationClip.cpp
    src/AnimationClip.h
    src/AnimationController.cpp
    src/AnimationController.h
    src/Base.h
    src/BoundingBox.cpp
    src/BoundingBox.h
    src/BoundingSphere.cpp
    src/BoundingSphere.h
    src/Camera.cpp
    src/Camera.h
    src/Curve.cpp
    src/Curve.h
    src/DebugNew.cpp
    src/DebugNew.h
    src/DepthStencilTarget.cpp
    src/DepthStencilTarget.h
    src/Drawable.cpp
    src/Drawable.h
    src/Effect.cpp
    src/Effect.h
    src/FileSystem.cpp
    src/FileSystem.h
    src/Font.cpp
    src/Font.h
    src/FrameBuffer.cpp
    src/FrameBuffer.h
    src/Frustum.cpp
    src/Frustum.h
    src/Game.cpp
    src/Game.h
    src/Image.cpp
    src/Image.h
    src/Joint.cpp
    src/Joint.h
    src/Light.cpp
    src/Light.h
    src/Logger.cpp
    src/Logger.h
    src/Material.cpp
    src/Material.h
    src/MaterialParameter.cpp
    src/MaterialParameter.h
    src/MathUtil.cpp
    src/MathUtil.h
    src/Matrix.cpp
    src/Matrix.h
    src/Mesh.cpp
    src/Mesh.h
    src/MeshBatch.cpp
    src/MeshBatch.h
    src/MeshPart.cpp
    src/MeshPart.h
    src/MeshSkin.cpp
    src/MeshSkin.h
    src/Model.cpp
    src/Model.h
    src/Node.cpp
    src/Node.h
    src/ParticleEmitter.cpp
    src/ParticleEmitter.h
    src/Pass.cpp
    src/Pass.h
    src/Plane.cpp
    src/Plane.h
    src/Platform.h
    src/Platform.cpp
    src/Quaternion.cpp
    src/Quaternion.h
    src/Ray.cpp
    src/Ray.h
    src/Rectangle.cpp
    src/Rectangle.h
    src/RenderState.cpp
    src/RenderState.h
    src/RenderTarget.cpp
    src/RenderTarget.h
    src/Scene.cpp
    src/Scene.h
    src/ScreenDisplayer.cpp
    src/ScreenDisplayer.h
    src/Sprite.cpp
    src/Sprite.h
    src/SpriteBatch.cpp
    src/SpriteBatch.h
    src/Technique.cpp
    src/Technique.h
    src/Text.cpp
    src/Text.h
    src/Texture.cpp
    src/Texture.h
    src/TileSet.cpp
    src/TileSet.h
    src/Transform.cpp
    src/Transform.h
    src/Vector2.cpp
    src/Vector2.h
    src/Vector3.cpp
    src/Vector3.h
    src/Vector4.cpp
    src/Vector4.h
    src/VertexAttributeBinding.cpp
    src/VertexAttributeBinding.h
    src/VertexFormat.cpp
    src/VertexFormat.h
)

set(GAMEPLAY_RES
    res/logo_black.png
    res/logo_powered_black.png
    res/logo_powered_white.png
    res/logo_white.png
)

set(GAMEPLAY_RES_SHADERS
    res/shaders/colored.frag
    res/shaders/colored.vert
    res/shaders/font.frag
    res/shaders/font.vert
    res/shaders/form.frag
    res/shaders/form.vert
    res/shaders/lighting.frag
    res/shaders/lighting.vert
    res/shaders/skinning.vert
    res/shaders/skinning-none.vert
    res/shaders/sprite.frag
    res/shaders/sprite.vert
    res/shaders/terrain.frag
    res/shaders/terrain.vert
    res/shaders/textured.frag
    res/shaders/textured.vert
)

set(GAMEPLAY_RES_SHADERS
    res/ui/arial.gpb
    res/ui/default.png
    res/ui/default.theme
)

add_library(gameplay STATIC
    ${GAMEPLAY_SRC}
)
target_include_directories(gameplay PUBLIC src)

set(GAMEPLAY_VERSION 3.0.0)

# debug
if ( "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG" )
    target_compile_definitions(gameplay PUBLIC -D_DEBUG -DNOMINMAX)
endif()

target_link_libraries(gameplay PUBLIC GLEW GLFW Boost)

set_target_properties(gameplay PROPERTIES
    OUTPUT_NAME "gameplay"
    CLEAN_DIRECT_OUTPUT 1
    VERSION ${GAMEPLAY_VERSION}
)

source_group(res FILES ${GAMEPLAY_RES} ${GAMEPLAY_RES} ${GAMEPLAY_RES_SHADERS})
source_group(src FILES ${GAMEPLAY_SRC})
