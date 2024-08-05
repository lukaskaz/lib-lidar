cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 23)

project(lidar)

include(cmake/dependencies.cmake)
include(cmake/flags.cmake)

option(RUN_TESTS "Creates and runs unit tests for the project" OFF)

if(RUN_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

add_subdirectory(examples)

include_directories(inc)
file(GLOB SOURCES "src/*.cpp")

add_library(${PROJECT_NAME} ${SOURCES})
add_dependencies(${PROJECT_NAME} libserial)
target_link_libraries(${PROJECT_NAME}
    serial
)
