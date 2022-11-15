# Vulkan_Engine

## Building Instructions

## Unix
- Install dependencies using your package manager
For example, 
```
    sudo apt install libvulkan-dev
    sudo apt install libglfw3-dev
    sudo apt install libglm-dev
    sudo apt install cmake
```
- [Install glslc](https://github.com/google/shaderc#downloads)
- To Build:
```
    cd Vulkan_Engine
    chmod +x UnixBuild.sh
    ./UnixBuild.sh
```
The generated app will be in bin folder.

### MacOS

#### Install Dependencies

- [Download and install MacOS Vulkan sdk](https://vulkan.lunarg.com/)
- [Download and install Homebrew](https://brew.sh/)
- Install libraries
```
    brew install cmake
    brew install glfw
    brew install glm
```
To Build:
```
    cd Vulkan_Engine
    chmod +x UnixBuild.sh
    ./UnixBuild.sh
```
