#include "fancy_core_precompile.h"
#include "ShaderCompilerVk.h"
#include "PathService.h"
#include "FileReader.h"

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
    String hlslSrcPathAbs = srcFilePath.GetBuffer();

    hlslSrcPathAbs = Resources::FindPath(hlslSrcPathAbs, &fileFound);
    if (!fileFound)
      return false;

    Path::AddQuotesAroundSpaceSegments(hlslSrcPathAbs);

    const uint64 srcFileWriteTime = Path::GetFileWriteTime(hlslSrcPathAbs);

    const uint64 shaderHash = aDesc.GetHash();
    String spvBinaryFilePathAbs(StaticFilePath("%sShaderCache/%llu.spv", Path::GetUserDataPath().c_str(), shaderHash));
    String spvBinaryFilePathAbs_NoQuotes(spvBinaryFilePathAbs);
    Path::CreateDirectoryTreeForPath(spvBinaryFilePathAbs);
    Path::AddQuotesAroundSpaceSegments(spvBinaryFilePathAbs);
    
    // TODO: Add fast-path if the cache-file is newer than the src-file and directly load the SPV binary from that file

    String dxcPath = Path::GetAppPath() + "/../../../dependencies/bin/dxc.exe";
    Path::RemoveFolderUpMarkers(dxcPath);
    Path::AddQuotesAroundSpaceSegments(dxcPath);
    
    if (!isGlsl)  // Use dxc.exe to convert from HLSL to SPIR-V binary
    {
      StaticString<4096> commandStr(
        "%s "
        "-spirv "  // Generate SPIR-V code
        "-fspv-reflect "  // Emit additional SPIR-V instructions to aid reflection
        "-fvk-invert-y "  // Negate SV_Position.y before writing to stage output in VS/DS/GS to accommodate Vulkan's coordinate system
        "-fvk-use-dx-layout "  // Use DirectX memory layout for Vulkan resources
        "-fvk-use-dx-position-w " // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
        // "-o0 " // Optimization Level 0  (Seems to be not recognized?!)
        "-Zpc " // Pack matrices in column-major order
        "-Zi " // Enable debug information
        "-E %s " // Entry point name
        "-T %s ", // Target HLSL profile
        dxcPath.c_str(),
        aDesc.myMainFunction.c_str(),
        GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage))
      );

      commandStr.Append("-D %s ", aStageDefine);

      for (const String& define : aDesc.myDefines)
        commandStr.Append("-D %s ", define.c_str());

      commandStr.Append("%s -Fo %s", hlslSrcPathAbs.c_str(), spvBinaryFilePathAbs.c_str());

      const int returnCode = system(commandStr.GetBuffer());
      if (returnCode != 0)
      {
        LOG_ERROR("Error converting hlsl shader %s to SPIR-V.", hlslSrcPathAbs.c_str());
        return false;
      }

      DynamicArray<uint8> spvBinaryData;
      const bool spvReadSuccess = FileReader::ReadBinaryFile(spvBinaryFilePathAbs_NoQuotes.c_str(), spvBinaryData);
      ASSERT(spvReadSuccess);




        


    }

    return true;
  }
//---------------------------------------------------------------------------//
}

