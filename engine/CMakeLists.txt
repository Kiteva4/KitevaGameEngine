cmake_minimum_required(VERSION 2.8)

project(KitevaGameEngine CXX)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(audio)
add_subdirectory(animation)
add_subdirectory(lib)
add_subdirectory(core)

include_directories(
    core/include
    )

target_link_libraries(${PROJECT_NAME}
    KGEAnimation
    KGEAudio
    KGECore
    KGELib
    pthread
    )
