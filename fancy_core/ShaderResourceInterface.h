#pragma once

#include "ShaderResourceInterfaceDesc.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  class ShaderResourceInterface
  {
  public:
    ShaderResourceInterface() = default;
    virtual ~ShaderResourceInterface() = default;

    bool IsEmpty() const { return myInterfaceDesc.myElements.empty(); }
    const ShaderResourceInterfaceDesc& GetDesc() const { return myInterfaceDesc; }

  protected:
    ShaderResourceInterfaceDesc myInterfaceDesc;
  };
//---------------------------------------------------------------------------//
}