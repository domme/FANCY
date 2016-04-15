#pragma once

#include "RendererPrerequisites.h"
#include PLATFORM_DEPENDENT_INCLUDE_SHADERRESOURCEINTERFACE

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class ShaderResourceInterface : public PLATFORM_DEPENDENT_NAME(ShaderResourceInterface)
  {
    
  };
//---------------------------------------------------------------------------//


  class ShaderResourceInterfacePoolDX12
  {
  public:
    
    static void Init();
    static void Destroy();

  protected:
    static std::vector<ShaderResourceInterfaceDX12*> myRootSignaturePool;
  };
} }  // Fancy::Rendering