#pragma once


namespace Fancy { 
  namespace IO {
  class Serializer;
  }

//---------------------------------------------------------------------------//
  struct ResourceDesc
  {
    virtual ~ResourceDesc() {}

    virtual ObjectName GetTypeName() const = 0;
    virtual uint64 GetHash() const = 0;
    virtual void Serialize(IO::Serializer* aSerializer) = 0;
  };
  //---------------------------------------------------------------------------//
}