cmake_minimum_required(VERSION 3.16)
project(pixel_free_bridge LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LINUX_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../SMBeautyEngine_linux")
set(GLAD_SOURCE_DIR "${LINUX_SDK_DIR}/third_party/glad/src")
set(GLAD_INCLUDE_DIR "${LINUX_SDK_DIR}/third_party/glad/include")
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lib")

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_library(pixel_free_bridge SHARED
    pixel_free_bridge.cpp
    ${LINUX_SDK_DIR}/src/opengl.cpp
    ${GLAD_SOURCE_DIR}/glad.cc
)

set_target_properties(pixel_free_bridge PROPERTIES
    PREFIX ""
    OUTPUT_NAME "pixel_free_bridge"
    LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_DIR}"
    BUILD_RPATH "$ORIGIN:$ORIGIN/../../SMBeautyEngine_linux/pixelfreeLib"
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../../SMBeautyEngine_linux/pixelfreeLib"
)

target_include_directories(pixel_free_bridge PRIVATE
    ${LINUX_SDK_DIR}/include
    ${LINUX_SDK_DIR}/pixelfreeLib/Include
    ${GLAD_INCLUDE_DIR}
)

target_link_libraries(pixel_free_bridge PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
    X11
    pthread
    dl
    ${LINUX_SDK_DIR}/pixelfreeLib/libPixelFree.so
)
