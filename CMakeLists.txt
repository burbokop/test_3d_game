cmake_minimum_required(VERSION 3.27.2)

project(test_3d_game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(test_3d_game src/main.cpp
    src/Camera.h src/Camera.cpp)

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
    ${CMAKE_CURRENT_LIST_DIR}/shaders/shader.vert
    ${CMAKE_CURRENT_LIST_DIR}/shaders/vert_uniform.vert
    ${CMAKE_CURRENT_LIST_DIR}/shaders/vert_uniform_bloated.vert)

add_dependencies(test_3d_game shaders)
target_link_libraries(test_3d_game PRIVATE shaders)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/fonts DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/models DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_LIST_DIR}/textures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
