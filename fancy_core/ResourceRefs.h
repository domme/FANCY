#pragma once

namespace Fancy {
//---------------------------------------------------------------------------//
  enum class TextureRef
  {
    DEFAULT_DIFFUSE = 0,
    DEFAULT_NORMAL,
    DEFAULT_SPECULAR,

    DEFAULT_BACKBUFFER,
    DEFAULT_DEPTHSTENCILBUFFER,
    
    NUM
  };
//---------------------------------------------------------------------------//
  enum class BufferRef
  {
    NUM
  };
//---------------------------------------------------------------------------//
  enum class MeshRef
  {
    UNIT_CUBE = 0,
    UNIT_SPHERE,
    COORD_GRID,

    NUM
  };
//---------------------------------------------------------------------------//
  namespace ResourceRef
  {
     ToString(TextureRef aRef);
     ToString(BufferRef aRef);
     ToString(MeshRef aRef);
  } 

}
