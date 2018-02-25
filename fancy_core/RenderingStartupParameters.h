#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class RenderingApi
  {
    DX12 = 0,
    VULKAN,
  };
//---------------------------------------------------------------------------//
}

namespace Fancy {
//---------------------------------------------------------------------------//  
  enum class RenderingTechnique
  {
    FORWARD = 0,
    FORWARD_PLUS,

    NUM
  };
//---------------------------------------------------------------------------//
  struct RenderingStartupParameters
  {
    RenderingStartupParameters()
      : myRenderingTechnique(RenderingTechnique::FORWARD)
      , myRenderingApi(RenderingApi::DX12)
    { }

    RenderingTechnique myRenderingTechnique;
    RenderingApi myRenderingApi;
  };
//---------------------------------------------------------------------------//
}

