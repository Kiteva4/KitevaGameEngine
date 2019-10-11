cmake_minimum_required(VERSION 2.8)

project(KitevaGameEngine CXX)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(audio)
add_subdirectory(animation)
add_subdirectory(lib)
add_subdirectory(core)

target_link_libraries(${PROJECT_NAME}
    KGEAnimation
    KGEAudio
    KGECore
    KGELib
    pthread
    )

link_directories(
    ${CMAKE_SOURCE_DIR}/build/engine/animation/
    ${CMAKE_SOURCE_DIR}/build/engine/audio/
    ${CMAKE_SOURCE_DIR}/build/engine/core/
    ${CMAKE_SOURCE_DIR}/build/engine/lib/
    )
