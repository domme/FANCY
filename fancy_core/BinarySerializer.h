#pragma once

#include "FancyCoreDefines.h"
#include "String.h"
#include "Log.h"

namespace Fancy
{
  class BinarySerializer;

  template<class T>
  struct GetSerializeFunc
  {
    void operator()(T& /*aVal*/, BinarySerializer& /*aSerializer*/)
    {
      ASSERT(false, "Base implementation of GetSerializeFunc should never be called");
    }
  };

  class BinarySerializer
  {
  public:
    enum SERIALIZE_MODE
    {
      READ, WRITE
    };

    BinarySerializer(const char* aPath, SERIALIZE_MODE aMode)
      : myMode(aMode)
      , myStream(std::fstream(aPath, std::ios::binary | (aMode == WRITE ? std::ios::out : std::ios::in)))
    { }

    bool IsGood() const { return myStream.good(); }
    bool IsReading() const { return myMode == READ; }
    bool IsWriting() const { return myMode == WRITE; }

    void Serialize(uint8* someData, uint64 aDataSize);

    template <class T>
    void Serialize(T& aVal);

    template <class T> void Serialize(eastl::vector<T>& aVal);
    template <class T, uint N> void Serialize(eastl::fixed_vector<T, N>& aVal);
    template <class T, uint N> void Serialize(T(&aVal)[N]);
    void Serialize(String& aString);
    void Serialize(eastl::string& aString);
    template <class T, uint N> void Serialize(eastl::fixed_string<T, N>& aVal);

    SERIALIZE_MODE myMode;
    std::fstream myStream;
  };
//---------------------------------------------------------------------------//
  inline void BinarySerializer::Serialize(uint8* someData, uint64 aDataSize)
  {
    if (IsReading())
      myStream.read((char*)someData, aDataSize);
    else
      myStream.write((const char*)someData, aDataSize);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void BinarySerializer::Serialize(T& aVal)
  {
    if (std::is_trivially_copyable<T>::value)
      Serialize((uint8*)&aVal, sizeof(T));
    else
      GetSerializeFunc<T>()(aVal, *this);
  }
//---------------------------------------------------------------------------//
  template <class T>
  void BinarySerializer::Serialize(eastl::vector<T>& aVal)
  {
    if (IsReading())
    {
      uint size = 0u;
      Serialize(size);
      aVal.resize(size);
    }
    else
    {
      uint size = static_cast<uint>(aVal.size());
      Serialize(size);
    }

    if (std::is_trivially_copyable<T>::value)
    {
      Serialize((uint8*)aVal.data(), aVal.size() * sizeof(T));
    }
    else
    {
      for (T& element : aVal)
        Serialize(element);
    }
  }
//---------------------------------------------------------------------------//
  template <class T, uint N>
  void BinarySerializer::Serialize(eastl::fixed_vector<T, N>& aVal)
  {
    if (IsReading())
    {
      uint size = 0u;
      Serialize(size);
      aVal.resize(size);
    }
    else
    {
      uint size = static_cast<uint>(aVal.size());
      Serialize(size);
    }

    if (std::is_trivially_copyable<T>::value)
    {
      Serialize((uint8*)aVal.data(), aVal.size() * sizeof(T));
    }
    else
    {
      for (T& element : aVal)
        Serialize(element);
    }
  }
//---------------------------------------------------------------------------//
  template <class T, uint N>
  void BinarySerializer::Serialize(T(& aVal)[N])
  {
    if (std::is_trivially_copyable<T>::value)
    {
      Serialize((uint8*)aVal, sizeof(T) * N);
    }
    else
    {
      for (T& element : aVal)
        Serialize(element);
    }
  }
//---------------------------------------------------------------------------//
  inline void BinarySerializer::Serialize(String& aString)
  {
    if (IsReading())
    {
      uint name_size;
      Serialize(name_size);

      const uint kExpectedMaxLength = 64u;
      char buf[kExpectedMaxLength];
      char* name_cstr = buf;

      if (name_size > kExpectedMaxLength)
        name_cstr = new char[name_size];

      Serialize((uint8*)name_cstr, name_size);
      aString = name_cstr;

      if (name_size > kExpectedMaxLength)
        delete[] name_cstr;
    }
    else
    {
      const char* name_cstr = aString.c_str();
      const uint name_size = static_cast<uint>(aString.size()) + 1u; // size + '/0'
      Serialize(name_size);
      Serialize((uint8*)name_cstr, name_size);
    }
  }
//---------------------------------------------------------------------------//
  inline void BinarySerializer::Serialize(eastl::string& aString)
  {
    if (IsReading())
    {
      uint name_size;
      Serialize(name_size);

      const uint kExpectedMaxLength = 64u;
      char buf[kExpectedMaxLength];
      char* name_cstr = buf;

      if (name_size > kExpectedMaxLength)
        name_cstr = new char[name_size];

      Serialize((uint8*)name_cstr, name_size);
      aString = name_cstr;

      if (name_size > kExpectedMaxLength)
        delete[] name_cstr;
    }
    else
    {
      const char* name_cstr = aString.c_str();
      const uint name_size = static_cast<uint>(aString.size()) + 1u; // size + '/0'
      Serialize(name_size);
      Serialize((uint8*)name_cstr, name_size);
    }
  }
//---------------------------------------------------------------------------//
  template <class T, uint N>
  void BinarySerializer::Serialize(eastl::fixed_string<T, N>& aString)
  {
    if (IsReading())
    {
      uint name_size;
      Serialize(name_size);

      const uint kExpectedMaxLength = 64u;
      char buf[kExpectedMaxLength];
      char* name_cstr = buf;

      if (name_size > kExpectedMaxLength)
        name_cstr = new char[name_size];

      Serialize((uint8*)name_cstr, name_size);
      aString = name_cstr;

      if (name_size > kExpectedMaxLength)
        delete[] name_cstr;
    }
    else
    {
      const char* name_cstr = aString.c_str();
      const uint name_size = static_cast<uint>(aString.size()) + 1u; // size + '/0'
      Serialize(name_size);
      Serialize((uint8*)name_cstr, name_size);
    }
  }
//---------------------------------------------------------------------------//
}