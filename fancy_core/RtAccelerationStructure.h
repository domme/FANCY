#pragma once

namespace Fancy
{
//---------------------------------------------------------------------------//
  enum RtBufferDataType
  {
    RT_BUFFER_DATA_TYPE_NONE = 0,
    RT_BUFFER_DATA_TYPE_GPU_BUFFER,
    RT_BUFFER_DATA_TYPE_CPU_DATA,
  };
//---------------------------------------------------------------------------//
  struct RtBufferData
  {
    struct BufferData
    {
      const GpuBuffer* myBuffer;
      uint64 myOffsetBytes;
      uint64 mySizeBytes;
    };
    struct CpuData
    {
      const uint8* myData;
      uint64 myDataSize;
    };

    RtBufferDataType myType = RT_BUFFER_DATA_TYPE_NONE;

    union
    {
      BufferData myBuffer;
      CpuData myCpuData;
    };
  };
//---------------------------------------------------------------------------//
  struct RtAccelerationStructureGeometryData
  {
    RtBufferData myVertexData;
    RtBufferData myIndexData;
    RtBufferData myTransformData;
    RtBufferData myProcedural_AABBData;
    DataFormat myVertexFormat = DataFormat::RGB_32F;
    DataFormat myIndexFormat = DataFormat::R_32UI;
    uint myNumVertices = 0;
    uint myNumIndices = 0;
    uint myFlags = 0; // RaytracingBVHGeometryFlags
    uint myProcedural_NumAABBs = 0;
    RtAccelerationStructureGeometryType myType = RtAccelerationStructureGeometryType::TRIANGLES;
  };
//---------------------------------------------------------------------------//
  struct RtAccelerationStructureInstanceData
  {
    
  };
//---------------------------------------------------------------------------//
  class RtAccelerationStructure
  {
  public:
    RtAccelerationStructure(const RtAccelerationStructureGeometryData* someGeometries, uint aNumGeometries, uint aSomeFlags = 0, const char* aName = nullptr);
    RtAccelerationStructure(const RtAccelerationStructureInstanceData* someInstances, uint aNumInstances, uint someFlags = 0, const char* aName = nullptr);
    virtual ~RtAccelerationStructure() = default;
    
  protected:
    SharedPtr<GpuBuffer> myBuffer;
    SharedPtr<GpuBufferView> myBufferRead;
    RtAccelerationStructureType myType;
  };
//---------------------------------------------------------------------------//
}



