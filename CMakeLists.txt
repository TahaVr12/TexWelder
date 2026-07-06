cmake_minimum_required(VERSION 3.15)
project(node_based_texture_maker CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_subdirectory(libs/raylib-6.0)

add_executable(TexWelder
    src/main.cpp
    src/classes/node.cpp
    src/classes/structs.cpp
    src/classes/extramenus.cpp
    src/classes/paramwidgets.cpp
)

target_link_libraries(TexWelder PRIVATE raylib)

add_custom_command(TARGET TexWelder POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/src/nodes/shaders
    $<TARGET_FILE_DIR:TexWelder>/nodes/shaders
)
