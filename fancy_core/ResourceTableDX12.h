#pragma once

#include "DX12Prerequisites.h"
#include "ResourceTable.h"

#if FANCY_ENABLE_DX12

namespace Fancy
{
  class ResourceTableDX12 : public ResourceTable
  {
  public:
    ResourceTableDX12();
    ~ResourceTableDX12();

  protected:
    
  };
}

#endif