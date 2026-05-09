#include "Test_SharedQueueResourceUsage.h"

#include "Rendering/RenderCore.h"
#include "Rendering/CommandList.h"
#include "Rendering/CommandQueue.h"
#include "Rendering/ShaderPipelineDesc.h"
#include "Rendering/TempResourcePool.h"

#include "EASTL/vector.h"

using namespace Fancy;

static uint kNumBufferElements = 1024;

Test_SharedQueueResourceUsage::Test_SharedQueueResourceUsage( Fancy::AssetManager * anAssetManager, Fancy::Window * aWindow,
                                                              Fancy::RenderOutput * aRenderOutput, Fancy::InputState * anInputState )
    : Test( anAssetManager, aWindow, aRenderOutput, anInputState, "Shared Queue Resources" ) {
  GpuBufferProperties props;
  props.myElementSizeBytes = sizeof( uint );
  props.myNumElements = kNumBufferElements;
  props.myIsShaderWritable = true;
  props.myCpuAccess = CpuMemoryAccessType::NO_CPU_ACCESS;
  props.myBindFlags = ( uint ) GpuBufferBindFlags::SHADER_BUFFER;

  eastl::vector< uint > initialData;
  initialData.resize( props.myNumElements, 0u );

  myBuffer = RenderCore::CreateBuffer( props, "Shared Queue Test Buffer", initialData.data() );

  GpuBufferViewProperties viewProps;
  viewProps.myFormat = DataFormat::R_32UI;
  viewProps.myIsShaderWritable = true;
  GpuBuffer * buf = RenderCore::GetBuffer( myBuffer );
  myBufferWrite = RenderCore::CreateBufferView( buf, viewProps, "Shared Queue Test Buffer UAV" );

  viewProps.myIsShaderWritable = false;
  myBufferRead = RenderCore::CreateBufferView( buf, viewProps, "Shared Queue Test Buffer SRV" );

  ShaderPipelineDesc pipelineDesc;
  ShaderDesc &       shaderDesc = pipelineDesc.myShader[ ( uint ) ShaderStage::SHADERSTAGE_COMPUTE ];
  shaderDesc.myShaderStage = ( uint ) ShaderStage::SHADERSTAGE_COMPUTE;
  shaderDesc.myPath = "fancy/resources/shaders/Tests/ModifyBuffer.hlsl";
  shaderDesc.myMainFunction = "main_increment";
  myWriteBufferShader = RenderCore::CreateShaderPipeline( pipelineDesc );

  shaderDesc.myMainFunction = "main_copy";
  myCopyBufferShader = RenderCore::CreateShaderPipeline( pipelineDesc );
}

Test_SharedQueueResourceUsage::~Test_SharedQueueResourceUsage() {
  RenderCore::WaitForIdle( CommandListType::Graphics );
  RenderCore::WaitForIdle( CommandListType::Compute );
}

void Test_SharedQueueResourceUsage::OnUpdate( bool aDrawProperties ) {
  struct CBuffer {
    uint myValue;
    uint myDstBufferIndex;
    uint mySrcBufferIndex;
  } cbuf;

  CommandList * graphicsContext = RenderCore::BeginCommandList( CommandListType::Graphics );
  graphicsContext->SetShaderPipeline( RenderCore::GetShaderPipeline( myWriteBufferShader ) );

  cbuf.myDstBufferIndex = RenderCore::GetBufferView( myBufferWrite )->GetGlobalDescriptorIndex();
  graphicsContext->BindConstantBuffer( &cbuf, sizeof( cbuf ), 0 );
  graphicsContext->Dispatch( glm::int3( kNumBufferElements, 1, 1 ) );
  graphicsContext->GlobalBarrier( BarrierSyncScope::AllShading, BarrierSyncScope::All, CacheFlush::ShaderWrite );
  const uint64 setValueFence = RenderCore::ExecuteAndFreeCommandList( graphicsContext );

  GpuBuffer * buf = RenderCore::GetBuffer( myBuffer );
  GpuBufferResourceProperties props;
  props.myBufferProperties = buf->GetProperties();
  props.myFormat = DataFormat::R_32UI;
  props.myIsShaderWritable = true;
  TempBufferResource tempBuffer = RenderCore::AllocateTempBuffer( props, 0u, "Temp buffer" );

  // Use the resource on the compute context in multiple readonly operations
  RenderCore::GetCommandQueue( CommandListType::Compute )->StallForFence( setValueFence );
  CommandList * computeContext = RenderCore::BeginCommandList( CommandListType::Compute );
  computeContext->SetShaderPipeline( RenderCore::GetShaderPipeline( myCopyBufferShader ) );

  cbuf.mySrcBufferIndex = RenderCore::GetBufferView( myBufferRead )->GetGlobalDescriptorIndex();
  cbuf.myDstBufferIndex = tempBuffer.myWriteView->GetGlobalDescriptorIndex();
  computeContext->BindConstantBuffer( &cbuf, sizeof( cbuf ), 0 );
  computeContext->Dispatch( glm::int3( kNumBufferElements, 1, 1 ) );

  computeContext->GlobalBarrier( BarrierSyncScope::AllShading, BarrierSyncScope::AllShading, CacheFlush::ShaderWrite );

  computeContext->CopyBuffer( tempBuffer.myBuffer, 0u, buf, 0u, sizeof( uint ) * kNumBufferElements );

  const uint64 copyBufferFence = RenderCore::ExecuteAndFreeCommandList( computeContext );
}

void Test_SharedQueueResourceUsage::OnRender() {}
