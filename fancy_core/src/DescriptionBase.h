#pragma once

namespace Fancy {
//---------------------------------------------------------------------------// 
 namespace IO {
  class Serializer;
}
//---------------------------------------------------------------------------//
  struct DescriptionBase
  {
    virtual ~DescriptionBase() {}

    virtual ObjectName GetTypeName() const = 0;
    virtual uint64 GetHash() const = 0;
    virtual void Serialize(IO::Serializer* aSerializer) = 0;

    virtual bool IsEmpty() const { return false; }
  };
//---------------------------------------------------------------------------//
}