# Nyxis Engine

## Building Instructions

Clone repository using 
```
    git clone https://github.com/itu-itis21-bandaliyev21/Nyxis-Engine.git --recursive
```

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

### Windows

#### Install Dependencies
- [Download and install MacOS Vulkan sdk](https://vulkan.lunarg.com/)
- [Download and install CMake](https://cmake.org/download/)
- [Install vcpkg]()

### Windows
#### Install Dependencies using vcpkg
- [Download and install Vulkan sdk](https://vulkan.lunarg.com/) and [vcpkg](https://vcpkg.io). Set path for glslc compiler from Vulkan sdk and vcpkg path in .env file

- Install glfw:
 ```
    vcpkg install glfw3:x64-windows
 ```
- Install glm:
```
    vcpkg install glm:x64-windows
```
- Install vulkan headers:
```
    vcpkg install vulkan:x64-windows
```

Then clone clone repository and build using provided build script:
```
    cd Vulkan_Engine
    .\WindowsBuild.ps1
```

The generated app will be in bin folder.
