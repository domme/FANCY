# FANCY

A small rendering framework that abstracts DX12 and Vulkan.
Mostly created for personal learning-purposes and graphics-demos. Most areas are still under constructions, but feel free to browse the code for reference in your own projects.

## Features
* Rendering abstraction API for both DX12 and Vulkan. No DX12/Vulkan-specific code in high-level rendering API
* Optional code-modules for Scene-management and GUI rendering
* Built-in profiler for both CPU and GPU
* Custom memory allocator for DX12 (planned for Vulkan as well)
* Supports async compute
* Shader hot-reloading
* HLSL->SPIR-V compilation using the [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
* Import of most common 3D files using [Assimp](https://github.com/assimp/assimp)
* Implicit and automatic hazard-tracking for resources

## Used Libraries
* [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler) (DirectX DXC)
* [Assimp](https://github.com/assimp/assimp)
* [GLM](https://github.com/g-truc/glm)
* [STB-Image](https://github.com/nothings/stb)
* [xxHash](https://github.com/Cyan4973/xxHash)
* [EASTL](https://github.com/electronicarts/EASTL)
* [WinPixEventRuntime](https://devblogs.microsoft.com/pix/) (GPU profiling)

## Building

This project uses [vcpkg](https://github.com/microsoft/vcpkg) as the unified package manager for all external dependencies. All required packages are declared in the `vcpkg.json` manifest file located in `external/vcpkg/`. WinPixEventRuntime is downloaded separately via a bootstrap script.

### Prerequisites

You will need:
* Visual Studio 2022 with C++ workload
* Git (with support for git submodules)
* CMake 4.3 or newer
* [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) (set VK_SDK_PATH environment variable)

### Build Steps

1. Clone the repository with all submodules:
   ```
   git clone --recurse-submodules https://github.com/yourusername/PathTracer.git
   cd PathTracer/FANCY
   ```

2. Download WinPixEventRuntime (one-time setup):
   ```
   powershell -ExecutionPolicy Bypass -File bootstrap_winpix.ps1
   ```

3. Install vcpkg dependencies from the manifest:
   ```
   cd external/vcpkg
   .\vcpkg.exe install
   cd ../..
   ```

4. Generate the project using CMake from the PathTracer root:
   ```
   cd ../..
   cmake --preset vs2022-win64
   ```

5. Build the solution:
   ```
   cmake --build _cmake_build --config Release
   ```

**Important:** Always run `vcpkg install` from the `FANCY/external/vcpkg/` directory where `vcpkg.json` is located. This ensures packages are installed to the correct location (`FANCY/external/vcpkg/installed/`) that CMake expects.