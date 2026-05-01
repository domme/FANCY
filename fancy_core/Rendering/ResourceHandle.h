#pragma once

#include "Common/FancyCoreDefines.h"

namespace Fancy {
  class Texture;
  class GpuBuffer;
  class TextureView;
  class GpuBufferView;
  class Shader;
  class ShaderPipeline;
  class BlendState;
  class DepthStencilState;
  class TextureSampler;
  struct VertexInputLayout;
  class RtAccelerationStructure;
  class RtPipelineState;
  class RtShaderBindingTable;
  class RenderOutput;

  template < typename T > struct ResourceHandle {
    static constexpr uint kInvalidIndex = UINT32_MAX;
    uint                  myIndex = kInvalidIndex;
    uint                  myGeneration = 0u;

    bool IsValid() const {
      return myIndex != kInvalidIndex;
    }
    explicit operator bool() const {
      return IsValid();
    }
    bool operator==( const ResourceHandle & o ) const {
      return myIndex == o.myIndex && myGeneration == o.myGeneration;
    }
    bool operator!=( const ResourceHandle & o ) const {
      return !( *this == o );
    }
  };

  using TextureHandle = ResourceHandle< Texture >;
  using GpuBufferHandle = ResourceHandle< GpuBuffer >;
  using TextureViewHandle = ResourceHandle< TextureView >;
  using GpuBufferViewHandle = ResourceHandle< GpuBufferView >;
  using ShaderHandle = ResourceHandle< Shader >;
  using ShaderPipelineHandle = ResourceHandle< ShaderPipeline >;
  using BlendStateHandle = ResourceHandle< BlendState >;
  using DepthStencilStateHandle = ResourceHandle< DepthStencilState >;
  using TextureSamplerHandle = ResourceHandle< TextureSampler >;
  using VertexInputLayoutHandle = ResourceHandle< VertexInputLayout >;
  using RtAccelerationStructureHandle = ResourceHandle< RtAccelerationStructure >;
  using RtPipelineStateHandle = ResourceHandle< RtPipelineState >;
  using RtShaderBindingTableHandle = ResourceHandle< RtShaderBindingTable >;
  using RenderOutputHandle = ResourceHandle< RenderOutput >;
}  // namespace Fancy
