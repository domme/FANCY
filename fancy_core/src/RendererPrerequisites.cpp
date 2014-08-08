#include "RendererPrerequisites.h"

using namespace Fancy::Core::Rendering;
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
DepthStencilState::DepthStencilState() :
  u32Hash(0),
  bDepthTestEnabled(true),
  bDepthWriteEnabled(true),
  eDepthCompFunc(CompFunc::LESS),
  bStencilEnabled(true),
  bTwoSidedStencil(false),
  iStencilRef(1),
  uStencilReadMask((uint32)-1)
{
  static uint32 numDepthStencilStates = 0;
  u32Hash = ++numDepthStencilStates;

  eStencilCompFunc[(uint) FaceType::FRONT] = CompFunc::EQUAL;
  eStencilCompFunc[(uint) FaceType::BACK] = CompFunc::EQUAL;

  uStencilWriteMask[(uint) FaceType::FRONT] = (uint32)-1;
  uStencilWriteMask[(uint) FaceType::BACK] = (uint32)-1;

  eStencilFailOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
  eStencilFailOp[(uint) FaceType::BACK]   = StencilOp::KEEP;

  eStencilDepthFailOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
  eStencilDepthFailOp[(uint) FaceType::BACK]   = StencilOp::KEEP;

  eStencilPassOp[(uint) FaceType::FRONT]  = StencilOp::KEEP;
  eStencilPassOp[(uint) FaceType::BACK]   = StencilOp::KEEP;
}
//---------------------------------------------------------------------------//
bool DepthStencilState::operator==( const DepthStencilState& clOther ) const
{
  return u32Hash == clOther.u32Hash;
}
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
BlendState::BlendState() :
  u32Hash(0),
  bAlphaToCoverageEnabled(false),
  bBlendStatePerRT(false)
{
  static uint32 numBlendStatesCreated = 0;
  u32Hash = ++numBlendStatesCreated;

  memset(bAlphaSeparateBlend, sizeof(bAlphaSeparateBlend), false);
  memset(bBlendEnabled, sizeof(bBlendEnabled), false);
  memset(eSrcBlend, sizeof(eSrcBlend), (uint) BlendInput::ONE);
  memset(eDestBlend, sizeof(eDestBlend), (uint) BlendInput::ONE);
  memset(eBlendOp, sizeof(eBlendOp), (uint) BlendOp::ADD);
  memset(eSrcBlendAlpha, sizeof(eSrcBlendAlpha), (uint) BlendInput::ONE);
  memset(eDestBlendAlpha, sizeof(eDestBlendAlpha), (uint) BlendInput::ONE);
  memset(eBlendOpAlpha, sizeof(eBlendOpAlpha), (uint) BlendOp::ADD);
  memset(uRTwriteMask, sizeof(uRTwriteMask), (uint32)-1);
}
//---------------------------------------------------------------------------//
bool BlendState::operator==( const BlendState& clOther ) const
{
  return u32Hash == clOther.u32Hash;
}
//---------------------------------------------------------------------------//

