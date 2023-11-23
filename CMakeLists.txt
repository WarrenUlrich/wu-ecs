cmake_minimum_required(VERSION 3.15)

project(wu-ecs)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(
    ${PROJECT_NAME} INTERFACE 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(test)