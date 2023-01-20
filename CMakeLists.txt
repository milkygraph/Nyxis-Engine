cmake_minimum_required(VERSION 3.22.1)

project(Vulkan_Engine)
set(PROJECT Vulkan_Engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)


file (GLOB_RECURSE SOURCES src/*.cpp Scene/*.cpp libs/imgui/*.cpp libs/entt/*.cpp libs/stbimage/*.cpp libs/tinyobjloader/*.cpp)
file (GLOB_RECURSE HEADERS include/*.hpp )
file (GLOB_RECURSE VENDOR )
file (GLOB_RECURSE VERTEX_SHADERS shaders/*.vert)
file (GLOB_RECURSE FRAGMENT_SHADERS shaders/*.frag)

foreach(SHADER ${VERTEX_SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    add_custom_command(
            OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/bin/shaders/${FILE_NAME}.spv
            COMMAND glslc -c ${FILE_NAME}
            DEPENDS ${SHADER}
    )
endforeach(SHADER)


add_executable(${PROJECT} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT} PUBLIC include Scene libs)
target_link_directories(${PROJECT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(${PROJECT} PUBLIC Vulkan::Vulkan glfw)

add_subdirectory(libs/spdlog)
target_link_libraries(${PROJECT} PRIVATE spdlog::spdlog)