#include "GpuMemoryViewer.h"

#include "imgui.h"
#include "Common/PagedLinearAllocator.h"
#include "Common/StaticString.h"
#include "Common/StringUtil.h"
#include "Rendering/RenderCore.h"
#include "Rendering/RenderEnums.h"

#if FANCY_ENABLE_DX12
#  include "Rendering/DX12/RenderCore_PlatformDX12.h"
#endif

namespace Fancy {
  //---------------------------------------------------------------------------//
  static const char * locMemoryTypeToString( GpuMemoryType aType ) {
    switch ( aType ) {
      case GpuMemoryType::BUFFER:
        return "buffer";
      case GpuMemoryType::TEXTURE:
        return "texture";
      case GpuMemoryType::RENDERTARGET:
        return "rendertarget";
      default:
        return "";
    }
  }

  static const char * locCpuAccessTypeToString( CpuMemoryAccessType aType ) {
    switch ( aType ) {
      case CpuMemoryAccessType::NO_CPU_ACCESS:
        return "default";
      case CpuMemoryAccessType::CPU_WRITE:
        return "write";
      case CpuMemoryAccessType::CPU_READ:
        return "read";
      default:
        return "";
    }
  }

  static const char * locGetMemoryLabel( uint64 aMemorySize, CircularStringBuffer & aStringPool ) {
    if ( aMemorySize >= SIZE_MB )
      return aStringPool.Format( "%d MiB", aMemorySize / SIZE_MB );
    if ( aMemorySize >= SIZE_KB )
      return aStringPool.Format( "%d KiB", aMemorySize / SIZE_KB );

    return aStringPool.Format( "%d Byte", aMemorySize );
  }

#if FANCY_ENABLE_DX12
  static void locDebugPrintMemoryAllocatorDx12( GpuMemoryAllocatorDX12 * anAllocatorDx12, float aMemoryToPixelScale ) {
    PagedLinearAllocator & allocator = *anAllocatorDx12;

    const float          elementHeight = 20.0f;
    CircularStringBuffer stringBuffer;

    const ImVec2 startPos = ImGui::GetCursorPos();
    ImVec2       pos = startPos;
    for ( uint i = 0u; i < allocator.myPages.size(); ++i ) {
      const auto & page = allocator.myPages[ i ];
      const float  pixelWidth = aMemoryToPixelScale * ( float ) ( page.myEnd - page.myStart );
      if ( pixelWidth >= 2 ) {
        ImGui::SetCursorPos( pos );
        ImGui::Button( stringBuffer.Format( "Heap %d", i ), ImVec2( pixelWidth - 1, elementHeight ) );
        if ( ImGui::IsItemHovered() ) {
          StaticString< 128 > str( "Start: %s\nEnd:%s\nSize:%s", locGetMemoryLabel( page.myStart, stringBuffer ),
                                   locGetMemoryLabel( page.myEnd, stringBuffer ),
                                   locGetMemoryLabel( page.myEnd - page.myStart, stringBuffer ) );
          ImGui::SetTooltip( str );
        }
      }
      pos.x += pixelWidth;
    }

    pos = startPos;
    pos.y += elementHeight + 2.0f;
    ImGui::PushStyleColor( ImGuiCol_Button, 0xFFAAAAAA );
    for ( auto freeBlockIt = allocator.myFreeList.Begin(); freeBlockIt != allocator.myFreeList.Invalid(); ++freeBlockIt ) {
      const uint64 freeMemory = freeBlockIt->myEnd - freeBlockIt->myStart;
      const float  pixelWidth = aMemoryToPixelScale * ( float ) freeMemory;
      if ( pixelWidth < 2 )
        continue;

      ImVec2 currPos = pos;
      currPos.x = startPos.x + ( float ) ( freeBlockIt->myStart * aMemoryToPixelScale );
      ImGui::SetCursorPos( currPos );
      ImGui::Button( stringBuffer.Format( "%s", locGetMemoryLabel( freeMemory, stringBuffer ) ), ImVec2( pixelWidth - 1, elementHeight ) );
      if ( ImGui::IsItemHovered() )
        ImGui::SetTooltip( stringBuffer.Format( "Free Block\nStart: %s\nEnd: %s\n Size: %s", locGetMemoryLabel( freeBlockIt->myStart, stringBuffer ),
                                                locGetMemoryLabel( freeBlockIt->myEnd, stringBuffer ),
                                                locGetMemoryLabel( freeMemory, stringBuffer ) ) );
    }
    ImGui::PopStyleColor( 1 );

    ImGui::PushStyleColor( ImGuiCol_Button, 0xFFFFAAAA );
    for ( const auto & debugInfo : allocator.myAllocDebugInfos ) {
      const uint64 allocatedMemory = debugInfo.myEnd - debugInfo.myStart;
      const float  pixelWidth = aMemoryToPixelScale * ( float ) allocatedMemory;
      if ( pixelWidth < 2 )
        continue;

      ImVec2 currPos = pos;
      currPos.x = startPos.x + ( float ) ( debugInfo.myStart * aMemoryToPixelScale );
      ImGui::SetCursorPos( currPos );
      ImGui::Button( stringBuffer.Format( "%s", locGetMemoryLabel( allocatedMemory, stringBuffer ) ), ImVec2( pixelWidth - 1, elementHeight ) );
      if ( ImGui::IsItemHovered() )
        ImGui::SetTooltip( stringBuffer.Format( "Allocated Block %s\nStart: %s\nEnd: %s\nSize: %s", debugInfo.myName.c_str(),
                                                locGetMemoryLabel( debugInfo.myStart, stringBuffer ),
                                                locGetMemoryLabel( debugInfo.myEnd, stringBuffer ),
                                                locGetMemoryLabel( allocatedMemory, stringBuffer ) ) );
    }
    ImGui::PopStyleColor( 1 );
  }
#endif
  //---------------------------------------------------------------------------//

  GpuMemoryViewer::GpuMemoryViewer()
      : myScale( 10.0f ) {}

  void GpuMemoryViewer::Render() {
#if FANCY_ENABLE_DX12
    RenderCore_PlatformDX12 * platformDx12 = RenderCore::GetPlatformDX12();
    if ( platformDx12 == nullptr )
      return;

    ImGui::SliderFloat( "Scale", &myScale, 0.1f, 2000.0f );

    ImGui::BeginChildFrame( 1, ImVec2( ImGui::GetWindowWidth(), 512 ), ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoBackground );
    for ( uint memType = 0; memType < ( uint ) GpuMemoryType::NUM; ++memType ) {
      if ( ImGui::TreeNode( locMemoryTypeToString( ( GpuMemoryType ) memType ) ) ) {
        for ( uint accessType = 0; accessType < ( uint ) CpuMemoryAccessType::NUM; ++accessType ) {
          ImGui::Text( locCpuAccessTypeToString( ( CpuMemoryAccessType ) accessType ) );
          GpuMemoryAllocatorDX12 * allocatorDx12 = platformDx12->myGpuMemoryAllocators[ memType ][ accessType ].get();
          locDebugPrintMemoryAllocatorDx12( allocatorDx12, myScale / SIZE_MB );
        }
        ImGui::TreePop();
      }
    }
    ImGui::EndChildFrame();
#endif
  }
}  // namespace Fancy
