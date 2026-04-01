cmake_minimum_required(VERSION 3.27.2)

project(test_3d_game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(test_3d_game src/Camera.cpp src/Camera.h src/main.cpp)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

add_subdirectory(3rdparty/BadgerEngine)

target_link_libraries(test_3d_game PRIVATE Vulkan::Vulkan glfw freetype BadgerEngine)

badger_engine_add_embedded_shader_target(
    shaders
    OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/shaders
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.frag
    ${CMAKE_CURRENT_LIST_DIR}/shaders/prism.frag
    ${CMAKE_CURRENT_LIST_DIR}/shaders/lit.frag
    ${CMAKE_CURRENT_LIST_DIR}/shaders/unlit.frag
    ${CMAKE_CURRENT_LIST_DIR}/shaders/unlit_shadow.frag
    ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.vert
    ${CMAKE_CURRENT_LIST_DIR}/shaders/vert_uniform.vert
    ${CMAKE_CURRENT_LIST_DIR}/shaders/vert_uniform_bloated.vert)

badger_engine_add_embedded_target(
    assets
    STATIC
    OUTPUT_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/assets
    SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/assets/fonts/ZCOOL.ttf
    ${CMAKE_CURRENT_LIST_DIR}/assets/models/cube.obj
    ${CMAKE_CURRENT_LIST_DIR}/assets/models/monkey.obj
    ${CMAKE_CURRENT_LIST_DIR}/assets/models/ship.bin
    ${CMAKE_CURRENT_LIST_DIR}/assets/models/ship.gltf
    ${CMAKE_CURRENT_LIST_DIR}/assets/models/untitled.obj
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Body.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Body_AO.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Body_NM2.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Guns.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Guns_AO.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Metal_001_AO-Metal_001_roughness.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Metal_001_normal.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/Metal_001_roughness.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/MissileRack.png
    ${CMAKE_CURRENT_LIST_DIR}/assets/textures/MissileRack_AO.png)

target_link_libraries(test_3d_game PRIVATE shaders assets)
