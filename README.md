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
In order to generate a FANCY solution and build it, all git submodules must be properly initialized and some external prebuilt libraries need to be provided. Fancy uses [vcpgk](https://github.com/microsoft/vcpkg) for most of these libraries.

If you don't already have vcpkg installed and set up (including the global Visual Studio integration), please checkout [vcpkg](https://github.com/microsoft/vcpkg) follow the [setup-steps for windows](https://github.com/microsoft/vcpkg#quick-start-windows).
With a properly set up vcpkg, please execute "get_external_libs.bat" located in the FANCY root folder using cmd or powershell and provide it with the path to the vcpgk root folder as its first argument (e.g. ".\get_external_libs.bat C:/vcpgk/"). This will download and install all required external libraries that are part of vcpkg (e.g. Assimp, EASTL and xxHash).

All other required libraries and binaries that are not part of vcpkg need to be downloaded using the bat-script "get_external_binaries.bat" which downloads and unpacks all other prebuilt binaries that are needed (mainly DirectXShaderCompiler and DX12 Agility SDK). This bat-file can just be executed as-is and doesn't require any arguments. This script will also download and set up [Sharpmake](https://github.com/ubisoft/Sharpmake), which FANCY uses to generate all of its projects and the solution itself. 

You will also need to have the [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) installed on your system (with VK_SDK_PATH environment variable pointing to the correct folder).

Once everything is set up as described, just execute "generate_solution.bat" and you should be ready to go to compile and start the "Tests" project included in the generated solution.