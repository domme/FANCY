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
  };
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
      StoreColorsTask & task = FgAddTask< StoreColorsTask >( aBuilder, "Store quadrant colors", CommandListType::Compute );
      task.myColorBuffer = aBuilder.Write( aColorBuffer, RESOURCE_ACCESS_COMPUTE_SHADER_WRITE );
      for ( uint i = 0u; i < kNumColors; ++i ) {
        task.myColors[ i ] = someColors[ i ];
      }
      task.myPipeline = aPipeline;
      return task;
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
      FillQuadrantComputeTask & task = FgAddTask< FillQuadrantComputeTask >( aBuilder, aName, CommandListType::Compute );
      task.myTarget = aBuilder.Write( aTarget, RESOURCE_ACCESS_COMPUTE_SHADER_WRITE );
      task.myColorBuffer = aBuilder.Read( aColorBuffer, RESOURCE_ACCESS_COMPUTE_SHADER_READ );
      task.myRect = aRect;
      task.myQuadrantIndex = aQuadrantIndex;
      task.myPipeline = aPipeline;
      return task;
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
      FillQuadrantRasterTask & task = FgAddTask< FillQuadrantRasterTask >( aBuilder, aName, CommandListType::Graphics );
      task.myTarget = aBuilder.Write( aTarget, RESOURCE_ACCESS_RENDER_TARGET );
      task.myColorBuffer = aBuilder.Read( aColorBuffer, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task.myRect = aRect;
      task.myQuadrantIndex = aQuadrantIndex;
      task.myPipeline = aPipeline;
      task.myDepthStencilState = aDepthStencilState;
      return task;
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
  class BlitToBackBufferTask {
  public:
    struct RenderData {
      TextureView * mySource = nullptr;
      TextureView * myBackBuffer = nullptr;
    };

    static BlitToBackBufferTask & Setup( FrameGraphBuilder & aBuilder, FgResourceHandle aSource, FgResourceHandle aBackBuffer,
                                         const glm::uvec2 & aBackBufferSize, const ShaderPipeline * aPipeline,
                                         DepthStencilState * aDepthStencilState ) {
      BlitToBackBufferTask & task = FgAddTask< BlitToBackBufferTask >( aBuilder, "Blit to backbuffer", CommandListType::Graphics );
      task.mySource = aBuilder.Read( aSource, RESOURCE_ACCESS_FRAGMENT_SHADER_READ );
      task.myBackBuffer = aBuilder.Write( aBackBuffer, RESOURCE_ACCESS_RENDER_TARGET );
      task.myBackBufferSize = aBackBufferSize;
      task.myPipeline = aPipeline;
      task.myDepthStencilState = aDepthStencilState;
      return task;
    }

    static void Compile( const BlitToBackBufferTask & aTask, const FrameGraphCompileContext & aContext, RenderData & someRenderData ) {
      someRenderData.mySource = aContext.GetSrv( aTask.mySource );
      someRenderData.myBackBuffer = aContext.GetRtv( aTask.myBackBuffer );
    }

    static void Execute( CommandList * aCommandList, const BlitToBackBufferTask & aTask, const RenderData & someRenderData ) {
      ShaderConsts consts = {};
      consts.myTextureIndex = someRenderData.mySource->GetGlobalDescriptorIndex();
      consts.mySamplerIndex = RenderCore::GetTextureSampler( RenderCore::ourLinearClampSampler )->GetGlobalDescriptorIndex();
      consts.myPixelToUv = glm::float2( 1.0f ) / glm::float2( aTask.myBackBufferSize );

      const glm::uvec4 rect( 0u, 0u, aTask.myBackBufferSize.x, aTask.myBackBufferSize.y );
      aCommandList->SetShaderPipeline( aTask.myPipeline );
      aCommandList->SetDepthStencilState( aTask.myDepthStencilState );
      aCommandList->SetCullMode( CullMode::NONE );
      aCommandList->SetViewport( rect );
      aCommandList->SetClipRect( rect );
      aCommandList->BindConstantBuffer( &consts, sizeof( consts ), 0 );
      aCommandList->BindVertexBuffer( ( void * ) kFullscreenTriangle, sizeof( kFullscreenTriangle ) );
      aCommandList->SetRenderTarget( someRenderData.myBackBuffer, nullptr );
      aCommandList->DrawInstanced( 3, 1, 0, 0 );
    }

    FgResourceHandle mySource;
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
  ASSERT( myStoreColorsShader.IsValid() && myFillQuadrantComputeShader.IsValid() && myFillColorShader.IsValid() && myBlitShader.IsValid() );

  DepthStencilStateProperties dsProps;
  dsProps.myDepthTestEnabled = false;
  dsProps.myDepthWriteEnabled = false;
  dsProps.myStencilEnabled = false;
  dsProps.myStencilWriteMask = 0u;
  myDepthTestOff = RenderCore::CreateDepthStencilState( dsProps );

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = ( uint ) GpuBufferBindFlags::SHADER_BUFFER;
  bufferProps.myIsShaderWritable = true;
  bufferProps.myNumElements = kNumColors;
  bufferProps.myElementSizeBytes = sizeof( glm::float4 );
  myColorBuffer = RenderCore::CreateBuffer( bufferProps, kColorBufferName );
  ASSERT( myColorBuffer.IsValid() );

  CreateQuadrantTexture();
}
//---------------------------------------------------------------------------//
Test_FrameGraph::~Test_FrameGraph() {
  // The builder's views need to go before the resources
  myFrameGraph.Reset();
  myFrameGraph.ReleaseCompiledGraph();
  RenderCore::DeleteTexture( myQuadrantTexture );
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
  FgResourceHandle quadrants = myFrameGraph.RegisterExisting( myQuadrantTexture );
  const FgResourceHandle backBuffer = myFrameGraph.RegisterBackBuffer();

  const StoreColorsTask & storeColorsTask =
      StoreColorsTask::Setup( myFrameGraph, colorBuffer, colors, RenderCore::GetShaderPipeline( myStoreColorsShader ) );
  const FgResourceHandle quadrantColors = storeColorsTask.myColorBuffer;

  bool isAnyQuadrantEnabled = false;
  for ( uint i = 0u; i < kNumQuadrants; ++i ) {
    if ( !myQuadrantEnabled[ i ] ) {
      continue;
    }
    isAnyQuadrantEnabled = true;

    if ( i % 2u == 0u ) {
      const FillQuadrantComputeTask & task =
          FillQuadrantComputeTask::Setup( myFrameGraph, names[ i ], quadrants, quadrantColors, i, rects[ i ], fillComputeShader );
      quadrants = task.myTarget;
    } else {
      const FillQuadrantRasterTask & task =
          FillQuadrantRasterTask::Setup( myFrameGraph, names[ i ], quadrants, quadrantColors, i, rects[ i ], fillColorShader, depthTestOff );
      quadrants = task.myTarget;
    }
  }

  if ( isAnyQuadrantEnabled ) {
    BlitToBackBufferTask::Setup( myFrameGraph, quadrants, backBuffer, glm::uvec2( myWindow->GetWidth(), myWindow->GetHeight() ),
                                 RenderCore::GetShaderPipeline( myBlitShader ), depthTestOff );
  }

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
