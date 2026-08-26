cmake_minimum_required(VERSION 3.15)
project(tower_defense_cpp CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tắt Wayland và các thành phần không cần thiết của GLFW
set(GLFW_BUILD_WAYLAND OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Biên dịch mã nguồn GLFW
add_subdirectory(externals/glfw-3.5.1)

set(CHAI3D_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/chai3d")

include_directories(
    include
    ${CHAI3D_DIR}/src
    ${CHAI3D_DIR}/externals/Eigen
    ${CHAI3D_DIR}/externals/glew/include
    externals/glfw-3.5.1/include
    /usr/include/eigen3
)

link_directories(
    ${CHAI3D_DIR}/lib/release/lin-x86_64-cc
)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(tower_defense_cpp ${SOURCES})

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(tower_defense_cpp PRIVATE
    chai3d
    glfw
    OpenGL::GL
    GLU
    Threads::Threads
    X11
    asound
    usb-1.0
    dl
)