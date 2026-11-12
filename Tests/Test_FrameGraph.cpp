#include "Test_FrameGraph.h"

#include "imgui.h"
#include "Common/Window.h"
#include "Rendering/CommandList.h"
#include "Rendering/DepthStencilState.h"
#include "Rendering/FrameGraph/FrameGraphTaskHelper.h"
#include "Rendering/GpuBuffer.h"
#include "Rendering/RenderCore.h"
#include "Rendering/RenderOutput.h"
#include "Rendering/ShaderPipeline.h"
#include "Rendering/Texture.h"
#include "Rendering/TextureSampler.h"

using namespace Fancy;

namespace Priv_Test_FrameGraph {
  const char * kQuadrantTextureName = "fgQuadrants";
  const char * kColorBufferName = "fgQuadrantColors";
  const uint kQuadrantTextureSize = 512u;
  const uint kNumColors = 4u;
  const char * kMipChainTextureName = "fgMipChain";
  const char * kDepthStencilTextureName = "fgDepthStencil";
  const char * kStencilCopyTextureName = "fgStencilCopy";
  const uint kSubresourceTextureSize = 256u;
  const uint kNumMips = 4u;
  const uint kCheckerCellSize = 32u;
  const uint kNumStencilRects = 3u;
  const uint kStencilRefs[ kNumStencilRects ] = { 64u, 128u, 255u };
  const glm::uvec4 kStencilRects[ kNumStencilRects ] = { glm::uvec4( 16u, 16u, 160u, 160u ), glm::uvec4( 64u, 64u, 208u, 208u ),
                                                         glm::uvec4( 112u, 112u, 240u, 240u ) };  // left, top, right, bottom
  const char * kShaderPath = "fancy/resources/shaders/Tests/FrameGraph.hlsl";
  //---------------------------------------------------------------------------//
  // Matches CB0 in FrameGraph.hlsl
  struct ShaderConsts {
    glm::float4 myColors[ kNumColors ];
    glm::uvec2 myOffset;
    uint myTextureIndex;
    uint mySamplerIndex;
    glm::float2 myPixelToUv;
    uint myBufferIndex;
    uint myQuadrantIndex;
    glm::uvec2 myDstSize;
    uint mySrcIndex;
    uint myDstIndex;
    uint myDisplayMode;
    uint myMipLevel;
    glm::uvec2 myDisplayOrigin;
    float myDisplayScale;
  };
  enum DisplayMode { kDisplayTexture = 0, kDisplayMip = 1, kDisplayStencil = 2, kDisplayDepth = 3 };
  //---------------------------------------------------------------------------//
  const glm::float2 kFullscreenTriangle[] = { { -4.0f, -1.0f }, { 1.0f, -1.0f }, { 1.0f, 4.0f } };
  //---------------------------------------------------------------------------//
  class StoreColorsTask {
  public:
    struct RenderData {
      GpuBufferView * myColorBuffer = nullptr;
    };

    static StoreColorsTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aColorBuffer, const glm::float4 * someColors,
                                    const ShaderPipeline * aPipeline ) {
      StoreColorsTask * task = nullptr;
      const uint taskIndex = FgAddTask< StoreColorsTask >( aBuilder, "Store quadrant colors", CommandListType::Compute, &task );
      task->myColorBuffer = aColorBuffer;
      aBuilder.Write( taskIndex, task->myColorBuffer, RESOURCE_ACCESS_COMPUTE_SHADER_WRITE );
      for ( uint i = 0u; i < kNumColors; ++i ) {
        task->myColors[ i ] = someColors[ i ];
      }
      task->myPipeline = aPipeline;
      return *task;
    }

    static void Compile( const StoreColorsTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.myColorBuffer = aContext.GetBufferUav( aTask.myColorBuffer );
    }

    static void Execute( CommandList * aCommandList, const StoreColorsTask & aTask, const RenderData & someRenderData ) {
      ShaderConsts consts = {};
      for ( uint i = 0u; i < kNumColors; ++i ) {
        consts.myColors[ i ] = aTask.myColors[ i ];
      }
      consts.myBufferIndex = someRenderData.myColorBuffer->GetGlobalDescriptorIndex();

      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->Dispatch( glm::int3( ( int ) kNumColors, 1, 1 ) );
    }

    FgResourceHandle myColorBuffer;
    glm::float4 myColors[ kNumColors ];
    const ShaderPipeline * myPipeline = nullptr;
  };
  //---------------------------------------------------------------------------//
  class FillQuadrantComputeTask {
  public:
    struct RenderData {
      TextureView * myTarget = nullptr;
      GpuBufferView * myColorBuffer = nullptr;
    };

    static FillQuadrantComputeTask & Setup( FrameGraphBuilder & aBuilder, const char * aName, FgResourceHandle aTarget, FgResourceHandle aColorBuffer,
                                            uint aQuadrantIndex, const glm::uvec4 & aRect, const ShaderPipeline * aPipeline ) {
      FillQuadrantComputeTask * task = nullptr;
      const uint taskIndex = FgAddTask< FillQuadrantComputeTask >( aBuilder, aName, CommandListType::Compute, &task );
      task->myTarget = aTarget;
      aBuilder.Write( taskIndex, task->myTarget, RESOURCE_ACCESS_COMPUTE_SHADER_WRITE );
      task->myColorBuffer = aColorBuffer;
      aBuilder.Read( taskIndex, task->myColorBuffer, RESOURCE_ACCESS_COMPUTE_SHADER_READ );
      task->myRect = aRect;
      task->myQuadrantIndex = aQuadrantIndex;
      task->myPipeline = aPipeline;
      return *task;
    }

    static void Compile( const FillQuadrantComputeTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.myTarget = aContext.GetUav( aTask.myTarget );
      someRenderData.myColorBuffer = aContext.GetBufferSrv( aTask.myColorBuffer );
    }

    static void Execute( CommandList * aCommandList, const FillQuadrantComputeTask & aTask, const RenderData & someRenderData ) {
      ShaderConsts consts = {};
      consts.myOffset = glm::uvec2( aTask.myRect.x, aTask.myRect.y );
      consts.myTextureIndex = someRenderData.myTarget->GetGlobalDescriptorIndex();
      consts.myBufferIndex = someRenderData.myColorBuffer->GetGlobalDescriptorIndex();
      consts.myQuadrantIndex = aTask.myQuadrantIndex;

      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->Dispatch( glm::int3( ( int ) aTask.myRect.z, ( int ) aTask.myRect.w, 1 ) );
    }

    FgResourceHandle myTarget;
    FgResourceHandle myColorBuffer;
    glm::uvec4 myRect;  // x, y, width, height
    uint myQuadrantIndex = 0u;
    const ShaderPipeline * myPipeline = nullptr;
  };
  //---------------------------------------------------------------------------//
  class FillQuadrantRasterTask {
  public:
    struct RenderData {
      TextureView * myTarget = nullptr;
      GpuBufferView * myColorBuffer = nullptr;
    };

    static FillQuadrantRasterTask & Setup( FrameGraphBuilder & aBuilder, const char * aName, FgResourceHandle aTarget, FgResourceHandle aColorBuffer,
                                           uint aQuadrantIndex, const glm::uvec4 & aRect, const ShaderPipeline * aPipeline,
                                           DepthStencilState * aDepthStencilState ) {
      FillQuadrantRasterTask * task = nullptr;
      const uint taskIndex = FgAddTask< FillQuadrantRasterTask >( aBuilder, aName, CommandListType::Graphics, &task );
      task->myTarget = aTarget;
      aBuilder.Write( taskIndex, task->myTarget, RESOURCE_ACCESS_RENDER_TARGET );
      task->myColorBuffer = aColorBuffer;
      aBuilder.Read( taskIndex, task->myColorBuffer, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task->myRect = aRect;
      task->myQuadrantIndex = aQuadrantIndex;
      task->myPipeline = aPipeline;
      task->myDepthStencilState = aDepthStencilState;
      return *task;
    }

    static void Compile( const FillQuadrantRasterTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.myTarget = aContext.GetRtv( aTask.myTarget );
      someRenderData.myColorBuffer = aContext.GetBufferSrv( aTask.myColorBuffer );
    }

    static void Execute( CommandList * aCommandList, const FillQuadrantRasterTask & aTask, const RenderData & someRenderData ) {
      ShaderConsts consts = {};
      consts.myBufferIndex = someRenderData.myColorBuffer->GetGlobalDescriptorIndex();
      consts.myQuadrantIndex = aTask.myQuadrantIndex;

      const glm::uvec4 & rect = aTask.myRect;
      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->SetDepthStencilState( aTask.myDepthStencilState );
      aCommandList->SetCullMode( CullMode::NONE );
      aCommandList->SetViewport( rect );
      aCommandList->SetClipRect( glm::uvec4( rect.x, rect.y, rect.x + rect.z, rect.y + rect.w ) );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->BindVertexBuffer( ( void * ) kFullscreenTriangle, sizeof( kFullscreenTriangle ) );
      aCommandList->SetRenderTarget( someRenderData.myTarget, nullptr );
      aCommandList->DrawInstanced( 3, 1, 0, 0 );
    }

    FgResourceHandle myTarget;
    FgResourceHandle myColorBuffer;
    glm::uvec4 myRect;  // x, y, width, height
    uint myQuadrantIndex = 0u;
    const ShaderPipeline * myPipeline = nullptr;
    DepthStencilState * myDepthStencilState = nullptr;
  };
  //---------------------------------------------------------------------------//
  class DownsampleMipTask {
  public:
    struct RenderData {
      TextureView * mySrc = nullptr;
      TextureView * myDst = nullptr;
    };

    static DownsampleMipTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aMipChain, uint aMip, const ShaderPipeline * aPipeline ) {
      eastl::string name( eastl::string::CtorSprintf(), "Downsample mip %d", aMip );
      DownsampleMipTask * task = nullptr;
      const uint taskIndex = FgAddTask< DownsampleMipTask >( aBuilder, name.c_str(), CommandListType::Compute, &task );
      task->mySrc = aMipChain.Mips( aMip - 1u );
      aBuilder.Read( taskIndex, task->mySrc, RESOURCE_ACCESS_COMPUTE_SHADER_READ );
      task->myDst = aMipChain.Mips( aMip );
      aBuilder.Write( taskIndex, task->myDst, RESOURCE_ACCESS_COMPUTE_SHADER_WRITE );
      task->myDstSize = kSubresourceTextureSize >> aMip;
      task->myPipeline = aPipeline;
      return *task;
    }

    static void Compile( const DownsampleMipTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.mySrc = aContext.GetSrv( aTask.mySrc );
      someRenderData.myDst = aContext.GetUav( aTask.myDst );
    }

    static void Execute( CommandList * aCommandList, const DownsampleMipTask & aTask, const RenderData & someRenderData ) {
      ShaderConsts consts = {};
      consts.myDstSize = glm::uvec2( aTask.myDstSize );
      consts.mySrcIndex = someRenderData.mySrc->GetGlobalDescriptorIndex();
      consts.myDstIndex = someRenderData.myDst->GetGlobalDescriptorIndex();

      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->Dispatch( glm::int3( ( int ) aTask.myDstSize, ( int ) aTask.myDstSize, 1 ) );
    }

    FgResourceHandle mySrc;
    FgResourceHandle myDst;
    uint myDstSize = 0u;
    const ShaderPipeline * myPipeline = nullptr;
  };
  //---------------------------------------------------------------------------//
  class DrawStencilPatternTask {
  public:
    struct RenderData {
      TextureView * myDepthStencil = nullptr;
    };

    static DrawStencilPatternTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aDepthStencil, const ShaderPipeline * aPipeline,
                                           DepthStencilState ** someStencilStates ) {
      DrawStencilPatternTask * task = nullptr;
      const uint taskIndex = FgAddTask< DrawStencilPatternTask >( aBuilder, "Draw stencil pattern", CommandListType::Graphics, &task );
      task->myDepthStencil = aDepthStencil;
      aBuilder.Write( taskIndex, task->myDepthStencil, RESOURCE_ACCESS_DEPTH_WRITE );
      task->myPipeline = aPipeline;
      for ( uint i = 0u; i < kNumStencilRects; ++i ) {
        task->myStencilStates[ i ] = someStencilStates[ i ];
      }
      return *task;
    }

    static void Compile( const DrawStencilPatternTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.myDepthStencil = aContext.GetDsv( aTask.myDepthStencil );
    }

    static void Execute( CommandList * aCommandList, const DrawStencilPatternTask & aTask, const RenderData & someRenderData ) {
      aCommandList->SetRenderTargets( nullptr, 0u, someRenderData.myDepthStencil );
      aCommandList->ClearDepthStencilTarget( someRenderData.myDepthStencil, 1.0f, 0,
                                             ( uint ) DepthStencilClearFlags::CLEAR_DEPTH | ( uint ) DepthStencilClearFlags::CLEAR_STENCIL );

      ShaderConsts consts = {};
      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->SetCullMode( CullMode::NONE );
      aCommandList->SetViewport( glm::uvec4( 0u, 0u, kSubresourceTextureSize, kSubresourceTextureSize ) );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->BindVertexBuffer( ( void * ) kFullscreenTriangle, sizeof( kFullscreenTriangle ) );
      for ( uint i = 0u; i < kNumStencilRects; ++i ) {
        aCommandList->SetDepthStencilState( aTask.myStencilStates[ i ] );
        aCommandList->SetClipRect( kStencilRects[ i ] );
        aCommandList->DrawInstanced( 3, 1, 0, 0 );
      }
    }

    FgResourceHandle myDepthStencil;
    const ShaderPipeline * myPipeline = nullptr;
    DepthStencilState * myStencilStates[ kNumStencilRects ] = {};
  };
  //---------------------------------------------------------------------------//
  class CopyStencilTask {
  public:
    struct RenderData {
      TextureView * mySrc = nullptr;
      TextureView * myDst = nullptr;
    };

    static CopyStencilTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aDepthStencil, FgResourceHandle aStencilCopy,
                                    const ShaderPipeline * aPipeline ) {
      CopyStencilTask * task = nullptr;
      const uint taskIndex = FgAddTask< CopyStencilTask >( aBuilder, "Copy stencil", CommandListType::Compute, &task );
      task->mySrc = aDepthStencil.Planes( 1u );
      aBuilder.Read( taskIndex, task->mySrc, RESOURCE_ACCESS_COMPUTE_SHADER_READ );
      task->myDst = aStencilCopy;
      aBuilder.Write( taskIndex, task->myDst, RESOURCE_ACCESS_COMPUTE_SHADER_WRITE );
      task->myPipeline = aPipeline;
      return *task;
    }

    static void Compile( const CopyStencilTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.mySrc = aContext.GetSrv( aTask.mySrc );
      someRenderData.myDst = aContext.GetUav( aTask.myDst );
    }

    static void Execute( CommandList * aCommandList, const CopyStencilTask & aTask, const RenderData & someRenderData ) {
      ShaderConsts consts = {};
      consts.myDstSize = glm::uvec2( kSubresourceTextureSize );
      consts.mySrcIndex = someRenderData.mySrc->GetGlobalDescriptorIndex();
      consts.myDstIndex = someRenderData.myDst->GetGlobalDescriptorIndex();

      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->Dispatch( glm::int3( ( int ) kSubresourceTextureSize, ( int ) kSubresourceTextureSize, 1 ) );
    }

    FgResourceHandle mySrc;
    FgResourceHandle myDst;
    const ShaderPipeline * myPipeline = nullptr;
  };
  //---------------------------------------------------------------------------//
  class BlitToBackBufferTask {
  public:
    struct RenderData {
      TextureView * mySource = nullptr;
      TextureView * myMipChain = nullptr;
      TextureView * myStencilCopy = nullptr;
      TextureView * myDepth = nullptr;
      TextureView * myBackBuffer = nullptr;
    };

    static BlitToBackBufferTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aSource, FgResourceHandle aMipChain,
                                         FgResourceHandle aStencilCopy, FgResourceHandle aDepthStencil, FgResourceHandle aBackBuffer,
                                         const glm::uvec2 & aBackBufferSize, const ShaderPipeline * aPipeline,
                                         DepthStencilState * aDepthStencilState ) {
      BlitToBackBufferTask * task = nullptr;
      const uint taskIndex = FgAddTask< BlitToBackBufferTask >( aBuilder, "Blit to backbuffer", CommandListType::Graphics, &task );
      task->mySource = aSource;
      aBuilder.Read( taskIndex, task->mySource, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task->myMipChain = aMipChain;
      aBuilder.Read( taskIndex, task->myMipChain, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task->myStencilCopy = aStencilCopy;
      aBuilder.Read( taskIndex, task->myStencilCopy, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task->myDepth = aDepthStencil.Planes( 0u );
      aBuilder.Read( taskIndex, task->myDepth, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task->myBackBuffer = aBackBuffer;
      aBuilder.Write( taskIndex, task->myBackBuffer, RESOURCE_ACCESS_RENDER_TARGET );
      task->myBackBufferSize = aBackBufferSize;
      task->myPipeline = aPipeline;
      task->myDepthStencilState = aDepthStencilState;
      return *task;
    }

    static void Compile( const BlitToBackBufferTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.mySource = aContext.GetSrv( aTask.mySource );
      someRenderData.myMipChain = aContext.GetSrv( aTask.myMipChain );
      someRenderData.myStencilCopy = aContext.GetSrv( aTask.myStencilCopy );
      someRenderData.myDepth = aContext.GetSrv( aTask.myDepth );
      someRenderData.myBackBuffer = aContext.GetRtv( aTask.myBackBuffer );
    }

    static void DrawRect( CommandList * aCommandList, const ShaderConsts & someConsts, const glm::uvec4 & aRect ) {
      ShaderConsts consts = someConsts;
      consts.myDisplayOrigin = glm::uvec2( aRect.x, aRect.y );
      aCommandList->SetViewport( aRect );
      aCommandList->SetClipRect( glm::uvec4( aRect.x, aRect.y, aRect.x + aRect.z, aRect.y + aRect.w ) );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->DrawInstanced( 3, 1, 0, 0 );
    }

    static void Execute( CommandList * aCommandList, const BlitToBackBufferTask & aTask, const RenderData & someRenderData ) {
      const uint width = aTask.myBackBufferSize.x;
      const uint height = aTask.myBackBufferSize.y;
      const uint halfWidth = width / 2u;
      const uint cell = glm::min( width / 8u, height / 3u );

      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->SetDepthStencilState( aTask.myDepthStencilState );
      aCommandList->SetCullMode( CullMode::NONE );
      aCommandList->BindVertexBuffer( ( void * ) kFullscreenTriangle, sizeof( kFullscreenTriangle ) );
      aCommandList->SetRenderTarget( someRenderData.myBackBuffer, nullptr );

      // Left half: quadrants
      ShaderConsts consts = {};
      consts.myDisplayMode = kDisplayTexture;
      consts.myTextureIndex = someRenderData.mySource->GetGlobalDescriptorIndex();
      consts.mySamplerIndex = RenderCore::GetTextureSampler( RenderCore::ourLinearClampSampler )->GetGlobalDescriptorIndex();
      consts.myPixelToUv = glm::float2( 1.0f ) / glm::float2( ( float ) halfWidth, ( float ) height );
      DrawRect( aCommandList, consts, glm::uvec4( 0u, 0u, halfWidth, height ) );

      // Right half, first row: the mips side by side, each at its own level
      consts.myTextureIndex = someRenderData.myMipChain->GetGlobalDescriptorIndex();
      consts.myDisplayMode = kDisplayMip;
      for ( uint mip = 0u; mip < kNumMips; ++mip ) {
        consts.myMipLevel = mip;
        consts.myDisplayScale = ( float ) ( kSubresourceTextureSize >> mip ) / ( float ) cell;
        DrawRect( aCommandList, consts, glm::uvec4( halfWidth + mip * cell, 0u, cell, cell ) );
      }

      // Second row: stencil copy and depth
      consts.myMipLevel = 0u;
      consts.myDisplayScale = ( float ) kSubresourceTextureSize / ( float ) ( cell * 2u );
      consts.myDisplayMode = kDisplayStencil;
      consts.myTextureIndex = someRenderData.myStencilCopy->GetGlobalDescriptorIndex();
      DrawRect( aCommandList, consts, glm::uvec4( halfWidth, cell, cell * 2u, cell * 2u ) );

      consts.myDisplayMode = kDisplayDepth;
      consts.myTextureIndex = someRenderData.myDepth->GetGlobalDescriptorIndex();
      DrawRect( aCommandList, consts, glm::uvec4( halfWidth + cell * 2u, cell, cell * 2u, cell * 2u ) );
    }

    FgResourceHandle mySource;
    FgResourceHandle myMipChain;
    FgResourceHandle myStencilCopy;
    FgResourceHandle myDepth;
    FgResourceHandle myBackBuffer;
    glm::uvec2 myBackBufferSize;
    const ShaderPipeline * myPipeline = nullptr;
    DepthStencilState * myDepthStencilState = nullptr;
  };
  //---------------------------------------------------------------------------//
}  // namespace Priv_Test_FrameGraph
//---------------------------------------------------------------------------//
Test_FrameGraph::Test_FrameGraph( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow, Fancy::RenderOutput * aRenderOutput,
                                  Fancy::InputState * anInputState )
    : Test( anAssetManager, aWindow, aRenderOutput, anInputState, "Frame Graph" ), myFrameGraph( aRenderOutput ) {
  using namespace Priv_Test_FrameGraph;

  myStoreColorsShader = RenderCore::CreateComputeShaderPipeline( kShaderPath, "main", "STORE_COLORS_CS" );
  myFillQuadrantComputeShader = RenderCore::CreateComputeShaderPipeline( kShaderPath, "main", "FILL_QUADRANT_CS" );
  myFillColorShader = RenderCore::CreateVertexPixelShaderPipeline( kShaderPath, "main", "main", "FILL_COLOR" );
  myBlitShader = RenderCore::CreateVertexPixelShaderPipeline( kShaderPath, "main", "main", "BLIT" );
  myDownsampleShader = RenderCore::CreateComputeShaderPipeline( kShaderPath, "main", "DOWNSAMPLE_CS" );
  myCopyStencilShader = RenderCore::CreateComputeShaderPipeline( kShaderPath, "main", "COPY_STENCIL_CS" );
  myFillStencilShader = RenderCore::CreateVertexPixelShaderPipeline( kShaderPath, "main", "main", "FILL_STENCIL" );
  ASSERT( myStoreColorsShader.IsValid() && myFillQuadrantComputeShader.IsValid() && myFillColorShader.IsValid() && myBlitShader.IsValid() );
  ASSERT( myDownsampleShader.IsValid() && myCopyStencilShader.IsValid() && myFillStencilShader.IsValid() );

  DepthStencilStateProperties dsProps;
  dsProps.myDepthTestEnabled = false;
  dsProps.myDepthWriteEnabled = false;
  dsProps.myStencilEnabled = false;
  dsProps.myStencilWriteMask = 0u;
  myDepthTestOff = RenderCore::CreateDepthStencilState( dsProps );

  for ( uint i = 0u; i < kNumStencilRects; ++i ) {
    DepthStencilStateProperties stencilProps;
    stencilProps.myDepthTestEnabled = false;
    stencilProps.myDepthWriteEnabled = false;
    stencilProps.myStencilEnabled = true;
    stencilProps.myTwoSidedStencil = true;
    stencilProps.myFrontFace.myStencilCompFunc = CompFunc::ALWAYS;
    stencilProps.myFrontFace.myStencilPassOp = StencilOp::REPLACE;
    stencilProps.myBackFace.myStencilCompFunc = CompFunc::ALWAYS;
    stencilProps.myBackFace.myStencilPassOp = StencilOp::REPLACE;
    stencilProps.myStencilReadMask = 0xFFu;
    stencilProps.myStencilWriteMask = 0xFFu;
    stencilProps.myStencilRef = ( int ) kStencilRefs[ i ];
    myStencilStates[ i ] = RenderCore::CreateDepthStencilState( stencilProps );
    ASSERT( myStencilStates[ i ].IsValid() );
  }

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = ( uint ) GpuBufferBindFlags::SHADER_BUFFER;
  bufferProps.myIsShaderWritable = true;
  bufferProps.myNumElements = kNumColors;
  bufferProps.myElementSizeBytes = sizeof( glm::float4 );
  myColorBuffer = RenderCore::CreateBuffer( bufferProps, kColorBufferName );
  ASSERT( myColorBuffer.IsValid() );

  CreateQuadrantTexture();

  {
    TextureProperties props;
    props.myWidth = kSubresourceTextureSize;
    props.myHeight = kSubresourceTextureSize;
    props.myFormat = DataFormat::RGBA_8;
    props.myNumMipLevels = kNumMips;
    props.myIsShaderWritable = true;

    // Only mip 0 is uploaded: a checkerboard
    TextureSubData uploadData( props );
    uint8 * pixels = new uint8[ ( size_t ) uploadData.myTotalSizeBytes ];
    for ( uint y = 0u; y < kSubresourceTextureSize; ++y ) {
      for ( uint x = 0u; x < kSubresourceTextureSize; ++x ) {
        const bool isLight = ( ( x / kCheckerCellSize ) + ( y / kCheckerCellSize ) ) % 2u == 0u;
        uint8 * pixel = pixels + y * uploadData.myRowSizeBytes + x * 4u;
        pixel[ 0 ] = isLight ? 240u : 20u;
        pixel[ 1 ] = isLight ? 200u : 60u;
        pixel[ 2 ] = isLight ? 40u : 200u;
        pixel[ 3 ] = 255u;
      }
    }
    uploadData.myData = pixels;
    myMipChainTexture = RenderCore::CreateTexture( props, kMipChainTextureName, &uploadData, 1u );
    delete[] pixels;
    ASSERT( myMipChainTexture.IsValid() );
  }

  {
    TextureProperties props;
    props.myWidth = kSubresourceTextureSize;
    props.myHeight = kSubresourceTextureSize;
    props.myFormat = DataFormat::D_32FLOAT_S_8UI;
    props.bIsDepthStencil = true;
    props.myIsRenderTarget = true;
    myDepthStencilTexture = RenderCore::CreateTexture( props, kDepthStencilTextureName );
    ASSERT( myDepthStencilTexture.IsValid() );
  }

  {
    TextureProperties props;
    props.myWidth = kSubresourceTextureSize;
    props.myHeight = kSubresourceTextureSize;
    props.myFormat = DataFormat::R_8UI;
    props.myIsShaderWritable = true;
    myStencilCopyTexture = RenderCore::CreateTexture( props, kStencilCopyTextureName );
    ASSERT( myStencilCopyTexture.IsValid() );
  }
}
//---------------------------------------------------------------------------//
Test_FrameGraph::~Test_FrameGraph() {
  // The builder's views need to go before the resources
  myFrameGraph.Reset();
  myFrameGraph.ReleaseCompiledGraph();
  RenderCore::DeleteTexture( myQuadrantTexture );
  RenderCore::DeleteTexture( myMipChainTexture );
  RenderCore::DeleteTexture( myDepthStencilTexture );
  RenderCore::DeleteTexture( myStencilCopyTexture );
  RenderCore::DeleteBuffer( myColorBuffer );
}
//---------------------------------------------------------------------------//
void Test_FrameGraph::CreateQuadrantTexture() {
  using namespace Priv_Test_FrameGraph;

  TextureProperties props;
  props.myWidth = kQuadrantTextureSize;
  props.myHeight = kQuadrantTextureSize;
  props.myFormat = DataFormat::RGBA_8;
  props.myIsShaderWritable = true;
  props.myIsRenderTarget = true;

  const TextureHandle oldTexture = myQuadrantTexture;
  myQuadrantTexture = RenderCore::CreateTexture( props, kQuadrantTextureName );
  ASSERT( myQuadrantTexture.IsValid() );

  // The compiled graph has views of the old texture, which need to go before it
  if ( oldTexture.IsValid() ) {
    myFrameGraph.ReleaseCompiledGraph();
    RenderCore::DeleteTexture( oldTexture );
  }
}
//---------------------------------------------------------------------------//
void Test_FrameGraph::OnUpdate( bool aDrawProperties ) {
  using namespace Priv_Test_FrameGraph;

  if ( aDrawProperties ) {
    for ( uint i = 0u; i < kNumQuadrants; ++i ) {
      eastl::string label( eastl::string::CtorSprintf(), "Quadrant %d", i );
      ImGui::Checkbox( label.c_str(), &myQuadrantEnabled[ i ] );
    }
    if ( ImGui::Button( "Recreate texture" ) ) {
      CreateQuadrantTexture();
    }
    ImGui::Text( "Declaration hash: %llx", ( unsigned long long ) myFrameGraph.GetDeclarationHash() );
    ImGui::Text( "Number of compiles: %d", myFrameGraph.GetNumCompiles() );
    ImGui::Checkbox( "Show Frame Graph viewer", &myShowFrameGraphViewer );
  }

  myFrameGraph.Reset();

  const uint half = kQuadrantTextureSize / 2u;
  const glm::uvec4 rects[ kNumQuadrants ] = {
    glm::uvec4( 0u, 0u, half, half ),
    glm::uvec4( half, 0u, half, half ),
    glm::uvec4( 0u, half, half, half ),
    glm::uvec4( half, half, half, half ),
  };
  const glm::float4 colors[ kNumQuadrants ] = {
    glm::float4( 1.0f, 0.0f, 0.0f, 1.0f ),
    glm::float4( 0.0f, 1.0f, 0.0f, 1.0f ),
    glm::float4( 0.0f, 0.0f, 1.0f, 1.0f ),
    glm::float4( 1.0f, 1.0f, 0.0f, 1.0f ),
  };
  const char * names[ kNumQuadrants ] = { "Fill top left (compute)", "Fill top right (raster)", "Fill bottom left (compute)",
                                          "Fill bottom right (raster)" };

  const ShaderPipeline * fillComputeShader = RenderCore::GetShaderPipeline( myFillQuadrantComputeShader );
  const ShaderPipeline * fillColorShader = RenderCore::GetShaderPipeline( myFillColorShader );
  DepthStencilState * depthTestOff = RenderCore::GetDepthStencilState( myDepthTestOff );

  // All resources are registered every frame, before the tasks are set up
  const FgResourceHandle colorBuffer = myFrameGraph.RegisterExisting( myColorBuffer );
  const FgResourceHandle quadrants = myFrameGraph.RegisterExisting( myQuadrantTexture );
  const FgResourceHandle mipChain = myFrameGraph.RegisterExisting( myMipChainTexture );
  const FgResourceHandle depthStencil = myFrameGraph.RegisterExisting( myDepthStencilTexture );
  const FgResourceHandle stencilCopy = myFrameGraph.RegisterExisting( myStencilCopyTexture );
  const FgResourceHandle backBuffer = myFrameGraph.RegisterBackBuffer();

  StoreColorsTask::Setup( myFrameGraph, colorBuffer, colors, RenderCore::GetShaderPipeline( myStoreColorsShader ) );

  for ( uint i = 0u; i < kNumQuadrants; ++i ) {
    if ( !myQuadrantEnabled[ i ] ) {
      continue;
    }
    if ( i % 2u == 0u ) {
      FillQuadrantComputeTask::Setup( myFrameGraph, names[ i ], quadrants, colorBuffer, i, rects[ i ], fillComputeShader );
    } else {
      FillQuadrantRasterTask::Setup( myFrameGraph, names[ i ], quadrants, colorBuffer, i, rects[ i ], fillColorShader, depthTestOff );
    }
  }

  const ShaderPipeline * downsampleShader = RenderCore::GetShaderPipeline( myDownsampleShader );
  for ( uint mip = 1u; mip < kNumMips; ++mip ) {
    DownsampleMipTask::Setup( myFrameGraph, mipChain, mip, downsampleShader );
  }

  DepthStencilState * stencilStates[ kNumStencilRects ];
  for ( uint i = 0u; i < kNumStencilRects; ++i ) {
    stencilStates[ i ] = RenderCore::GetDepthStencilState( myStencilStates[ i ] );
  }
  DrawStencilPatternTask::Setup( myFrameGraph, depthStencil, RenderCore::GetShaderPipeline( myFillStencilShader ), stencilStates );
  CopyStencilTask::Setup( myFrameGraph, depthStencil, stencilCopy, RenderCore::GetShaderPipeline( myCopyStencilShader ) );

  BlitToBackBufferTask::Setup( myFrameGraph, quadrants, mipChain, stencilCopy, depthStencil, backBuffer,
                               glm::uvec2( myWindow->GetWidth(), myWindow->GetHeight() ), RenderCore::GetShaderPipeline( myBlitShader ),
                               depthTestOff );

  myFrameGraph.BuildFrameGraph();

  if ( myShowFrameGraphViewer ) {
    myFrameGraphViewer.Render( myFrameGraph, &myShowFrameGraphViewer );
  }
}
//---------------------------------------------------------------------------//
void Test_FrameGraph::OnRender() {
  if ( myFrameGraph.IsBuilt() ) {
    myFrameGraph.Execute();
  }
}
//---------------------------------------------------------------------------//
