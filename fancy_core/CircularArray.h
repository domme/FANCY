#pragma once

#include "FancyCoreDefines.h"
#include "Log.h"

namespace Fancy
{
//---------------------------------------------------------------------------//
  template <class T>
  class CircularArray
  {
  public:
    struct Handle
    {
      void operator=(const Handle& anOther) { myIndex = anOther.myIndex; }

      bool operator==(const Handle& anOther) const { return myIndex == anOther.myIndex; }
      bool operator!=(const Handle& anOther) const { return myIndex != anOther.myIndex; }
      
      bool operator==(uint anIndex) const { return myIndex == anIndex; }
      bool operator!=(uint anIndex) const { return myIndex != anIndex; }

      const T* Get(CircularArray<T>& anArray) const { return myIndex < anArray.myCapacity ? &anArray.myBuffer[myIndex] : nullptr; }

      uint myIndex = UINT_MAX;
    };

    CircularArray(uint aCapacity)
      : myBuffer(new T[aCapacity])
      , myHead(0u)
      , myTail(0u)
      , mySize(0u)
      , myCapacity(aCapacity)
      , myHasBufferOnHeap(true)
    {
    }

    ~CircularArray()
    {
      if (myHasBufferOnHeap)
        delete[] myBuffer;
    }

    bool IsFull() const { return mySize == myCapacity; }
    bool IsEmpty() const { return mySize == 0u; }
    uint Capacity() const { return myCapacity; }
    uint Size() const { return mySize; }
    T* GetBuffer() { return myBuffer; }

    uint GetBufferIndex(uint anElement) const { ASSERT(anElement < Size()); return (myHead + anElement) % myCapacity; }
    Handle GetHandle(uint anElement) { return { GetBufferIndex(anElement) }; }

    T& operator[](uint anElement) { ASSERT(anElement < Size()); return myBuffer[(myHead + anElement) % myCapacity]; }
    const T& operator[](uint anElement) const { ASSERT(anElement < Size()); return myBuffer[(myHead + anElement) % myCapacity]; }

    T& operator[](Handle aHandle) { ASSERT(aHandle.myIndex < myCapacity); return myBuffer[aHandle.myIndex]; }
    const T& operator[](Handle aHandle) const { ASSERT(aHandle.myIndex < myCapacity); return myBuffer[aHandle.myIndex]; }

    void Add(T aVal)
    {
      ASSERT(!IsFull());
      const uint tail = myTail;
      myTail = myTail == (myCapacity - 1u) ? 0 : myTail + 1u;
      ++mySize;
      myBuffer[tail] = std::move(aVal);
    }

    void RemoveLastElement()
    {
      ASSERT(!IsEmpty());
      --mySize;
      myTail = myTail == 0 ? (myCapacity - 1u) : myTail - 1u;
    }

    void RemoveFirstElement()
    {
      ASSERT(!IsEmpty());
      --mySize;
      myHead = myHead == myCapacity - 1u ? 0 : myHead + 1u;
    }

  protected:
    CircularArray(uint aCapacity, T* aBuffer)
      : myBuffer(aBuffer)
      , myHead(0u)
      , myTail(0u)
      , mySize(0u)
      , myCapacity(aCapacity)
      , myHasBufferOnHeap(false)
    {

    }

    T* myBuffer;
    uint myHead;
    uint myTail;
    uint mySize;
    const uint myCapacity;
    const bool myHasBufferOnHeap;
  };
//---------------------------------------------------------------------------//
  template <class T, uint N>
  class StaticCircularArray final : public CircularArray<T>
  {
  public:
    StaticCircularArray()
      : CircularArray<T>(N, myArray)
    {
    }

  private:
    T myArray[N];
  };
//---------------------------------------------------------------------------//
}