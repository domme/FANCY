#include "fancy_core_precompile.h"
#include "ShaderVk.h"
#include "VkPrerequisites.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderCompiler.h"

namespace Fancy
{
  namespace Priv_ShaderVk
  {
  //---------------------------------------------------------------------------//
    VkShaderStageFlagBits locResolveShaderStage(ShaderStage aShaderStage)
    {
      switch (aShaderStage)
      {
        case ShaderStage::VERTEX: return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderStage::FRAGMENT: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderStage::GEOMETRY: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderStage::TESS_HULL: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderStage::TESS_DOMAIN: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderStage::COMPUTE: return VK_SHADER_STAGE_COMPUTE_BIT;
        default: ASSERT(false, "Missing implementation"); return VK_SHADER_STAGE_VERTEX_BIT;
      }
    }
  //---------------------------------------------------------------------------//
  }
//---------------------------------------------------------------------------//
  ShaderVk::~ShaderVk()
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    vkDestroyShaderModule(platformVk->myDevice, myModule, nullptr);
    myModule = nullptr;
  }
//---------------------------------------------------------------------------//
  void ShaderVk::SetFromCompilerOutput(const ShaderCompilerResult& aCompilerOutput)
  {
    Shader::SetFromCompilerOutput(aCompilerOutput);

    const ShaderCompiledDataVk& data = aCompilerOutput.myNativeData.To<ShaderCompiledDataVk>();

    if (myModule != nullptr)
    {
      RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
      vkDestroyShaderModule(platformVk->myDevice, myModule, nullptr);
    }

    myModule = data.myModule;
    myResourceInfos = data.myResourceInfos;
    myVertexAttributeDesc = data.myVertexAttributeDesc;

    myShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    myShaderStageCreateInfo.pNext = nullptr;
    myShaderStageCreateInfo.flags = 0;
    myShaderStageCreateInfo.pName = myDesc.myMainFunction.c_str();
    myShaderStageCreateInfo.pSpecializationInfo = nullptr;
    myShaderStageCreateInfo.module = myModule;
    myShaderStageCreateInfo.stage = Priv_ShaderVk::locResolveShaderStage((ShaderStage) myDesc.myShaderStage);
  }
//---------------------------------------------------------------------------//
  uint64 ShaderVk::GetNativeBytecodeHash() const
  {
    return reinterpret_cast<uint64>(myModule);
  }
//---------------------------------------------------------------------------//
}


