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
A Visual Studio 2019 solution is included in the sources. In order to build this solution, all git submodules must be properly initialized and some external prebuilt libraries need to be provided. Fancy uses [vcpgk](https://github.com/microsoft/vcpkg) for most of these libraries (all except DirectXShaderCompiler since this is not included in vcpkg).

If you don't already have vcpkg installed and set up (including the global Visual Studio integration), please checkout [vcpkg](https://github.com/microsoft/vcpkg) follow the [setup-steps fow windows](https://github.com/microsoft/vcpkg#quick-start-windows).
With a properly set up vcpkg, please execute "get_external_libs.bat" located in the FANCY root folder using cmd or powershell and provide it with the path to the vcpgk root folder as its first argument (e.g. ".\get_external_libs.bat C:/vcpgk/"). This will download and install all required external libraries that are part of vcpkg (e.g. Assimp, EASTL and xxHash).

The DirectXShaderCompiler is the only required library that's not part of vcpkg unfortunately. You can use the bat-file "get_directx_shadercompiler.bat" which downloads and unpacks the specific [release-version](https://github.com/microsoft/DirectXShaderCompiler/releases) of DXC that FANCY currently uses. Alternatively, you can also do this manually by extracting the contents of a DXC release package into external/dxc/

You will also need to have the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) installed on your system (with VK_SDK_PATH environment variable pointing to the correct folder).

Now you're all set to compile and start the "Tests" project.


