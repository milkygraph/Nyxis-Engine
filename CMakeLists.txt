cmake_minimum_required(VERSION 3.22.1)

project(Vulkan CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

file (GLOB_RECURSE SOURCES src/*.cpp vendor/**/*.cpp vendor/**/*.c)
file (GLOB_RECURSE HEADERS include/*.hpp vendor/**/*.hpp)
file (GLOB_RECURSE VENDOR )
file (GLOB_RECURSE VERTEX_SHADERS shaders/*.vert)
file (GLOB_RECURSE FRAGMENT_SHADERS shaders/*.frag)

execute_process(COMMAND glslc ${VERTEX_SHADERS} -o ${VERTEX_SHADERS}.spv)
execute_process(COMMAND glslc ${FRAGMENT_SHADERS} -o ../shaders/simple_shader.frag.spv)


add_executable(Vulkan ${SOURCES} ${HEADERS})
target_include_directories(Vulkan PUBLIC include vendor)
target_link_libraries(Vulkan Vulkan::Vulkan glfw)