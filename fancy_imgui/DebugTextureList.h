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

    char myFilterText[256] = {};
    UniquePtr<ImGuiMippedDebugImage> mySelectedDebugImage;
  };
}



