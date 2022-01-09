set vcpkg_folder=%1
%vcpkg_folder%\vcpkg.exe install eastl:x64-windows
%vcpkg_folder%\vcpkg.exe install assimp:x64-windows
%vcpkg_folder%\vcpkg.exe install xxhash:x64-windows