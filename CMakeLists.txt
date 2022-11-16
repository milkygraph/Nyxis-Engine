cmake_minimum_required(VERSION 3.22.1)

project(Vulkan_Engine)
set(PROJECT Vulkan_Engine)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
find_package(Vulkan REQUIRED)

find_package(glfw3 REQUIRED)

file (GLOB_RECURSE SOURCES src/*.cpp libs/*/*.cpp)
file (GLOB_RECURSE HEADERS include/*.hpp )
file (GLOB_RECURSE VENDOR )
file (GLOB_RECURSE VERTEX_SHADERS shaders/*.vert)
file (GLOB_RECURSE FRAGMENT_SHADERS shaders/*.frag)

set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(${PROJECT} ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT} PUBLIC include libs)
target_link_libraries(${PROJECT} Vulkan::Vulkan glfw)