#pragma once
#include <EASTL/string.h>

#include "DebugImage.h"

namespace Fancy
{
  class DebugTextureList
  {
  public:
    void Update();

  private:
    eastl::string mySelectedItem;
    bool myIsOpen = false;

    UniquePtr<ImGuiMippedDebugImage> mySelectedDebugImage;
  };
}



