#include "fancy_core_precompile.h"
#include "ShaderPipelineDX12.h"
#include "ShaderDX12.h"

#if FANCY_ENABLE_DX12

void Fancy::ShaderPipelineDX12::CreateFromShaders()
{
  // On DX12 all shader-stages share the same root-signature so we can just pick it from the first available shader
  for (const SharedPtr<Shader>& shader : myShaders)
  {
    if (shader)
    {
      const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(shader.get());
      myRootSignature = shaderDx12->GetRootSignature();
      myRootSignatureLayout = shaderDx12->GetRootSignatureLayout();
      break;
    }
  }

  const bool hasComputeShader = myShaders[(uint)ShaderStage::COMPUTE] != nullptr;

  // Create a list of all resource infos used by all shader stages
  myResourceInfos.clear();
  for (uint i = 0u; i < (uint)ShaderStage::NUM; ++i)
  {
    Shader* shader = myShaders[i].get();
    if (shader == nullptr)
      continue;

    ASSERT(!hasComputeShader || i == (uint)ShaderStage::COMPUTE, "Can't mix a compute shader with other stages in the same pipeline");

    const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(shader);
    const eastl::vector<ShaderResourceInfoDX12>& resInfos = shaderDx12->GetResourceInfos();

    if (myResourceInfos.empty())
    {
      myResourceInfos.insert(myResourceInfos.begin(), resInfos.begin(), resInfos.end());
      continue;
    }

    for (const ShaderResourceInfoDX12& info : resInfos)
      if (std::find(myResourceInfos.begin(), myResourceInfos.end(), info) == myResourceInfos.end())
        myResourceInfos.push_back(info);
  }
}

#endif