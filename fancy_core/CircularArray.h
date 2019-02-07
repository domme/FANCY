#pragma once

#include "FancyCoreDefines.h"

namespace Fancy
{
  template <class T>
  class CircularArray
  {
  public:
    CircularArray(uint aCapacity)
      : myBuffer(new T[aCapacity])
        , myHead(0u)
        , myTail(0u)
        , myCapacity(aCapacity)
    {
    }

    ~CircularArray()
    {
      delete[] myBuffer;
    }

    bool IsFull() const { return (myTail + 1u) % myCapacity == myHead; }
    bool IsEmpty() const { return myHead == myTail; }
    uint Capacity() const { return myCapacity; }
    T* GetBuffer() { return myBuffer; }

    uint Size() const
    {
      const uint tailUnwrapped = myTail < myHead ? myTail + myCapacity : myTail;
      return (tailUnwrapped - myHead) + 1u;
    }

    T& Add()
    {
      ASSERT(!IsFull());
      myTail = myTail == (myCapacity - 1u) ? 0 : myTail + 1u;
      return myBuffer[myTail];
    }

    void Add(T aVal)
    {
      T& newElement = Add();
      newElement = std::move(aVal);
    }

    void RemoveLastElement()
    {
      ASSERT(!IsEmpty());
      myTail = myTail == 0 ? (myCapacity - 1u) : myTail - 1u;
    }

    void RemoveFirstElement()
    {
      ASSERT(!IsEmpty());
      myHead = myHead == myCapacity - 1u ? 0 : myHead + 1u;
    }

  private:
    T* myBuffer;
    uint myHead;
    uint myTail;
    const uint myCapacity;
  };
}
