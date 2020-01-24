#include "fancy_core_precompile.h"
#include "ShaderPipelineDX12.h"
#include "ShaderDX12.h"

void Fancy::ShaderPipelineDX12::UpdateResourceInterface()
{
  // On DX12 all shader-stages share the same root-signature so we can just pick it from the first available shader
  for (const SharedPtr<Shader>& shader : myShaders)
  {
    if (shader)
    {
      const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(shader.get());
      myRootSignature = shaderDx12->GetRootSignature();
      break;
    }
  }

  // Create a list of all resource infos used by all shader stages
  myResourceInfos.clear();
  for (const SharedPtr<Shader>& shader : myShaders)
  {
    if (shader == nullptr)
      continue;

    const ShaderDX12* shaderDx12 = static_cast<const ShaderDX12*>(shader.get());
    const DynamicArray<ShaderResourceInfoDX12>& resInfos = shaderDx12->GetResourceInfos();

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
