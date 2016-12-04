#include "GeometryVertexLayout.h"
#include "Serializer.h"

namespace Fancy { namespace Rendering {
//---------------------------------------------------------------------------//
  GeometryVertexElement::GeometryVertexElement() :
    mySemanticIndex(0u),
    eSemantics(VertexSemantics::NONE),
    u32OffsetBytes(0u), u32SizeBytes(0u),
    eFormat(DataFormat::RGB_32F)
  {

  }
//---------------------------------------------------------------------------//
  void GeometryVertexElement::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&name, "myName");
    aSerializer->Serialize(&eSemantics, "mySemenatics");
    aSerializer->Serialize(&mySemanticIndex, "mySemanticIndex");
    aSerializer->Serialize(&u32OffsetBytes, "myOffsetBytes");
    aSerializer->Serialize(&u32SizeBytes, "mySizeBytes");
    aSerializer->Serialize(&eFormat, "myFormat");
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  GeometryVertexLayout::GeometryVertexLayout() :
    m_u32StrideBytes(0u)
  {

  }
//---------------------------------------------------------------------------//
  GeometryVertexLayout::~GeometryVertexLayout()
  {

  }
//---------------------------------------------------------------------------//
  void GeometryVertexLayout::Serialize(IO::Serializer* aSerializer)
  {
    aSerializer->Serialize(&m_u32StrideBytes, "m_u32StrideBytes");
    aSerializer->Serialize(&m_vVertexElements, "m_vVertexElements");
  }
//---------------------------------------------------------------------------//
  void GeometryVertexLayout::addVertexElement( const GeometryVertexElement& clVertexElement )
  {
    ASSERT(clVertexElement.u32SizeBytes > 0, "Invalid vertex element");

    // Adjacency-check. Currently we require all vertex elements to be specified in order to
    // avoid a sorting-step afterwards
    ASSERT((m_vVertexElements.size() == 0 && clVertexElement.u32OffsetBytes == 0) ||
              (m_vVertexElements.size() > 0 && m_vVertexElements.back().u32OffsetBytes
                + m_vVertexElements.back().u32SizeBytes == clVertexElement.u32OffsetBytes), 
                "Vertex-elements must be added in order");

    m_vVertexElements.push_back(clVertexElement);
    m_u32StrideBytes += clVertexElement.u32SizeBytes;
  }
//---------------------------------------------------------------------------//
} }  // end of namespace Fancy::Rendering