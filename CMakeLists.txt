cmake_minimum_required(VERSION 2.8)

project(KitevaGameEngine CXX)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -m64 -g -O0 -std=c++17")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -m64 -g -O0 -std=c++17")

include_directories(libs)
include_directories(external)

add_subdirectory(engine)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
