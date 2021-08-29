#include "Test_Raytracing.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_core/GpuBufferProperties.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>

#include "fancy_core/CommandList.h"
#include "fancy_core/RaytracingBVH.h"
#include "fancy_core/RaytracingPipelineState.h"
#include "fancy_core/RaytracingShaderTable.h"
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
  TextureProperties texProps;
  texProps.myIsShaderWritable = true;
  texProps.myIsRenderTarget = false;
  texProps.myFormat = DataFormat::RGBA_8;
  texProps.myWidth = myWindow->GetWidth();
  texProps.myHeight = myWindow->GetHeight();
  TempTextureResource rtOutputTex = RenderCore::AllocateTempTexture(texProps, 0u, "RT Test Result Texture");

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
  props.myBindFlags = (uint)GpuBufferBindFlags::RAYTRACING_BVH_BUILD_INPUT;
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

  RaytracingBVHGeometry rtAsGeometry;
  rtAsGeometry.myVertexBuffer = myRTvertexBuffer.get();
  rtAsGeometry.myNumVertices = (uint) vertices.size();
  rtAsGeometry.myVertexFormat = DataFormat::RGB_32F;
  rtAsGeometry.myIndexBuffer = myRTindexBuffer.get();
  rtAsGeometry.myIndexFormat = DataFormat::R_32UI;
  rtAsGeometry.myNumIndices = indexCount;
  rtAsGeometry.myTransformBuffer = myRTtransformBuffer.get();
  rtAsGeometry.myType = RaytracingBVHGeometryType::TRIANGLES;
  rtAsGeometry.myFlags = (uint)RaytracingBVHGeometryFlags::OPAQUE_GEOMETRY;

  RaytracingBVHProps bvhProps;
  bvhProps.myFlags = (uint)RaytracingBVHFlags::ALLOW_UPDATE;
  bvhProps.myType = RaytracingBVHType::BOTTOM_LEVEL;
  myBottomLevelBVH = RenderCore::CreateRtAccelerationStructure(bvhProps, { &rtAsGeometry, 1u }, "Test_Raytracing Bottom-level BVH");*/

  

  RaytracingPipelineStateProperties rtPipelineProps;
  const uint raygenIdx = rtPipelineProps.AddRayGenShader("RayTracing/RayGen.hlsl", "RayGen");
  rtPipelineProps.SetMaxAttributeSize(32u);
  rtPipelineProps.SetMaxPayloadSize(128u);
  rtPipelineProps.SetMaxRecursionDepth(2u);
  // const uint missIdx = rtPipelineProps.AddMissShader("RayTracing/Miss.hlsl", "Miss");
  // const uint hitIdx = rtPipelineProps.AddHitGroup(L"HitGroup0", RT_HIT_GROUP_TYPE_TRIANGLES, nullptr, nullptr, nullptr, nullptr, "RayTracing/Hit.hlsl", "ClosestHit");
  myRtPso = RenderCore::CreateRtPipelineState(rtPipelineProps);

  RaytracingShaderTableProperties sbtProps;
  // sbtProps.myNumMissShaderRecords = 5;
  // sbtProps.myNumHitShaderRecords = 5;
  sbtProps.myNumRaygenShaderRecords = 1;
  myShaderTable = RenderCore::CreateRtShaderTable(sbtProps);

  struct Constants
  {
    uint myOutTexIndex;
  } consts;
  consts.myOutTexIndex = 0;

  myShaderTable->AddShaderRecord(myRtPso->GetRayGenShaderIdentifier(raygenIdx), &consts, sizeof(consts));
  // myShaderTable->AddShaderRecord(myRtPso->GetMissShaderIdentifier(missIdx));
  // myShaderTable->AddShaderRecord(myRtPso->GetHitShaderIdentifier(hitIdx));


}

void Test_Raytracing::OnWindowResized(uint aWidth, uint aHeight)
{
}

void Test_Raytracing::OnUpdate(bool aDrawProperties)
{

}

void Test_Raytracing::OnRender()
{
  

  CommandList* ctx = RenderCore::BeginCommandList(CommandListType::Graphics);
  ctx->SetRaytracingPipelineState(myRtPso.get());

  ctx->TransitionShaderResource(rtOutputTex.myWriteView, ShaderResourceTransition::TO_SHADER_WRITE);
  
  DispatchRaysDesc desc;
  desc.myRayGenShaderTableRange = myShaderTable->GetRayGenRange();
  // desc.myMissShaderTableRange = myShaderTable->GetMissRange();
  // desc.myHitGroupTableRange = myShaderTable->GetHitRange();
  desc.myWidth = props.myTextureProperties.myWidth;
  desc.myHeight = props.myTextureProperties.myHeight;
  desc.myDepth = 1;
  ctx->DispatchRays(desc);

  ctx->ResourceUAVbarrier(rtOutputTex.myTexture);
  SubresourceLocation subresourceLoc;
  TextureRegion region = { glm::uvec3(0), glm::uvec3(props.myTextureProperties.myWidth, props.myTextureProperties.myHeight, 1u) };

  Texture* backbuffer = myOutput->GetBackbuffer();
  ctx->CopyTexture(backbuffer, subresourceLoc, region, rtOutputTex.myTexture, subresourceLoc, region);

  RenderCore::ExecuteAndFreeCommandList(ctx, SyncMode::BLOCKING);
}
