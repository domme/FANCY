#include "Test_Raytracing.h"
#include "Rendering/GpuBuffer.h"
#include "Rendering/GpuBufferProperties.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>

#include "Rendering/CommandList.h"
#include "Rendering/DX12/CommandListDX12.h"
#include "Rendering/RtAccelerationStructure.h"
#include "Rendering/RtPipelineState.h"
#include "Rendering/RtShaderBindingTable.h"
#include "Rendering/ShaderDesc.h"
#include "Rendering/RenderEnums.h"
#include "Rendering/RenderOutput.h"
#include "Rendering/TempResourcePool.h"
#include "Common/Window.h"

using namespace Fancy;

Test_Raytracing::Test_Raytracing(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
    : Application(aRuntime, aWindow, aRenderOutput, anInputState, "Raytracing")
{
  // Create bottom level BVH
  eastl::vector<glm::float3> vertices = {
    {  1.0f, -1.0f, 0.0f },
    { -1.0f, -1.0f, 0.0f},
    { 0.0f, 1.0f, 0.0f }
  };

  // Setup indices
  eastl::vector<unsigned short> indices = { 1, 2, 0 };
  
  RtAccelerationStructureGeometryData geometryData;
  geometryData.myType = RtAccelerationStructureGeometryType::TRIANGLES;
  geometryData.myFlags = (uint)RtAccelerationStructureGeometryFlags::OPAQUE_GEOMETRY;
  geometryData.myVertexFormat = DataFormat::RGB_32F;
  geometryData.myNumVertices = (uint)vertices.size();
  geometryData.myIndexFormat = DataFormat::R_16UI;
  geometryData.myNumIndices = (uint)indices.size();
  geometryData.myVertexData.myType = RT_BUFFER_DATA_TYPE_CPU_DATA;
  geometryData.myVertexData.myCpuData.myData = vertices.data();
  geometryData.myVertexData.myCpuData.myDataSize = VECTOR_BYTESIZE(vertices);
  geometryData.myIndexData.myType = RT_BUFFER_DATA_TYPE_CPU_DATA;
  geometryData.myIndexData.myCpuData.myData = indices.data();
  geometryData.myIndexData.myCpuData.myDataSize = VECTOR_BYTESIZE(indices);

  myBLAS = RenderCore::CreateRtBottomLevelAccelerationStructure(&geometryData, 1u);

  RtAccelerationStructureInstanceData instanceData;
  instanceData.myInstanceId = 0;
  instanceData.mySbtHitGroupOffset = 0;
  instanceData.myInstanceBLAS = myBLAS;
  instanceData.myInstanceMask = UINT8_MAX;
  instanceData.myFlags = RT_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE | RT_INSTANCE_FLAG_FORCE_OPAQUE;
  myTLAS = RenderCore::CreateRtTopLevelAccelerationStructure(&instanceData, 1u);

  RtPipelineStateProperties rtPipelineProps;
  const uint raygenIdx = rtPipelineProps.AddRayGenShader("fancy/resources/shaders/Tests/Raytracing/RayGen.hlsl", "RayGen");
  const uint missIdx = rtPipelineProps.AddMissShader("fancy/resources/shaders/Tests/Raytracing/Miss.hlsl", "Miss");
  const uint hitIdx = rtPipelineProps.AddHitGroup(L"HitGroup0", RT_HIT_GROUP_TYPE_TRIANGLES, nullptr, nullptr, nullptr, nullptr, "fancy/resources/shaders/Tests/Raytracing/Hit.hlsl", "ClosestHit");
  rtPipelineProps.SetMaxAttributeSize(32u);
  rtPipelineProps.SetMaxPayloadSize(128u);
  rtPipelineProps.SetMaxRecursionDepth(2u);
  myRtPso = RenderCore::CreateRtPipelineState(rtPipelineProps);

  RtShaderBindingTableProperties sbtProps;
  sbtProps.myNumRaygenShaderRecords = 1;
  sbtProps.myNumMissShaderRecords = 5;
  sbtProps.myNumHitShaderRecords = 5;
  mySBT = RenderCore::CreateRtShaderTable(sbtProps);
  mySBT->AddShaderRecord(myRtPso->GetRayGenShaderIdentifier(raygenIdx));
  mySBT->AddShaderRecord(myRtPso->GetMissShaderIdentifier(missIdx));
  mySBT->AddShaderRecord(myRtPso->GetHitShaderIdentifier(hitIdx));
}

void Test_Raytracing::OnWindowResized(uint aWidth, uint aHeight)
{
}

void Test_Raytracing::OnUpdate(bool aDrawProperties)
{

}

void Test_Raytracing::OnRender()
{
  TextureResourceProperties texProps;
  texProps.myIsShaderWritable = true;
  texProps.myIsRenderTarget = false;
  texProps.myTextureProperties.myFormat = DataFormat::RGBA_8;
  texProps.myTextureProperties.myWidth = myWindow->GetWidth();
  texProps.myTextureProperties.myHeight = myWindow->GetHeight();
  TempTextureResource rtOutputTex = RenderCore::AllocateTempTexture(texProps, 0u, "RT Test Result Texture");

  CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
  ctx->SetRaytracingPipelineState(myRtPso.get());

  ctx->PrepareResourceShaderAccess(rtOutputTex.myWriteView);
  ctx->PrepareResourceShaderAccess(myTLAS->GetBufferRead());

  struct Consts
  {
    glm::float3 myCamCenter;
    uint myIsBGR;

    glm::float4 myPixelToWorldScaleOffset;

    uint myOutTexIndex;
    uint myAsIndex;
  } consts;


  float viewportSize = 2.0f;
  consts.myCamCenter = glm::float3(0.0f, 0.0f, -1.0f);
  consts.myIsBGR = myOutput->GetBackbuffer()->GetProperties().myFormat == DataFormat::BGRA_8 ? 1 : 0;
  consts.myPixelToWorldScaleOffset.x = viewportSize / texProps.myTextureProperties.myWidth;
  consts.myPixelToWorldScaleOffset.y = viewportSize / texProps.myTextureProperties.myHeight;
  consts.myPixelToWorldScaleOffset.z = -viewportSize * 0.5f;
  consts.myPixelToWorldScaleOffset.w = -viewportSize * 0.5f;
  consts.myOutTexIndex = rtOutputTex.myWriteView->GetGlobalDescriptorIndex();
  consts.myAsIndex = myTLAS->GetBufferRead()->GetGlobalDescriptorIndex();
  
  ctx->BindConstantBuffer(&consts, sizeof(consts), 0);
    
  DispatchRaysDesc desc;
  desc.myRayGenShaderTableRange = mySBT->GetRayGenRange();
  desc.myMissShaderTableRange = mySBT->GetMissRange();
  desc.myHitGroupTableRange = mySBT->GetHitRange();
  desc.myWidth = texProps.myTextureProperties.myWidth;
  desc.myHeight = texProps.myTextureProperties.myHeight;
  desc.myDepth = 1;
  ctx->DispatchRays(desc);

  ctx->ResourceUAVbarrier(rtOutputTex.myTexture);
  SubresourceLocation subresourceLoc;
  TextureRegion region = { glm::uvec3(0), glm::uvec3(texProps.myTextureProperties.myWidth, texProps.myTextureProperties.myHeight, 1u) };

  Texture* backbuffer = myOutput->GetBackbuffer();
  ctx->CopyTexture(backbuffer, subresourceLoc, region, rtOutputTex.myTexture, subresourceLoc, region);

  RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
}
