cmake_minimum_required(VERSION 3.22.1)

project(Vulkan CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

file (GLOB_RECURSE SOURCES src/*.cpp libs/*/*.cpp)
file (GLOB_RECURSE HEADERS include/*.hpp )
file (GLOB_RECURSE VENDOR )
file (GLOB_RECURSE VERTEX_SHADERS shaders/*.vert)
file (GLOB_RECURSE FRAGMENT_SHADERS shaders/*.frag)

set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

execute_process(COMMAND glslc ${DIR}/shaders/simple_shader.vert -o ${DIR}/shaders/simple_shader.vert.spv)
execute_process(COMMAND glslc ${DIR}/shaders/simple_shader.frag -o ${DIR}/shaders/simple_shader.frag.spv)
execute_process(COMMAND glslc ${DIR}/shaders/point_light.vert -o ${DIR}/shaders/point_light.vert.spv)
execute_process(COMMAND glslc ${DIR}/shaders/point_light.frag -o ${DIR}/shaders/point_light.frag.spv)


add_executable(Vulkan ${SOURCES} ${HEADERS})
target_include_directories(Vulkan PUBLIC include libs)
target_link_libraries(Vulkan Vulkan::Vulkan glfw)