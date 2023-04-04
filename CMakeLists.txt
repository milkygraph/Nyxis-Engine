cmake_minimum_required(VERSION 3.22.1)

project(Nyxis_Engine)
set(PROJECT Nyxis_Engine)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

file (GLOB_RECURSE SOURCES source/*.cpp libs/entt/*.cpp libs/stbimage/*.cpp libs/tinyobjloader/*.cpp libs/imgui/imgui.cpp 
        libs/imgui/imgui_demo.cpp libs/imgui/imgui_draw.cpp libs/imgui/imgui_tables.cpp libs/imgui/imgui_widgets.cpp 
        libs/imgui/misc/cpp/imgui_stdlib.cpp libs/imgui/imgui libs/imgui/backends/imgui_impl_glfw.cpp libs/imgui/backends/imgui_impl_vulkan.cpp
        libs/ImGuizmo/*/*.cpp libs/ImGuizmo/*.cpp) 
file (GLOB_RECURSE HEADERS source/*/*.hpp )
file (GLOB_RECURSE VENDOR )

add_executable(${PROJECT} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT} PUBLIC source libs libs/imgui libs/imgui/backends)
target_link_directories(${PROJECT} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs)
target_link_libraries(${PROJECT} PUBLIC Vulkan::Vulkan glfw assimp::assimp)

add_subdirectory(libs/spdlog)
add_subdirectory(libs/gli)
target_link_libraries(${PROJECT} PRIVATE spdlog gli)

set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT})
