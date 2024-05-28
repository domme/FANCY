set vcpkg_folder=.\external\vcpkg
%vcpkg_folder%\vcpkg.exe install eastl:x64-windows
%vcpkg_folder%\vcpkg.exe install assimp:x64-windows
%vcpkg_folder%\vcpkg.exe install xxhash:x64-windows
%vcpkg_folder%\vcpkg.exe install glm:x64-windows
%vcpkg_folder%\vcpkg.exe install directx-dxc:x64-windows
%vcpkg_folder%\vcpkg.exe install directx12-agility:x64-windows
PAUSE