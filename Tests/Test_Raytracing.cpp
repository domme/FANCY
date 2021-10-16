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
  /*// Create bottom level BVH
  struct Vertex {
    float pos[3];
  };
  eastl::vector<Vertex> vertices = {
    { {  1.0f,  1.0f, 0.0f } },
    { { -1.0f,  1.0f, 0.0f } },
    { {  0.0f, -1.0f, 0.0f } }
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

  GpuBufferProperties props;
  props.myBindFlags = (uint)GpuBufferBindFlags::RT_ACCELERATION_STRUCTURE_BUILD_INPUT;
  props.myCpuAccess = CpuMemoryAccessType::CPU_WRITE;
  props.myElementSizeBytes = sizeof(Vertex);
  props.myNumElements = (uint)vertices.size();
  props.myIsShaderWritable = false;
  myRTvertexBuffer = RenderCore::CreateBuffer(props, "RT vertex buffer", vertices.data());

  props.myElementSizeBytes = sizeof(uint32_t);
  props.myNumElements = (uint)indices.size();
  myRTindexBuffer = RenderCore::CreateBuffer(props, "RT index buffer", indices.data());

  props.myElementSizeBytes = sizeof(glm::mat3x4);
  props.myNumElements = 1;
  myRTtransformBuffer = RenderCore::CreateBuffer(props, "RT transform buffer", &transformMatrix);

  RaytracingAsGeometryInfo rtAsGeometry;
  rtAsGeometry.myVertexBuffer = myRTvertexBuffer.get();
  rtAsGeometry.myNumVertices = (uint) vertices.size();
  rtAsGeometry.myVertexFormat = DataFormat::RGB_32F;
  rtAsGeometry.myIndexBuffer = myRTindexBuffer.get();
  rtAsGeometry.myIndexFormat = DataFormat::R_32UI;
  rtAsGeometry.myNumIndices = indexCount;
  rtAsGeometry.myTransformBuffer = myRTtransformBuffer.get();
  rtAsGeometry.myType = RtAccelerationStructureGeometryType::TRIANGLES;
  rtAsGeometry.myFlags = (uint)RtAccelerationStructureGeometryFlags::OPAQUE_GEOMETRY;

  RaytracingAsProps bvhProps;
  bvhProps.myFlags = (uint)RtAccelerationStructureFlags::ALLOW_UPDATE;
  bvhProps.myType = RtAccelerationStructureType::BOTTOM_LEVEL;
  myBottomLevelBVH = RenderCore::CreateRtAccelerationStructure(bvhProps, { &rtAsGeometry, 1u }, "Test_Raytracing Bottom-level BVH");*/

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
