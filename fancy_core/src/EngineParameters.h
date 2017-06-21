#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  enum class RenderingApi
  {
    DX12 = 0,
    VULKAN,
  };
//---------------------------------------------------------------------------//
} }

namespace Fancy {
//---------------------------------------------------------------------------//  
  enum class RenderingTechnique
  {
    FORWARD = 0,
    FORWARD_PLUS,

    NUM
  };
//---------------------------------------------------------------------------//
  struct DLLEXPORT EngineParameters
  {
    EngineParameters()
      : myResourceFolder("../../../resources/")
      , myRenderingTechnique(RenderingTechnique::FORWARD)
      , myRenderingApi(Rendering::RenderingApi::DX12)
    { }

    String myResourceFolder;
    RenderingTechnique myRenderingTechnique;
    Rendering::RenderingApi myRenderingApi;
  };
//---------------------------------------------------------------------------//
}

