#pragma once

#include "FancyCoreDefines.h"
#include "Log.h"

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

    StaticArray<T, N>& operator=(const StaticArray<T, N>& anOther)
    {
      for (uint i = 0u; i < mySize; ++i)
        myData[i].~T();

      mySize = anOther.mySize;
      memcpy(myData, anOther.myData, sizeof(T) * mySize);

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
      ASSERT((uint)i < mySize);
      return myData[i];
    }

    const T& operator[](int i ) const
    {
      ASSERT((uint)i < mySize);
      return myData[i];
    }

    T& Add() { return AddElement(); }

    void Add(const T& anElement)
    {
      T& newElement = AddElement();
      newElement = anElement;
    }

    void AddUnique(const T& anElement)
    {
      for (uint i = 0u; i < mySize; ++i)
        if (myData[i] == anElement)
          return;

      Add(anElement);
    }

    void Add(T&& anElement)
    {
      ASSERT(mySize < N);
      myData[mySize++] = std::move(anElement);
    }

    void GrowToSize(uint aNewSize)
    {
      ASSERT(aNewSize <= N);
      while (mySize < aNewSize)
        AddElement();
    }

    void Clear()
    {
      for (uint i = 0u; i < mySize; ++i)
        myData[i].~T();

      mySize = 0u;
    }

    T& GetLast()
    {
      ASSERT(!IsEmpty());
      return myData[mySize - 1];
    }

    const T& GetLast() const
    {
      ASSERT(!IsEmpty());
      return myData[mySize - 1];
    }

    T& GetFirst()
    {
      ASSERT(!IsEmpty());
      return myData[0];
    }

    const T& GetFirst() const
    {
      ASSERT(!IsEmpty());
      return myData[0];
    }

    /// Clears without calling destructors. Should only be used with POD-types
    void ClearDiscard()
    {
      mySize = 0u;
    }

    void RemoveLast()
    {
      ASSERT(!IsEmpty());
      myData[mySize - 1].~T();
      --mySize;
    }

    void RemoveCyclicAt(uint anIndex)
    {
      ASSERT(mySize > anIndex);
      if (anIndex == mySize - 1)
      {
        RemoveLast();
      }
      else
      {
        myData[anIndex].~T();
        myData[anIndex] = std::move(myData[mySize - 1]);
        --mySize;
      }
    }

    void RemoveAt(uint anIndex)
    {
      ASSERT(mySize > anIndex);
      if (anIndex == mySize - 1)
      {
        RemoveLast();
      }
      else
      {
        myData[anIndex].~T();
        for (uint i = anIndex; i < mySize - 1; ++i)
          myData[anIndex] = std::move(myData[anIndex + 1]);
      }
      --mySize;
    }

    bool IsEmpty() const { return mySize == 0u; }
    bool IsFull() const { return mySize == N; }
    uint Size() const { return mySize; }
    uint64 ByteSize() const { return sizeof(T) * mySize; }
    static uint Capacity() { return N; }
    T* GetBuffer() { return myData; }
    const T* GetBuffer() const { return myData; }

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