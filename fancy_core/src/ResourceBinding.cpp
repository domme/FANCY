#include "FancyCorePrerequisites.h"

#include "ResourceBinding.h"
#include "RenderContext.h"
#include "ShaderConstantsManager.h"
#include "MaterialPassInstance.h"
#include "Descriptor.h"
#include "Texture.h"

namespace Fancy { namespace Rendering { namespace ResourceBinding {
//---------------------------------------------------------------------------//
  void BindResources_ForwardColorPass(RenderContext* aContext, MaterialPassInstance* aMaterial)
  {
    aContext->SetConstantBuffer(ShaderConstantsManager::GetConstantBuffer(ConstantBufferType::PER_FRAME), 0);
    aContext->SetConstantBuffer(ShaderConstantsManager::GetConstantBuffer(ConstantBufferType::PER_VIEWPORT), 1);
    aContext->SetConstantBuffer(ShaderConstantsManager::GetConstantBuffer(ConstantBufferType::PER_CAMERA), 2);
    aContext->SetConstantBuffer(ShaderConstantsManager::GetConstantBuffer(ConstantBufferType::PER_LIGHT), 3);
    aContext->SetConstantBuffer(ShaderConstantsManager::GetConstantBuffer(ConstantBufferType::PER_MATERIAL), 4);
    aContext->SetConstantBuffer(ShaderConstantsManager::GetConstantBuffer(ConstantBufferType::PER_OBJECT), 5);

    const uint32 kNumTextures = 3u;
    Descriptor textureDescriptors[kNumTextures];

    for (uint32 i = 0u; i < kNumTextures; ++i)
    {
      textureDescriptors[i] = aMaterial->getReadTextures()[i]->GetSrv();
    }

    aContext->SetMultipleResources(textureDescriptors, kNumTextures, (uint32)GpuDescriptorTypeFlags::BUFFER_TEXTURE_CONSTANT_BUFFER, 6);
  }
//---------------------------------------------------------------------------//
} } }  // end of namespace Fancy::Rendering::ResourceBinding