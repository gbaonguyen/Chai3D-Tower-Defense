cmake_minimum_required(VERSION 3.15)
project(TowerDefense CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CHAI3D_DIR "${CMAKE_CURRENT_SOURCE_DIR}/externals/chai3d")

include_directories(
    include
    ${CHAI3D_DIR}/src
    ${CHAI3D_DIR}/external/Eigen
    ${CHAI3D_DIR}/external/glew/include
)

# Trỏ đúng vào thư mục vừa tìm thấy
link_directories(
    ${CHAI3D_DIR}/lib/release/lin-x86_64-cc
)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(TowerDefense ${SOURCES})

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(TowerDefense PRIVATE
    chai3d
    OpenGL::GL
    GLU
    Threads::Threads
    X11
    asound
    usb-1.0
    dl
)