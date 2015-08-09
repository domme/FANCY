#include "Serializer.h"

namespace Fancy { namespace IO {
//---------------------------------------------------------------------------//
  Serializer::Serializer(ESerializationMode aMode) :
    myMode(aMode) 
  {
    
  }
//---------------------------------------------------------------------------//
  Serializer::~Serializer()
  {
    if (myArchive.good())
    {
      myArchive.close();
    }
  }  
//---------------------------------------------------------------------------//

} }  // end of namespace Fancy::IO
