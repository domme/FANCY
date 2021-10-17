#include "Test_Raytracing.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_core/GpuBufferProperties.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>

#include "fancy_core/CommandList.h"
#include "fancy_core/CommandListDX12.h"
#include "fancy_core/RtAccelerationStructure.h"
#include "fancy_core/RtPipelineState.h"
#include "fancy_core/RtShaderBindingTable.h"
#include "fancy_core/ShaderDesc.h"
#include "fancy_core/RenderEnums.h"
#include "fancy_core/RenderOutput.h"
#include "fancy_core/TempResourcePool.h"
#include "fancy_core/Window.h"

using namespace Fancy;

Test_Raytracing::Test_Raytracing(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
    : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Raytracing")
{
  // Create bottom level BVH
  eastl::vector<glm::float3> vertices = {
    {  1.0f,  1.0f, 0.0f },
    { -1.0f,  1.0f, 0.0f },
    {  0.0f, -1.0f, 0.0f }
  };

  // Setup identity transform matrix
  glm::mat3x4 transformMatrix = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f
  };

  // Setup indices
  eastl::vector<uint32_t> indices = { 0, 1, 2 };
  uint indexCount = static_cast<uint32_t>(indices.size());

  RtAccelerationStructureGeometryData geometryData;
  geometryData.myType = RtAccelerationStructureGeometryType::TRIANGLES;
  geometryData.myFlags = (uint)RtAccelerationStructureGeometryFlags::OPAQUE_GEOMETRY;
  geometryData.myVertexFormat = DataFormat::RGB_32F;
  geometryData.myNumVertices = (uint)vertices.size();
  geometryData.myVertexData.myType = RT_BUFFER_DATA_TYPE_CPU_DATA;
  geometryData.myVertexData.myCpuData.myData = vertices.data();
  geometryData.myVertexData.myCpuData.myDataSize = VECTOR_BYTESIZE(vertices);
  geometryData.myIndexFormat = DataFormat::R_32UI;
  geometryData.myNumIndices = (uint)indices.size();
  geometryData.myIndexData.myType = RT_BUFFER_DATA_TYPE_CPU_DATA;
  geometryData.myIndexData.myCpuData.myData = indices.data();
  geometryData.myIndexData.myCpuData.myDataSize = VECTOR_BYTESIZE(indices);
  geometryData.myTransformData.myType = RT_BUFFER_DATA_TYPE_CPU_DATA;
  geometryData.myTransformData.myCpuData.myData = &transformMatrix;
  geometryData.myTransformData.myCpuData.myDataSize = sizeof(transformMatrix);
  myBLAS = RenderCore::CreateRtBottomLevelAccelerationStructure(&geometryData, 1u, (uint)RtAccelerationStructureFlags::ALLOW_UPDATE);

  RtAccelerationStructureInstanceData instanceData;
  instanceData.myInstanceId = 0;
  instanceData.mySbtHitGroupOffset = 0;
  instanceData.myInstanceBLAS = myBLAS;
  myTLAS = RenderCore::CreateRtTopLevelAccelerationStructure(&instanceData, 1u, (uint)RtAccelerationStructureFlags::ALLOW_UPDATE);

  RtPipelineStateProperties rtPipelineProps;
  const uint raygenIdx = rtPipelineProps.AddRayGenShader("RayTracing/RayGen.hlsl", "RayGen");
  const uint missIdx = rtPipelineProps.AddMissShader("RayTracing/Miss.hlsl", "Miss");
  const uint hitIdx = rtPipelineProps.AddHitGroup(L"HitGroup0", RT_HIT_GROUP_TYPE_TRIANGLES, nullptr, nullptr, nullptr, nullptr, "RayTracing/Hit.hlsl", "ClosestHit");
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

  ctx->TransitionShaderResource(rtOutputTex.myWriteView, ShaderResourceTransition::TO_SHADER_WRITE);

  struct Consts
  {
    uint myOutTexIndex;
  } consts;
  consts.myOutTexIndex = rtOutputTex.myWriteView->GetGlobalDescriptorIndex();
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
