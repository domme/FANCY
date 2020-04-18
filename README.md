# FANCY

A small rendering framework that abstracts DX12 and Vulkan.
Mostly created for personal learning-purposes and graphics-demos. Most areas are still under constructions, but feel free to browse the code for reference in your own projects.

## Features
* Rendering abstraction API for both DX12 and Vulkan (see the Vulkan-branch). No DX12/Vulkan-specific code in high-level rendering API
* Optional code-modules for Scene-management and GUI rendering
* Built-in profiler for both CPU and GPU
* Custom memory allocator for DX12 (Vulkan still under construction)
* Supports async compute
* Shader hot-reloading
* HLSL->SPIR-V compilation using the [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
* Import of most common 3D files using [Assimp](https://github.com/assimp/assimp)
* Simplified, high-level resource-barrier design similar to the [Simple Vulkan Synchronization](https://github.com/Tobski/simple_vulkan_synchronization) project that maps to both DX12 and Vulkan

## Used Libraries
* [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
* [Assimp](https://github.com/assimp/assimp)
* [GLM](https://github.com/g-truc/glm)
* [SPIRV-Reflect](https://github.com/chaoticbob/SPIRV-Reflect)
* [STB-Image](https://github.com/nothings/stb)
* [xxHash](https://github.com/Cyan4973/xxHash)

## Building
A Visual Studio 2017 solution is included in the sources. The required dependency binaries for the used libraries can be downloaded here:
https://www.dropbox.com/sh/npuok5i4xkiju4o/AAC-UplWpQF970a6dhW3_XCUa?dl=0
