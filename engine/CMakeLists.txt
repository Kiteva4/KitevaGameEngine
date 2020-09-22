cmake_minimum_required(VERSION 3.8)

project(engine)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(audio)
add_subdirectory(animation)
add_subdirectory(lib)
add_subdirectory(core)

include_directories(
    core/include
    )

target_link_libraries(${PROJECT_NAME}
    KGECore
    KGEAnimation
    KGEAudio
    KGELib
    pthread
    )
