#include "fancy_core_precompile.h"
#include "ShaderPipelineDX12.h"

void Fancy::ShaderPipelineDX12::UpdateResourceInterface()
{
  // On DX12 all shader-stages share the same root-signature so we can just pick it from the first available shader
  for (SharedPtr<Shader>& shader : myGpuPrograms)
  {
    if (shader)
    {
      myResourceInterface = shader->myProperties.myResourceInterface;
      myRootSignature = myResourceInterface.myNativeData.To<Microsoft::WRL::ComPtr<ID3D12RootSignature>>();
      break;
    }
  }
}
