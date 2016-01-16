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
  struct GpuProgramPipelineGuidParams
  {
    Rendering::GpuProgram* myGpuPrograms;
    uint myNumGpuPrograms;
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
  class GUID
  {
    uint64 Create(const ShaderGuidParams& someShaderParams);
    uint64 Create(const GeometryGuidParams& someGeometryGuidParams);
    uint64 Create(const GpuProgramPipelineGuidParams& someProgramPipelineGuidParams);
    uint64 Create(const Rendering::MaterialPassProperties& someMaterialPassProperties);


    
    
  };
//---------------------------------------------------------------------------//
}