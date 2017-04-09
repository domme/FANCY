#pragma once

#include "RendererPrerequisites.h"
#include "ShaderResourceInterfaceDesc.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  class ShaderResourceInterface
  {
  public:
    ShaderResourceInterface() = default;
    virtual ~ShaderResourceInterface() = default;

    bool IsEmpty() const { return myInterfaceDesc.myElements.empty(); }

    ShaderResourceInterfaceDesc myInterfaceDesc;
  };
//---------------------------------------------------------------------------//
} }  // Fancy::Rendering