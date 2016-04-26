#include "FancyCorePrerequisites.h"
#include "RendererPrerequisites.h"
#include "ShaderResourceInterfaceDX12.h"

#if defined(RENDERER_DX12)

#include "ShaderResourceInterface.h"
#include "MathUtil.h"

namespace Fancy { namespace Rendering { namespace DX12 {
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  
//---------------------------------------------------------------------------//
  void ShaderResourceInterfacePoolDX12::Init()
  {
  
  }
//---------------------------------------------------------------------------//
  void ShaderResourceInterfacePoolDX12::Destroy()
  {
    for (ShaderResourceInterfaceDX12* rs : myRootSignaturePool)
      FANCY_DELETE(rs, MemoryCategory::MATERIALS);

    myRootSignaturePool.clear();
  }
//---------------------------------------------------------------------------//
}}}

#endif // RENDERER_DX12