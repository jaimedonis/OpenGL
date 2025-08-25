# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

cmake_minimum_required(VERSION 3.22.1)

project("my_mobile_app")

# Creates your game shared library. The name must be the same as the
# one used for loading in your Kotlin/Java or AndroidManifest.txt files.
add_library(my_mobile_app SHARED
        main.cpp
        AndroidOut.cpp
        Renderer.cpp
        Shader.cpp
        TextureAsset.cpp
        Utility.cpp
        GltfMeshModelLoader.cpp
        MeshModelBuilder.cpp
        Model.cpp
        external/tiny_gltf/tiny_gltf.cc
        scene/CameraBaseNode.cpp
        scene/OrthographicCamera.cpp
        scene/PerspectiveCamera.cpp
        scene/RenderObject.cpp
        scene/SceneGraph.cpp
        scene/Transform.cpp
)

# Searches for a package provided by the game activity dependency
find_package(game-activity REQUIRED CONFIG)

add_subdirectory(external/glm)

target_include_directories(my_mobile_app PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/external/glm
        ${CMAKE_CURRENT_SOURCE_DIR}/external/tiny_gltf)

# Configure libraries CMake uses to link your target library.
target_link_libraries(my_mobile_app
        # The game activity
        game-activity::game-activity
        glm
        # EGL and other dependent libraries required for drawing
        # and interacting with Android system
        EGL
        GLESv3
        jnigraphics
        android
        log)

