#pragma once

#include "FancyCorePrerequisites.h"

namespace Fancy { namespace Rendering {
  struct GpuProgramDesc;
  struct MaterialPassDesc;
  struct GpuProgramPipelineDesc;

} }


namespace Fancy {


  
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