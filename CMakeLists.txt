cmake_minimum_required(VERSION 2.8)

project(KitevaGameEngine CXX)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS}         -g -O0 -std=c++11")
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS}  -g -O0 -std=c++11")

add_subdirectory(engine)
