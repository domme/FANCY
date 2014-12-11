#ifndef INCLUDE_FIXEDARRAY_H
#define INCLUDE_FIXEDARRAY_H

#include "FancyCorePrerequisites.h"

namespace Fancy {
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  class FixedArrayDatastore_Stack
  {
    public:
      FixedArrayDatastore_Stack();
      virtual ~FixedArrayDatastore_Stack();

    protected:
      T m_Array[capacity];
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Stack<T, capacity>::~FixedArrayDatastore_Stack()
  {

  }
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Stack<T, capacity>::FixedArrayDatastore_Stack()
  {

  }
//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  class FixedArrayDatastore_Heap
  {
  public:
    FixedArrayDatastore_Heap();
    virtual ~FixedArrayDatastore_Heap();

  protected:
    T* m_Array;
  };
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Heap<T, capacity>::FixedArrayDatastore_Heap()
  {
    m_Array = reinterpret_cast<T*>(FANCY_ALLOCATE(sizeof(T) * capacity, MemoryCategory::GENERAL));
    ASSERT(m_Array != nullptr);
  }
//---------------------------------------------------------------------------//
  template<class T, uint32 capacity>
  FixedArrayDatastore_Heap<T, capacity>::~FixedArrayDatastore_Heap()
  {
    ASSERT(m_Array != nullptr);
    FANCY_FREE(m_Array, MemoryCategory::GENERAL);
  }
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  //template<class T, uint32 u32Capacity, template<class, uint32> StorageType = FixedArrayDatastore_Stack>
  template<class T, uint32 u32Capacity>
  class FixedArray : FixedArrayDatastore_Stack<T, u32Capacity>
  {
    public:
      FixedArray();
      virtual ~FixedArray() {};
    //---------------------------------------------------------------------------//  
      uint32 capacity() const {return u32Capacity;}
      uint32 size() const {return m_u32Size;}
      bool empty() const {return m_u32Size > 0u;}
      void clear() {m_u32Size = 0u;}
      void resize(uint32 uNewSize) {ASSERT_M(uNewSize <= u32Capacity, "Array too small"); m_u32Size = uNewSize; }
      void push_back(const T& clElement) {ASSERT_M(m_u32Size < u32Capacity, "Array is full"); m_Array[m_u32Size++] = clElement;}
    //---------------------------------------------------------------------------//
      T& back() { ASSERT(m_u32Size > 0); return m_Array[m_u32Size - 1u]; }
      const T& back() const { ASSERT(m_u32Size > 0); return m_Array[m_u32Size - 1u]; }
    //---------------------------------------------------------------------------//
      T& operator[](const uint32 u32Index) {ASSERT(u32Index < m_u32Size); return m_Array[u32Index];}
      const T& operator[](const uint32 u32Index) const {ASSERT(u32Index < m_u32Size); return m_Array[u32Index];}
    //---------------------------------------------------------------------------//
    private:
      uint32 m_u32Size;
  };
//---------------------------------------------------------------------------//
  //template<class T, uint32 capacity, class StorageType /* = FixedArrayDatastore_Stack */>
  //FixedArray<T, capacity, StorageType>::FixedArray() :
  //  m_u32Size(0), 
  //  StorageType<T, capacity>()
  //{
  //
  //}
  template<class T, uint32 u32Capacity>
  FixedArray<T, u32Capacity>::FixedArray() :
    m_u32Size(0), 
    FixedArrayDatastore_Stack<T, u32Capacity>()
  {

  }
//---------------------------------------------------------------------------//
} // end of namespace Fancy


#endif  // INCLUDE_FIXEDARRAY_H
