#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  struct ShaderGuidParams
  {
    String myShaderPath;
    uint64 myPermutationHash;
    uint myShaderStage;
  };
//---------------------------------------------------------------------------//
  struct GeometryGuidParams
  {
    float* myVertexData;
    uint myVertexDataCount;
    int* myIndexData;
    uint mIndexDataCount;
  };
//---------------------------------------------------------------------------//
  struct 

//---------------------------------------------------------------------------//
  class GUID
  {
    uint64 Create(const ShaderGuidParams& someShaderParams);
    uint64 Create(const GeometryGuidParams& someGeometryGuidParams);
    
  };
//---------------------------------------------------------------------------//
}