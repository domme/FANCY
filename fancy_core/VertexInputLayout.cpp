#include "fancy_core_precompile.h"
#include "VertexInputLayout.h"
#include "DynamicArray.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  ShaderVertexInputLayout ShaderVertexInputLayout::ourDefaultModelLayout = 
  {
    { 
      {"Normal", VertexSemantics::NORMAL, 0u, 12u, DataFormat::RGB_32F },
      {"Position", VertexSemantics::POSITION, 0u, 12u, DataFormat::RGB_32F },
      {"Tangent", VertexSemantics::TANGENT, 0u, 12u, DataFormat::RGB_32F },
      {"Bitangent", VertexSemantics::BITANGENT, 0u, 12u, DataFormat::RGB_32F },
      {"Texcoord", VertexSemantics::TEXCOORD, 0u, 8u, DataFormat::RG_32F } 
    }
  };
//---------------------------------------------------------------------------//
}