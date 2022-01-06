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
      const void* myData;
      uint64 myDataSize;
    };

    RtBufferDataType myType = RT_BUFFER_DATA_TYPE_NONE;

    union
    {
      BufferData myBuffer;
      CpuData myCpuData;
    };

    bool IsEmpty() const {
      return myType == RT_BUFFER_DATA_TYPE_NONE;
    }

    uint64 GetGpuBufferAddress(CommandList* aCommandList, uint anAlingment = 0) const;
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
    uint myVertexStride = 0; // Byte size between to consecutive position-elements in the vertex-data. Useful for interleaved vertex data. Can be left 0, in which case it defaults to the format size
    uint myNumIndices = 0;
    uint myFlags = 0; // RtAccelerationStructureGeometryFlags
    uint myProcedural_NumAABBs = 0;
    RtAccelerationStructureGeometryType myType = RtAccelerationStructureGeometryType::TRIANGLES;
  };
//---------------------------------------------------------------------------//
  struct RtAccelerationStructureInstanceData
  {
    RtAccelerationStructureInstanceData()
      : myTransform(glm::mat3x4()) // Initialized to identity in default ctor
      , myInstanceId(0)
      , myInstanceMask(UINT8_MAX)
      , mySbtHitGroupOffset(0)
      , myFlags(0)
    {}

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
    GpuBufferView* GetBufferRead() const { return myTopLevelBufferRead.get(); }
    
  protected:
    eastl::string myName;
    SharedPtr<GpuBuffer> myBuffer;
    SharedPtr<GpuBufferView> myTopLevelBufferRead;
    RtAccelerationStructureType myType;
  };
//---------------------------------------------------------------------------//
}



