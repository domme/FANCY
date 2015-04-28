#ifndef INCLUDE_SERIALIZABLE_H
#define INCLUDE_SERIALIZABLE_H

#include "ObjectName.h"

namespace Fancy {
  namespace IO {
    class SerializerBinary;
  }
}

namespace Fancy {
//---------------------------------------------------------------------------//
  class Serializable
  {
  public:
    virtual ~Serializable() {}

    virtual ObjectName getTypeName() = 0;
    virtual bool serialize(IO::SerializerBinary* aSerializer) = 0;
  };
//---------------------------------------------------------------------------//
}

#endif  // INCLUDE_SERIALIZABLE_H