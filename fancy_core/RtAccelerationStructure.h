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
    uint myFlags = 0; // RtAccelerationStructureGeometryFlags
    uint myProcedural_NumAABBs = 0;
    RtAccelerationStructureGeometryType myType = RtAccelerationStructureGeometryType::TRIANGLES;
  };
//---------------------------------------------------------------------------//
  struct RtAccelerationStructureInstanceData
  {
    glm::mat3x4 myTransform;
    uint myInstanceId : 24;
    uint8 myInstanceMask;
    uint mySbtHitGroupOffset : 24;
    uint8 myFlags;
    SharedPtr<RtAccelerationStructure> myInstanceBLAS;
  };
//---------------------------------------------------------------------------//
  class RtAccelerationStructure
  {
  public:
    RtAccelerationStructure(RtAccelerationStructureType aType, const char* aName);
    virtual ~RtAccelerationStructure() = default;

    RtAccelerationStructureType GetType() const { return myType; }
    GpuBuffer* GetBuffer() const { return myBuffer.get(); }
    GpuBufferView* GetBufferRead() const { myBufferRead.get(); }
    
  protected:
    eastl::fixed_string<char, 32> myName;
    SharedPtr<GpuBuffer> myBuffer;
    SharedPtr<GpuBufferView> myBufferRead;
    RtAccelerationStructureType myType;
  };
//---------------------------------------------------------------------------//
}



