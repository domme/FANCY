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
* [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler)
* [Assimp](https://github.com/assimp/assimp)
* [GLM](https://github.com/g-truc/glm)
* [SPIRV-Reflect](https://github.com/chaoticbob/SPIRV-Reflect)
* [STB-Image](https://github.com/nothings/stb)
* [xxHash](https://github.com/Cyan4973/xxHash)
* [EASTL](https://github.com/electronicarts/EASTL)

## Building
A Visual Studio 2019 solution is included in the sources. The required data (binaries, libs, models, textures,...) is not included in this repository and needs to be downloaded here:
https://www.dropbox.com/s/0vh9ktvtxa8mztg/Fancy_Data.zip?dl=0
