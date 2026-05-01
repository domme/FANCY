#pragma once

#include "ResourceHandle.h"
#include "Debug/Log.h"
#include "EASTL/fixed_vector.h"
#include "EASTL/hash_map.h"
#include <type_traits>

namespace Fancy {
  //---------------------------------------------------------------------------//
  // NoCacheType -- sentinel used as the default DescType to indicate that a
  // ResourcePool has no descriptor cache.
  //---------------------------------------------------------------------------//
  struct NoCacheType {};

  //---------------------------------------------------------------------------//
  // ResourcePool<T, Cap, DescType>
  //
  // A handle-based pool with optional descriptor-keyed caching.
  //
  //---------------------------------------------------------------------------//

  template < typename T, uint Cap = 256, typename DescType = NoCacheType >
  class ResourcePool {
  public:
    using Handle = ResourceHandle< T >;
    using CacheMap = eastl::hash_map< uint64, Handle >;

  //---------------------------------------------------------------------------//
    // Add with explicit hash (cached pools only)
    Handle Add( T * aResource, uint64 aHash ) {
      static_assert( !std::is_same< DescType, NoCacheType >::value,
                     "Add(resource, hash) requires a cached pool with DescType != NoCacheType" );
      ASSERT( aResource != nullptr );
      ASSERT( aHash != 0ull, "Add() with cached pool requires explicit hash" );
      uint index;
      if ( !myFreeIndices.empty() ) {
        index = myFreeIndices.back();
        myFreeIndices.pop_back();
      } else {
        index = ( uint ) mySlots.size();
        ASSERT( index < Cap, "ResourcePool capacity exceeded for %s", typeid( T ).name() );
        mySlots.push_back( Slot{} );
      }
      Slot & slot = mySlots[ index ];
      slot.myResource = aResource;
      Handle handle;
      handle.myIndex = index;
      handle.myGeneration = slot.myGeneration;
      myCache[ aHash ] = handle;
      myHandleToHash[ index ] = aHash;
      return handle;
    }
  //---------------------------------------------------------------------------//
    // Add with descriptor (cached pools only)
    Handle Add( T * aResource, const DescType & aDesc ) {
      static_assert( !std::is_same< DescType, NoCacheType >::value,
                     "Add(resource, desc) requires a cached pool with DescType != NoCacheType" );
      return Add( aResource, DescType::Hash( aDesc ) );
    }
  //---------------------------------------------------------------------------//
    // Add without cache (non-cached pools only)
    Handle Add( T * aResource ) {
      static_assert( std::is_same< DescType, NoCacheType >::value,
                     "Add(resource) requires a non-cached pool with DescType == NoCacheType" );
      ASSERT( aResource != nullptr );
      uint index;
      if ( !myFreeIndices.empty() ) {
        index = myFreeIndices.back();
        myFreeIndices.pop_back();
      } else {
        index = ( uint ) mySlots.size();
        ASSERT( index < Cap, "ResourcePool capacity exceeded for %s", typeid( T ).name() );
        mySlots.push_back( Slot{} );
      }
      Slot & slot = mySlots[ index ];
      slot.myResource = aResource;
      Handle handle;
      handle.myIndex = index;
      handle.myGeneration = slot.myGeneration;
      return handle;
    }
  //---------------------------------------------------------------------------//
    T * Get( Handle aHandle ) const {
      ASSERT( IsValid( aHandle ) );
      return mySlots[ aHandle.myIndex ].myResource;
    }
  //---------------------------------------------------------------------------//
    // Get by descriptor (cached pools only)
    Handle Get( const DescType & aDesc ) const {
      static_assert( !std::is_same< DescType, NoCacheType >::value,
                     "Get(desc) requires a cached pool with DescType != NoCacheType" );
      auto it = myCache.find( DescType::Hash( aDesc ) );
      if ( it != myCache.end() )
        return it->second;
      return Handle{};
    }
  //---------------------------------------------------------------------------//
    bool IsValid( Handle aHandle ) const {
      if ( !aHandle.IsValid() )
        return false;
      if ( aHandle.myIndex >= ( uint ) mySlots.size() )
        return false;
      const Slot & slot = mySlots[ aHandle.myIndex ];
      return slot.myResource != nullptr && slot.myGeneration == aHandle.myGeneration;
    }
  //---------------------------------------------------------------------------//
    // Get const reference to cache (cached pools only)
    const CacheMap & GetCache() const {
      static_assert( !std::is_same< DescType, NoCacheType >::value,
                     "GetCache() requires a cached pool with DescType != NoCacheType" );
      return myCache;
    }
  //---------------------------------------------------------------------------//
    ~ResourcePool() = default;
  //---------------------------------------------------------------------------//
    void Delete( Handle aHandle ) {
      ASSERT( IsValid( aHandle ) );
      if constexpr ( !std::is_same_v< DescType, NoCacheType > ) {
        auto revIt = myHandleToHash.find( aHandle.myIndex );
        if ( revIt != myHandleToHash.end() ) {
          myCache.erase( revIt->second );
          myHandleToHash.erase( revIt );
        }
      }
      Slot & slot = mySlots[ aHandle.myIndex ];
      delete static_cast< T * >( slot.myResource );
      slot.myResource = nullptr;
      ++slot.myGeneration;
      myFreeIndices.push_back( aHandle.myIndex );
    }
  //---------------------------------------------------------------------------//
    void DeleteAll() {
      if constexpr ( !std::is_same_v< DescType, NoCacheType > ) {
        myCache.clear();
        myHandleToHash.clear();
      }
      for ( Slot & slot : mySlots ) {
        if ( slot.myResource != nullptr ) {
          delete static_cast< T * >( slot.myResource );
          slot.myResource = nullptr;
          ++slot.myGeneration;
        }
      }
      myFreeIndices.clear();
    }
  //---------------------------------------------------------------------------//
  private:
    struct Slot {
      T *  myResource   = nullptr;
      uint myGeneration = 0u;
    };

    eastl::fixed_vector< Slot, Cap > mySlots;
    eastl::fixed_vector< uint, Cap > myFreeIndices;
    // Cache members exist in all pools but are only used for cached pools.
    CacheMap myCache;                    // descriptor hash -> handle
    eastl::hash_map< uint, uint64 > myHandleToHash; // slot index -> hash (for cache invalidation)
  };
}  // namespace Fancy