#include "fancy_core_precompile.h"
#include "TempResourcePool.h"

#include "Common/MathUtil.h"
#include "RenderCore.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  namespace Priv_TempResourcePool {
    uint64 GetHash( const TextureResourceProperties & someProps, uint someFlags ) {
      uint64 hash = 0u;
      MathUtil::hash_combine( hash, static_cast< uint >( someProps.myTextureProperties.myDimension ) );
      MathUtil::hash_combine( hash, static_cast< uint >( someProps.myTextureProperties.myFormat ) );
      MathUtil::hash_combine( hash, someProps.myTextureProperties.IsArray() ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myTextureProperties.myNumMipLevels ? 1 : 0 );
      MathUtil::hash_combine( hash, static_cast< uint >( someProps.myTextureProperties.GetDepthSize() ) );
      MathUtil::hash_combine( hash, static_cast< uint >( someProps.myTextureProperties.myAccessType ) );
      MathUtil::hash_combine( hash, someProps.myIsRenderTarget ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myIsTexture ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myIsShaderWritable ? 1 : 0 );
      return hash;
    }

    uint64 GetHash( const GpuBufferResourceProperties & someProps, uint someFlags ) {
      uint64 hash = 0u;
      MathUtil::hash_combine( hash, static_cast< uint >( someProps.myFormat ) );
      MathUtil::hash_combine( hash, someProps.myStructureSize );
      MathUtil::hash_combine( hash, someProps.myIsStructured ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myIsRaw ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myIsShaderResource ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myIsShaderWritable ? 1 : 0 );
      MathUtil::hash_combine( hash, someProps.myBufferProperties.myCpuAccess );
      MathUtil::hash_combine( hash, someProps.myBufferProperties.myBindFlags );
      return hash;
    }
  }  // namespace Priv_TempResourcePool
  //---------------------------------------------------------------------------//
  TempResourcePool::~TempResourcePool() {
    ASSERT( myNumOpenBufferAllocs == 0, "%d open temp buffer allocs when destroying the temp resource pool",
            myNumOpenBufferAllocs );
    ASSERT( myNumOpenTextureAllocs == 0, "%d open temp texture allocs when destroying the temp resource pool",
            myNumOpenTextureAllocs );
  }
  //---------------------------------------------------------------------------//
  void TempResourcePool::Reset() {
    ASSERT( myNumOpenBufferAllocs == 0, "%d open temp buffer allocs accross frame boundary", myNumOpenBufferAllocs );
    ASSERT( myNumOpenTextureAllocs == 0, "%d open temp texture allocs accross frame boundary", myNumOpenTextureAllocs );
  }
  //---------------------------------------------------------------------------//
  TempTextureResource TempResourcePool::AllocateTexture( const TextureResourceProperties & someProps, uint someFlags,
                                                         const char * aName ) {
    const uint64 key = Priv_TempResourcePool::GetHash( someProps, someFlags );

    std::list< TextureResource * > & availableList = myAvailableTextureBuckets[ key ];
    if ( !availableList.empty() ) {
      auto it = std::find_if( availableList.begin(), availableList.end(), [ & ]( const TextureResource * aResource ) {
        const TextureProperties & texProps = RenderCore::GetTexture( aResource->myTexture )->GetProperties();

        if ( someFlags & FORCE_SIZE )
          return texProps.myWidth == someProps.myTextureProperties.myWidth &&
                 texProps.myHeight == someProps.myTextureProperties.myHeight &&
                 texProps.myDepthOrArraySize == someProps.myTextureProperties.myDepthOrArraySize;

        return texProps.myWidth >= someProps.myTextureProperties.myWidth &&
               texProps.myHeight >= someProps.myTextureProperties.myHeight &&
               texProps.myDepthOrArraySize >= someProps.myTextureProperties.myDepthOrArraySize;
      } );

      if ( it != availableList.end() ) {
        TextureResource * res = ( *it );
        availableList.erase( it );

        TempTextureResource returnRes;
        returnRes.myTexture = RenderCore::GetTexture( res->myTexture );
        returnRes.myReadView = res->myReadView.IsValid() ? RenderCore::GetTextureView( res->myReadView ) : nullptr;
        returnRes.myWriteView = res->myWriteView.IsValid() ? RenderCore::GetTextureView( res->myWriteView ) : nullptr;
        returnRes.myRenderTargetView =
            res->myRenderTargetView.IsValid() ? RenderCore::GetTextureView( res->myRenderTargetView ) : nullptr;
        returnRes.myKeepAlive.reset( new TempResourceKeepAlive( this, returnRes.myTexture, key ) );
        ++myNumOpenTextureAllocs;
        return returnRes;
      }
    }

    // Create resource
    TextureResource res;
    res.Update( someProps, aName );
    myTexturePool[ RenderCore::GetTexture( res.myTexture ) ] = res;

    TempTextureResource returnRes;
    returnRes.myTexture = RenderCore::GetTexture( res.myTexture );
    returnRes.myReadView = res.myReadView.IsValid() ? RenderCore::GetTextureView( res.myReadView ) : nullptr;
    returnRes.myWriteView = res.myWriteView.IsValid() ? RenderCore::GetTextureView( res.myWriteView ) : nullptr;
    returnRes.myRenderTargetView =
        res.myRenderTargetView.IsValid() ? RenderCore::GetTextureView( res.myRenderTargetView ) : nullptr;
    returnRes.myKeepAlive.reset( new TempResourceKeepAlive( this, returnRes.myTexture, key ) );
    ++myNumOpenTextureAllocs;
    return returnRes;
  }
  //---------------------------------------------------------------------------//
  TempBufferResource TempResourcePool::AllocateBuffer( const GpuBufferResourceProperties & someProps, uint someFlags,
                                                       const char * aName ) {
    const uint64 desiredSize =
        someProps.myBufferProperties.myNumElements * someProps.myBufferProperties.myElementSizeBytes;

    const uint64                       key = Priv_TempResourcePool::GetHash( someProps, someFlags );
    std::list< GpuBufferResource * > & availableList = myAvailableBufferBuckets[ key ];
    if ( !availableList.empty() ) {
      auto it =
          std::find_if( availableList.begin(), availableList.end(),
                        [ desiredSize, someFlags ]( const GpuBufferResource * aResource ) {
                          const uint64 currSize = RenderCore::GetBuffer( aResource->myBuffer )->GetByteSize();
                          return ( someFlags & FORCE_SIZE ) > 0 ? currSize == desiredSize : currSize >= desiredSize;
                        } );

      if ( it != availableList.end() ) {
        GpuBufferResource * res = ( *it );
        availableList.erase( it );

        TempBufferResource returnRes;
        returnRes.myBuffer = RenderCore::GetBuffer( res->myBuffer );
        returnRes.myReadView = res->myReadView.IsValid() ? RenderCore::GetBufferView( res->myReadView ) : nullptr;
        returnRes.myWriteView = res->myWriteView.IsValid() ? RenderCore::GetBufferView( res->myWriteView ) : nullptr;
        returnRes.myKeepAlive.reset( new TempResourceKeepAlive( this, returnRes.myBuffer, key ) );
        ++myNumOpenBufferAllocs;
        return returnRes;
      }
    }

    // Create resource
    GpuBufferResource res;
    res.Update( someProps, aName );
    myBufferPool[ RenderCore::GetBuffer( res.myBuffer ) ] = res;

    TempBufferResource returnRes;
    returnRes.myBuffer = RenderCore::GetBuffer( res.myBuffer );
    returnRes.myReadView = res.myReadView.IsValid() ? RenderCore::GetBufferView( res.myReadView ) : nullptr;
    returnRes.myWriteView = res.myWriteView.IsValid() ? RenderCore::GetBufferView( res.myWriteView ) : nullptr;
    returnRes.myKeepAlive.reset( new TempResourceKeepAlive( this, returnRes.myBuffer, key ) );
    ++myNumOpenBufferAllocs;
    return returnRes;
  }
  //---------------------------------------------------------------------------//
  void TempResourcePool::FreeResource( void * aResource, uint64 aBucketHash ) {
    GpuResource * resource = static_cast< GpuResource * >( aResource );
    if ( resource->myType == GpuResourceType::BUFFER ) {
      auto it = myBufferPool.find( static_cast< GpuBuffer * >( aResource ) );
      ASSERT( it != myBufferPool.end() );
      auto listIt = myAvailableBufferBuckets.find( aBucketHash );
      ASSERT( listIt != myAvailableBufferBuckets.end() );

      GpuBufferResource * res = &it->second;
#if FANCY_RENDERER_USE_VALIDATION
      ASSERT( std::find( listIt->second.begin(), listIt->second.end(), res ) == listIt->second.end() );
#endif
      listIt->second.push_back( res );

      ASSERT( myNumOpenBufferAllocs > 0 );
      --myNumOpenBufferAllocs;
    } else {
      auto it = myTexturePool.find( static_cast< Texture * >( aResource ) );
      ASSERT( it != myTexturePool.end() );
      auto listIt = myAvailableTextureBuckets.find( aBucketHash );
      ASSERT( listIt != myAvailableTextureBuckets.end() );

      TextureResource * res = &it->second;
#if FANCY_RENDERER_USE_VALIDATION
      ASSERT( std::find( listIt->second.begin(), listIt->second.end(), res ) == listIt->second.end() );
#endif
      listIt->second.push_back( res );

      ASSERT( myNumOpenTextureAllocs > 0 );
      --myNumOpenTextureAllocs;
    }
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
