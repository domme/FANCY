#include "fancy_core_precompile.h"
#include "ShaderVk.h"
#include "VkPrerequisites.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderCompiler.h"

namespace Fancy
{
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

    myModule = data.myModule;
    myBindingInfo = data.myBindingInfo;

    // TODO (VK): Create native vertex input layout as in DX12 here? Could also scrap the platform-independent input layout and just create the native input layout in the compiler
  }
//---------------------------------------------------------------------------//
  uint64 ShaderVk::GetNativeBytecodeHash() const
  {
    return reinterpret_cast<uint64>(myModule);
  }
//---------------------------------------------------------------------------//
}


