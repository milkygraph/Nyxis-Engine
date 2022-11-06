cmake_minimum_required(VERSION 3.22.1)

project(Vulkan CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

file (GLOB_RECURSE SOURCES src/*.cpp)
file (GLOB_RECURSE HEADERS include/*.hpp)

add_executable(Vulkan ${SOURCES} ${HEADERS})
target_include_directories(Vulkan PUBLIC include)
target_link_libraries(Vulkan Vulkan::Vulkan glfw)