#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  template<class T, uint N>
  class StaticArray
  {
  public:
    StaticArray() = default;
    ~StaticArray() = default;

    StaticArray(const StaticArray<T, N>& anOther)
    {
      for (uint i = 0u; i < mySize; ++i)
        myData[i].~T();

      mySize = anOther.mySize;
      memcpy(myData, anOther.myData, sizeof(T) * mySize);
    }

    StaticArray(const StaticArray<T, N>&& anOther) noexcept
    {
      myData = std::move(anOther.myData);
      mySize = anOther.mySize;
    }

    StaticArray<T, N>& operator=(const StaticArray<T, N>& anOther)
    {
      for (uint i = 0u; i < mySize; ++i)
        myData[i].~T();

      mySize = anOther.mySize;
      memcpy(myData, anOther.myData, sizeof(T) * mySize);

      return *this;
    }

    StaticArray<T, N>& operator=(StaticArray<T, N>&& anOther) noexcept
    {
      myData = std::move(anOther.myData);
      mySize = anOther.mySize;
      return *this;
    }

    bool operator==(const StaticArray<T, N>& anOther) const
    {
      bool same = mySize == anOther.mySize;
      for (uint i = 0u; same && i < mySize; ++i)
        same &= myData[i] == anOther.myData[i];
      return same;
    }

    T& operator[](int i)
    {
      ASSERT(i < mySize);
      return myData[i];
    }

    const T& operator[](int i ) const
    {
      ASSERT(i < mySize);
      return myData[i];
    }

    T& Add() { return AddElement(); }

    void Add(const T& anElement)
    {
      T& newElement = AddElement();
      newElement = anElement;
    }

    void Add(T&& anElement)
    {
      ASSERT(mySize < N);
      myData[mySize++] = std::move(anElement);
    }

    void Clear()
    {
      for (uint i = 0u; i < mySize; ++i)
        myData[i].~T();

      mySize = 0u;
    }

    bool IsEmpty() const { return mySize == 0u; }
    bool IsFull() const { return mySize == N; }
    uint Size() const { return mySize; }
    static uint Capacity() { return N; }
    T* GetBuffer() const { return myData; }

  private:
    T& AddElement()
    {
      ASSERT(mySize < N);
      T* newElement = new (&myData[mySize++]) T;
      return *newElement;
    }

    T myData[N];
    uint mySize = 0u;
  };
//---------------------------------------------------------------------------//
}