#include "Test_Raytracing.h"
#include "fancy_core/GpuBuffer.h"
#include "fancy_core/GpuBufferProperties.h"

#include <EASTL/fixed_vector.h>
#include <EASTL/vector.h>

#include "fancy_core/RaytracingBVH.h"

using namespace Fancy;

Test_Raytracing::Test_Raytracing(Fancy::FancyRuntime* aRuntime, Fancy::Window* aWindow,
  Fancy::RenderOutput* aRenderOutput, Fancy::InputState* anInputState)
    : Test(aRuntime, aWindow, aRenderOutput, anInputState, "Raytracing")
{

  // Create bottom level BVH
  {
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

    myBottomLevelBVH = RenderCore::CreateRtAccelerationStructure(bvhProps, { &rtAsGeometry, 1u }, "Test_Raytracing Bottom-level BVH");
  }
}

Test_Raytracing::~Test_Raytracing()
{
}

void Test_Raytracing::OnWindowResized(uint aWidth, uint aHeight)
{
}

void Test_Raytracing::OnUpdate(bool aDrawProperties)
{
}

void Test_Raytracing::OnRender()
{
}
