#include "fancy_core_precompile.h"
#include "ShaderCompilerVk.h"
#include "PathService.h"

namespace Fancy
{
  namespace Priv_ShaderCompilerVk
  {
    StaticFilePath locGetShaderPath(const char* aFilename, bool& isGlsl)
    {
      StaticFilePath path("shader/Vk/%s.glsl", aFilename);

      String pathRel = path.GetBuffer();
      String pathAbs = Resources::FindPath(pathRel);

      if (Path::FileExists(pathAbs.c_str()))
      {
        isGlsl = true;
        return path;
      }

      isGlsl = false;
      path.Format("shader/DX12/%s.hlsl", aFilename);
      return path;
    }
  }

//---------------------------------------------------------------------------//
  ShaderCompilerVk::ShaderCompilerVk()
  {
  }
//---------------------------------------------------------------------------//
  ShaderCompilerVk::~ShaderCompilerVk()
  {
  }
//---------------------------------------------------------------------------//
  String ShaderCompilerVk::GetShaderPath(const String& aFilename) const
  {
    bool isGlsl = false;
    return Priv_ShaderCompilerVk::locGetShaderPath(aFilename.c_str(), isGlsl).GetBuffer();
  }
//---------------------------------------------------------------------------//
  bool ShaderCompilerVk::Compile_Internal(const ShaderDesc& aDesc, const char* aStageDefine, ShaderCompilerResult* aCompilerOutput) const
  {
    // The Vulkan shader compiler should work like that:
    // - Translate default HLSL shader into a SPIR-V binary
    // - If there is a .../Vulkan/.glsl-file for that shader as well (or only that) prefer that one over the HLSL file. That way, HLSL-translation problems can be worked around by providing a glsl shader
    // - In the future: Use caches to skip source-file compilation!

    bool isGlsl = false;
    StaticFilePath srcFilePath = Priv_ShaderCompilerVk::locGetShaderPath(aDesc.myShaderFileName.c_str(), isGlsl);
    bool fileFound = false;
    String srcFilePathAbs = srcFilePath.GetBuffer();

    srcFilePathAbs = Resources::FindPath(srcFilePathAbs, &fileFound);
    if (!fileFound)
      return false;

    const uint64 srcFileWriteTime = Path::GetFileWriteTime(srcFilePathAbs);

    const uint64 shaderHash = aDesc.GetHash();
    StaticFilePath dstFilePathAbs("%sShaderCache/%llu.spv", Path::GetUserDataPath().c_str(), shaderHash);
    StaticFilePath dstFileErrorPathAbs("%sShaderCache/%llu_error.txt", Path::GetUserDataPath().c_str(), shaderHash);

    // TODO: Add fast-path if the cache-file is newer than the src-file and directly load the SPV binary from that file
    
    if (!isGlsl)  // Use dxc.exe to convert from HLSL to SPIR-V binary
    {
      StaticString<4096> commandStr(
        "../../../dependencies/bin/dxc.exe "
        "-spirv "  // Generate SPIR-V code
        "-fspv-reflect "  // Emit additional SPIR-V instructions to aid reflection
        "-fvk-invert-y "  // Negate SV_Position.y before writing to stage output in VS/DS/GS to accommodate Vulkan's coordinate system
        "-fvk-use-dx-layout "  // Use DirectX memory layout for Vulkan resources
        "-fvk-use-dx-position-w " // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
        "-o0 " // Optimization Level 0
        "-Zpc " // Pack matrices in column-major order
        "-Zi " // Enable debug information
        "-E %s " // Entry point name
        "-T %s ", // Target HLSL profile
        aDesc.myMainFunction.c_str(),
        GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage))
      );

      commandStr.Append("-D %s ", aStageDefine);

      for (const String& define : aDesc.myDefines)
        commandStr.Append("-D %s ", define.c_str());

      commandStr.Append("%s -Fo %s -Fe %s", srcFilePathAbs.c_str(), dstFilePathAbs.GetBuffer(), dstFileErrorPathAbs.GetBuffer());

      const int returnCode = system(commandStr.GetBuffer());
    }

    return true;
  }
//---------------------------------------------------------------------------//
}

