#include "Test_ModelViewer.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/span.h>

#include "Common/Window.h"
#include "Rendering/ShaderPipelineDesc.h"
#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"
#include "Rendering/RenderOutput.h"
#include "Rendering/GpuResourceView.h"
#include "IO/Mesh.h"
#include "Rendering/TextureSampler.h"
#include "Rendering/ShaderPipeline.h"
#include "Rendering/Shader.h"
#include "Rendering/Texture.h"
#include "Common/StringUtil.h"
#include "imgui.h"
#include "IO/Assets.h"
#include "Rendering/GpuBufferProperties.h"
#include "Rendering/GpuBuffer.h"
#include "IO/Material.h"
#include "IO/Scene.h"

using namespace Fancy;

bool ourDrawInstanced = false;

static ShaderPipelineHandle locLoadShader( const char * aShaderPath, const char * aMainVtxFunction = "main",
                                           const char * aMainFragmentFunction = "main",
                                           const char * someDefines = nullptr ) {
  eastl::vector< eastl::string > defines;
  if ( someDefines )
    StringUtil::Tokenize( someDefines, ",", defines );

  ShaderPipelineDesc pipelineDesc;

  ShaderDesc * shaderDesc = &pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_VERTEX ];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainVtxFunction;
  for ( const eastl::string & str : defines )
    shaderDesc->myDefines.push_back( str );

  shaderDesc = &pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_FRAGMENT ];
  shaderDesc->myPath = aShaderPath;
  shaderDesc->myMainFunction = aMainFragmentFunction;
  for ( const eastl::string & str : defines )
    shaderDesc->myDefines.push_back( str );

  return RenderCore::CreateShaderPipeline( pipelineDesc );
}

//---------------------------------------------------------------------------//

Test_ModelViewer::Test_ModelViewer( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow,
                                    Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState )
    : Test( anAssetManager, aWindow, aRenderOutput, anInputState, "Model Viewer" ), myCameraController( &myCamera ) {
  myUnlitTexturedShader = locLoadShader( "fancy/resources/shaders/Unlit_Textured.hlsl" );
  ASSERT( myUnlitTexturedShader.IsValid() );

  myInstancedUnlitTexturedShader =
      locLoadShader( "fancy/resources/shaders/Unlit_Textured.hlsl", "main", "main", "INSTANCED" );
  ASSERT( myInstancedUnlitTexturedShader.IsValid() );

  myUnlitVertexColorShader = locLoadShader( "fancy/resources/shaders/Unlit_Colored.hlsl" );
  ASSERT( myUnlitVertexColorShader.IsValid() );

  myDebugGeoShader = locLoadShader( "fancy/resources/shaders/DebugGeo_Colored.hlsl" );
  ASSERT( myDebugGeoShader.IsValid() );

  TextureSamplerProperties samplerProps;
  samplerProps.myAddressModeX = SamplerAddressMode::REPEAT;
  samplerProps.myAddressModeY = SamplerAddressMode::REPEAT;
  samplerProps.myAddressModeZ = SamplerAddressMode::REPEAT;
  samplerProps.myMinFiltering = SamplerFilterMode::TRILINEAR;
  samplerProps.myMagFiltering = SamplerFilterMode::TRILINEAR;
  mySampler = RenderCore::CreateTextureSampler( samplerProps );

  myCamera.myPosition = glm::float3( 0.0f, 0.0f, -10.0f );
  myCamera.myOrientation = glm::quat_cast( glm::lookAt(
      glm::float3( 0.0f, 0.0f, 10.0f ), glm::float3( 0.0f, 0.0f, 0.0f ), glm::float3( 0.0f, 1.0f, 0.0f ) ) );

  myCameraController.myMoveSpeed = 50.0f;

  myCamera.myFovDeg = 60.0f;
  myCamera.myNear = 1.0f;
  myCamera.myFar = 10000.0f;
  myCamera.myWidth = ( float ) myWindow->GetWidth();
  myCamera.myHeight = ( float ) myWindow->GetHeight();
  myCamera.myIsOrtho = false;

  myCamera.UpdateView();
  myCamera.UpdateProjection();

  eastl::fixed_vector< VertexShaderAttributeDesc, 16 > vertexAttributes;
  vertexAttributes.push_back( { VertexAttributeSemantic::POSITION, 0u, DataFormat::RGB_32F } );
  vertexAttributes.push_back( { VertexAttributeSemantic::TEXCOORD, 0u, DataFormat::RG_32F } );

  SceneData    sceneData;
  MeshImporter importer;
  const bool   importSuccess = importer.Import( "fancy/resources/models/cube.obj", vertexAttributes, sceneData );
  ASSERT( importSuccess );

  myScene = eastl::make_unique< Scene >( sceneData );

  VertexInputLayoutProperties instancedVertexLayoutProps = sceneData.myVertexInputLayoutProperties;
  instancedVertexLayoutProps.myAttributes.push_back(
      { DataFormat::RGB_32F, VertexAttributeSemantic::POSITION, 1u, 1u } );
  instancedVertexLayoutProps.myBufferBindings.push_back( { 12u, VertexInputRate::PER_INSTANCE } );
  myInstancedVertexLayout = RenderCore::CreateVertexInputLayout( instancedVertexLayoutProps );

  int numInstancesOneSide = 20;
  int numInstances = numInstancesOneSide * numInstancesOneSide * numInstancesOneSide;
  myNumInstances = numInstances;
  float                        offsetBetweenInstances = 7.0f;
  eastl::vector< glm::float3 > instancePositions;
  instancePositions.reserve( numInstances );
  for ( int x = -numInstancesOneSide / 2; x < numInstancesOneSide / 2; ++x )
    for ( int y = -numInstancesOneSide / 2; y < numInstancesOneSide / 2; ++y )
      for ( int z = -numInstancesOneSide / 2; z < numInstancesOneSide / 2; ++z )
        instancePositions.push_back(
            glm::float3( x * offsetBetweenInstances, y * offsetBetweenInstances, z * offsetBetweenInstances ) );

  GpuBufferProperties bufferProps;
  bufferProps.myBindFlags = ( uint ) GpuBufferBindFlags::VERTEX_BUFFER;
  bufferProps.myElementSizeBytes = sizeof( glm::float3 );
  bufferProps.myNumElements = numInstances;
  myInstancePositions =
      RenderCore::CreateBuffer( bufferProps, "Test_ModelViewer/InstancePositions", instancePositions.data() );

  UpdateDepthbuffer();
}

Test_ModelViewer::~Test_ModelViewer() {
  RenderCore::WaitForIdle( CommandListType::Graphics );
}

void Test_ModelViewer::OnWindowResized( uint aWidth, uint aHeight ) {
  myCamera.myWidth = myWindow->GetWidth();
  myCamera.myHeight = myWindow->GetHeight();
  myCamera.UpdateProjection();
  UpdateDepthbuffer();
}

void Test_ModelViewer::OnUpdate( bool aDrawProperties ) {
  myCameraController.Update( 0.016f, *myInput );

  if ( aDrawProperties ) {
    ImGui::Checkbox( "Instanced", &ourDrawInstanced );
  }
}

void Test_ModelViewer::OnRender() {
  CommandList * ctx = RenderCore::BeginCommandList( CommandListType::Graphics );
  ctx->ClearDepthStencilTarget( RenderCore::GetTextureView( myDepthStencilDsv ), 1.0f, 0u,
                                ( uint ) DepthStencilClearFlags::CLEAR_ALL );

  RenderGrid( ctx );
  RenderScene( ctx );

  RenderCore::ExecuteAndFreeCommandList( ctx );
}

void Test_ModelViewer::UpdateDepthbuffer() {
  uint width = myWindow->GetWidth();
  uint height = myWindow->GetHeight();

  TextureProperties dsTexProps;
  dsTexProps.myDimension = GpuResourceDimension::TEXTURE_2D;
  dsTexProps.bIsDepthStencil = true;
  dsTexProps.myFormat = DataFormat::D_24UNORM_S_8UI;
  dsTexProps.myIsRenderTarget = true;
  dsTexProps.myIsShaderWritable = false;
  dsTexProps.myWidth = width;
  dsTexProps.myHeight = height;
  dsTexProps.myNumMipLevels = 1u;

  TextureHandle dsTexture = RenderCore::CreateTexture( dsTexProps, "Backbuffer DepthStencil Texture" );
  ASSERT( dsTexture.IsValid() );

  TextureViewProperties props;
  props.myDimension = GpuResourceDimension::TEXTURE_2D;
  props.myIsRenderTarget = true;
  props.myFormat = DataFormat::D_24UNORM_S_8UI;
  props.mySubresourceRange = RenderCore::GetTexture( dsTexture )->mySubresources;
  myDepthStencilDsv =
      RenderCore::CreateTextureView( RenderCore::GetTexture( dsTexture ), props, "DepthStencil Texture" );
  ASSERT( myDepthStencilDsv.IsValid() );
}

void Test_ModelViewer::RenderGrid( Fancy::CommandList * ctx ) {
  ctx->SetViewport( glm::uvec4( 0, 0, myWindow->GetWidth(), myWindow->GetHeight() ) );
  ctx->SetClipRect( glm::uvec4( 0, 0, myWindow->GetWidth(), myWindow->GetHeight() ) );
  ctx->SetRenderTarget( myOutput->GetBackbufferRtv(), RenderCore::GetTextureView( myDepthStencilDsv ) );

  ctx->SetDepthStencilState( nullptr );
  ctx->SetBlendState( nullptr );
  ctx->SetCullMode( CullMode::NONE );
  ctx->SetFillMode( FillMode::SOLID );
  ctx->SetWindingOrder( WindingOrder::CCW );

  ctx->SetShaderPipeline( RenderCore::GetShaderPipeline( myDebugGeoShader ) );

  struct Cbuffer_DebugGeo {
    glm::float4x4 myWorldViewProj;
    glm::float4   myColor;
  };
  Cbuffer_DebugGeo cbuffer_debugGeo{
    myCamera.myViewProj,
    glm::float4( 1.0f, 0.0f, 0.0f, 1.0f ),
  };
  ctx->BindConstantBuffer( &cbuffer_debugGeo, sizeof( cbuffer_debugGeo ), 0 );

  struct GridGeoVertex {
    glm::float3 myPos;
    glm::float4 myColor;
  };

  GridGeoVertex vertices[ 4 ] = { { { 0.0f, 0.0f, -1.0f }, { 0, 0, 1.0f, 1.0f } },
                                  { { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
                                  { { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
                                  { { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } } };
  ctx->BindVertexBuffer( vertices, sizeof( vertices ) );

  uint indices[] = { 0, 1, 2, 3 };
  ctx->BindIndexBuffer( indices, sizeof( indices ), sizeof( indices[ 0 ] ) );

  ctx->SetTopologyType( TopologyType::LINES );
  ctx->Render( 4, 1, 0, 0, 0 );
}

void Test_ModelViewer::RenderScene( Fancy::CommandList * ctx ) {
  ctx->SetViewport( glm::uvec4( 0, 0, myWindow->GetWidth(), myWindow->GetHeight() ) );
  ctx->SetClipRect( glm::uvec4( 0, 0, myWindow->GetWidth(), myWindow->GetHeight() ) );
  ctx->SetRenderTarget( myOutput->GetBackbufferRtv(), RenderCore::GetTextureView( myDepthStencilDsv ) );

  ctx->SetDepthStencilState( nullptr );
  ctx->SetBlendState( nullptr );
  ctx->SetCullMode( CullMode::NONE );
  ctx->SetFillMode( FillMode::SOLID );
  ctx->SetWindingOrder( WindingOrder::CCW );

  ctx->SetTopologyType( TopologyType::TRIANGLE_LIST );
  ctx->SetShaderPipeline( ourDrawInstanced ? RenderCore::GetShaderPipeline( myInstancedUnlitTexturedShader )
                                           : RenderCore::GetShaderPipeline( myUnlitTexturedShader ) );

  const uint numInstances = ourDrawInstanced ? ( uint ) myNumInstances : 1u;

  auto RenderMesh = [ ctx, numInstances, this ]( Mesh * mesh ) {
    for ( uint partIdx = 0u; partIdx < mesh->myParts.size(); ++partIdx ) {
      MeshPart * meshPart = mesh->myParts[ partIdx ].get();
      const VertexInputLayout * layout = RenderCore::GetVertexInputLayout( meshPart->myVertexInputLayout );
      ctx->SetVertexInputLayout( ourDrawInstanced ? RenderCore::GetVertexInputLayout( myInstancedVertexLayout ) : layout );

      GpuBuffer * vertexBuffer = RenderCore::GetBuffer( meshPart->myVertexBuffer );
      GpuBuffer * instanceBuffer = RenderCore::GetBuffer( myInstancePositions );
      GpuBuffer * indexBuffer = RenderCore::GetBuffer( meshPart->myIndexBuffer );

      uint64            offsets[] = { 0u, 0u };
      uint64            sizes[] = { vertexBuffer->GetByteSize(), instanceBuffer->GetByteSize() };
      const GpuBuffer * buffers[] = { vertexBuffer, instanceBuffer };
      ctx->BindVertexBuffers( buffers, offsets, sizes, ourDrawInstanced ? 2u : 1u );
      ctx->BindIndexBuffer( indexBuffer, indexBuffer->GetProperties().myElementSizeBytes );

      ctx->Render( indexBuffer->GetProperties().myNumElements, numInstances, 0, 0, 0 );
    }
  };

  for ( SceneMeshInstance & meshInstance : myScene->myInstances ) {
    Mesh *        mesh = Assets::GetMesh( myScene->myMeshes[ meshInstance.myMeshIndex ] );
    glm::float4x4 transform = meshInstance.myTransform;
    Material *    material = Assets::GetMaterial( myScene->myMaterials[ meshInstance.myMaterialIndex ] );

    struct Cbuffer_PerObject {
      glm::float4x4 myWorldViewProj;
      uint          myTextureIndex;
      uint          mySamplerIndex;
    };
    Cbuffer_PerObject cbuffer_perObject{ myCamera.myViewProj * transform, UINT_MAX,
                                         RenderCore::GetTextureSampler( mySampler )->GetGlobalDescriptorIndex() };

    const GpuResourceView * diffuseTex =
        RenderCore::GetTextureView( material->myTextures[ ( uint ) MaterialTextureType::BASE_COLOR ] );
    if ( diffuseTex ) {
      ctx->PrepareResourceShaderAccess( diffuseTex );
      cbuffer_perObject.myTextureIndex = diffuseTex->GetGlobalDescriptorIndex();
    }

    ctx->BindConstantBuffer( &cbuffer_perObject, sizeof( cbuffer_perObject ), 0 );

    RenderMesh( mesh );
  }
}
