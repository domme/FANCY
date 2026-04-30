#pragma once

#include "ResourceHandle.h"
#include "Debug/Log.h"
#include "EASTL/fixed_vector.h"

namespace Fancy {
  // ResourcePool uses a fixed-size allocation strategy for predictable memory
  // and better cache locality. StaticCapacity should be set to the expected
  // maximum number of resources of this type.
  template < typename T, uint StaticCapacity = 256 > class ResourcePool {
  public:
    ResourceHandle< T > Add( T * aResource ) {
      ASSERT( aResource != nullptr );
      uint index;
      if ( !myFreeIndices.empty() ) {
        index = myFreeIndices.back();
        myFreeIndices.pop_back();
      } else {
        index = ( uint ) mySlots.size();
        ASSERT( index < StaticCapacity, "ResourcePool capacity exceeded for %s", typeid( T ).name() );
        mySlots.push_back( Slot{} );
      }
      Slot & slot = mySlots[ index ];
      slot.myResource = aResource;
      ResourceHandle< T > handle;
      handle.myIndex = index;
      handle.myGeneration = slot.myGeneration;
      return handle;
    }

    T * Get( ResourceHandle< T > aHandle ) const {
      ASSERT( IsValid( aHandle ) );
      return mySlots[ aHandle.myIndex ].myResource;
    }

    bool IsValid( ResourceHandle< T > aHandle ) const {
      if ( !aHandle.IsValid() )
        return false;
      if ( aHandle.myIndex >= ( uint ) mySlots.size() )
        return false;
      const Slot & slot = mySlots[ aHandle.myIndex ];
      return slot.myResource != nullptr && slot.myGeneration == aHandle.myGeneration;
    }

    void Delete( ResourceHandle< T > aHandle ) {
      ASSERT( IsValid( aHandle ) );
      Slot & slot = mySlots[ aHandle.myIndex ];
      delete static_cast< T * >( slot.myResource );
      slot.myResource = nullptr;
      ++slot.myGeneration;
      myFreeIndices.push_back( aHandle.myIndex );
    }

    void DeleteAll() {
      for ( Slot & slot : mySlots ) {
        if ( slot.myResource != nullptr ) {
          delete static_cast< T * >( slot.myResource );
          slot.myResource = nullptr;
          ++slot.myGeneration;
        }
      }
      myFreeIndices.clear();
    }

  private:
    struct Slot {
      T * myResource = nullptr;
      uint myGeneration = 0u;
    };

    eastl::fixed_vector< Slot, StaticCapacity > mySlots;
    eastl::fixed_vector< uint, StaticCapacity > myFreeIndices;
  };
}  // namespace Fancy
