#include "fancy_core_precompile.h"
#include "ShaderCompilerVk.h"
#include "PathService.h"
#include "FileReader.h"
#include "VkPrerequisites.h"
#include "ShaderVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"

#include "spirv_reflect/spirv_reflect.h"

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
    String hlslSrcPathAbs = Priv_ShaderCompilerVk::locGetShaderPath(aDesc.myShaderFileName.c_str(), isGlsl).GetBuffer();
    bool fileFound = false;

    // TODO: Add a glsl->SPIR-V compilation in case there is a glsl shader that should be picked over the HLSL one

    hlslSrcPathAbs = Resources::FindPath(hlslSrcPathAbs, &fileFound);
    if (!fileFound)
      return false;

    const uint64 srcFileWriteTime = Path::GetFileWriteTime(hlslSrcPathAbs);

    const uint64 shaderHash = aDesc.GetHash();
    String spvBinaryFilePathAbs(StaticFilePath("%sShaderCache/%llu.spv", Path::GetUserDataPath().c_str(), shaderHash));
    Path::CreateDirectoryTreeForPath(spvBinaryFilePathAbs);

    // TODO: Add fast-path if the cache-file is newer than the src-file and directly load the SPV binary from that file

    String dxcPath = Path::GetAppPath() + "/../../../dependencies/bin/dxc.exe";
    Path::RemoveFolderUpMarkers(dxcPath);
    
    if (!isGlsl)  // Use dxc.exe to convert from HLSL to SPIR-V binary
    {
      String commandStr = Path::GetAsCmdParameter(dxcPath.c_str()) + " " +
        "-spirv "  // Generate SPIR-V code
        "-fspv-reflect "  // Emit additional SPIR-V instructions to aid reflection
        "-fvk-use-dx-layout "  // Use DirectX memory layout for Vulkan resources
        "-fvk-use-dx-position-w " // Reciprocate SV_Position.w after reading from stage input in PS to accommodate the difference between Vulkan and DirectX
        // "-o0 " // Optimization Level 0  (Seems to be not recognized?!)
        "-Zpc " // Pack matrices in column-major order
        "-Zi " // Enable debug information
        + "-E " + aDesc.myMainFunction + " "
        + "-T " + GetHLSLprofileString(static_cast<ShaderStage>(aDesc.myShaderStage)) + " "
        + "-D " + aStageDefine + " ";

      if (aDesc.myShaderStage == (uint) ShaderStage::VERTEX)
        commandStr += "-fvk-invert-y ";  // Negate SV_Position.y before writing to stage output in VS/DS/GS to accommodate Vulkan's coordinate system

      for (const String& define : aDesc.myDefines)
        commandStr += "-D " + define + " ";

      // Redirect output of command into file
      String errorOut(StaticFilePath("%sShaderCache/%llu_error.txt", Path::GetUserDataPath().c_str(), shaderHash));
      commandStr += "-Fe " + Path::GetAsCmdParameter(errorOut.c_str()) + " ";
      commandStr += Path::GetAsCmdParameter(hlslSrcPathAbs.c_str()) + " -Fo " + Path::GetAsCmdParameter(spvBinaryFilePathAbs.c_str());

      const int returnCode = system(commandStr.c_str());
      if (returnCode != 0)
      {
        String errorOutMsg = FileReader::ReadTextFile(errorOut.c_str());
        LOG_ERROR("Error compiling hlsl shader %s to SPIR-V: %s", hlslSrcPathAbs.c_str(), errorOutMsg.c_str());
        return false;
      }

      DynamicArray<uint8> spvBinaryData;
      const bool spvReadSuccess = FileReader::ReadBinaryFile(spvBinaryFilePathAbs.c_str(), spvBinaryData);
      ASSERT(spvReadSuccess);

      VkShaderModuleCreateInfo moduleCreateInfo = {};
      moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      moduleCreateInfo.codeSize = spvBinaryData.size();
      moduleCreateInfo.pCode = reinterpret_cast<uint32_t*>(spvBinaryData.data());

      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();

      ShaderCompiledDataVk nativeData;
      ASSERT_VK_RESULT(vkCreateShaderModule(platformVk->myDevice, &moduleCreateInfo, nullptr, &nativeData.myModule));
      aCompilerOutput->myNativeData = nativeData;

      
      // Reflect the spirv data

      SpvReflectShaderModule reflectModule;
      SpvReflectResult reflectResult = spvReflectCreateShaderModule(spvBinaryData.size(), spvBinaryData.data(), &reflectModule);
      ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS);

      // Reflect the RootSignature

      



    }

    return true;
  }
//---------------------------------------------------------------------------//
}

