#pragma once

#include "FancyCoreDefines.h"
#include "MathUtil.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  /// Bump allocator over a fixed-capacity buffer inside the object. Doesn't construct or destruct anything.
  template < uint64 Capacity, uint64 BaseAlignment = 16u > class LinearAllocator {
  public:
    void * Allocate( uint64 aSize, uint64 anAlignment ) {
      ASSERT( anAlignment > 0u && ( anAlignment & ( anAlignment - 1u ) ) == 0u, "Alignment must be a power of two" );

      // Align the actual address, so alignments larger than BaseAlignment work as well
      const uint64 base = reinterpret_cast< uint64 >( myMemory );
      const uint64 alignedOffset = MathUtil::Align( base + myOffset, anAlignment ) - base;
      if ( alignedOffset + aSize > Capacity ) {
        return nullptr;
      }

      myOffset = alignedOffset + aSize;
      return myMemory + alignedOffset;
    }

    void Reset() {
      myOffset = 0u;
    }

    uint64 GetUsedSize() const {
      return myOffset;
    }

    static constexpr uint64 GetCapacity() {
      return Capacity;
    }

  private:
    alignas( BaseAlignment ) uint8 myMemory[ Capacity ];
    uint64 myOffset = 0u;
  };
  //---------------------------------------------------------------------------//
}  // namespace Fancy
