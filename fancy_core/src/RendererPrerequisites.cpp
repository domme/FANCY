#include "RendererPrerequisites.h"

using namespace FANCY::Core::Rendering;
//-----------------------------------------------------------------------//
bool BlendState::operator==( const BlendState& clOther ) const
{
  return u32Hash == clOther.u32Hash;
}
//-----------------------------------------------------------------------//
bool DepthStencilState::operator==( const DepthStencilState& clOther ) const
{
  return u32Hash == clOther.u32Hash;
}
//-----------------------------------------------------------------------//
