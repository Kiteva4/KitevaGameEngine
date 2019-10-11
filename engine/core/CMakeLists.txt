cmake_minimum_required(VERSION 2.8)

project(KGECore CXX)

file(GLOB_RECURSE HDRS *.h)
file(GLOB_RECURSE SRCS *.cpp)

add_library(${PROJECT_NAME} STATIC ${SRCS} ${HDRS})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "lib")
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "KGEAnimation")
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".a")

include_directories(${PROJECT_SOURCE_DIR}/include/)
include_directories(${PROJECT_SOURCE_DIR}/src/)

