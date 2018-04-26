#include "GeometryVertexLayout.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  GeometryVertexElement::GeometryVertexElement() :
    mySemanticIndex(0u),
    eSemantics(VertexSemantics::NONE),
    u32OffsetBytes(0u), u32SizeBytes(0u),
    eFormat(DataFormat::RGB_32F)
  {

  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GeometryVertexLayout::GeometryVertexLayout() 
    : myTopology(TopologyType::TRIANGLE_LIST)
    , myStride(0u)
  {

  }
//---------------------------------------------------------------------------//
  GeometryVertexLayout::~GeometryVertexLayout()
  {

  }
//---------------------------------------------------------------------------//
  void GeometryVertexLayout::addVertexElement( const GeometryVertexElement& clVertexElement )
  {
    ASSERT(clVertexElement.u32SizeBytes > 0, "Invalid vertex element");

    // Adjacency-check. Currently we require all vertex elements to be specified in order to
    // avoid a sorting-step afterwards
    ASSERT((myElements.size() == 0 && clVertexElement.u32OffsetBytes == 0) ||
              (myElements.size() > 0 && myElements.back().u32OffsetBytes
                + myElements.back().u32SizeBytes == clVertexElement.u32OffsetBytes), 
                "Vertex-elements must be added in order");

    myElements.push_back(clVertexElement);
    myStride += clVertexElement.u32SizeBytes;
  }
//---------------------------------------------------------------------------//
  void GeometryVertexLayout::AddVertexElement(VertexSemantics aSemantic, DataFormat aFormat, uint aSemanticIndex, const char* aName)
  {
    GeometryVertexElement elem;
    elem.name = aName;
    elem.eSemantics = aSemantic;
    elem.eFormat = aFormat;
    elem.mySemanticIndex = aSemanticIndex;
    
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(aFormat);
    elem.u32SizeBytes = formatInfo.mySizeBytes;

    elem.u32OffsetBytes = myStride;
    addVertexElement(elem);
  }
//---------------------------------------------------------------------------//
}