#include "FrameGraphViewer.h"

#include <algorithm>

#include "imgui.h"
#include "EASTL/vector.h"
#include "Rendering/RenderCore.h"
#include "Rendering/FrameGraph/FrameGraphBuilder.h"

namespace Fancy {
  //---------------------------------------------------------------------------//
  namespace Priv_FrameGraphViewer {
    const float kNodeWidth = 160.0f;
    const float kNodeHeight = 32.0f;
    const float kColumnSpacing = 60.0f;
    const float kRowSpacing = 20.0f;
    const float kCanvasPadding = 16.0f;
    //---------------------------------------------------------------------------//
    void locListAccesses( const FrameGraphBuilder & aFrameGraph, const FrameGraphTask & aTask ) {
      if ( aTask.myAccesses.empty() ) {
        ImGui::BulletText( "No resource accesses" );
      }
      for ( const FrameGraphTaskAccess & access : aTask.myAccesses ) {
        const FgResourceHandle & handle = access.myHandle;
        const SubresourceRange & range = handle.mySubresources;
        const FrameGraphResourceBase & base = aFrameGraph.GetResourceBases()[ handle.myBaseIndex ];
        const char * verb = RenderCore::IsWriteAccess( access.myAccess ) ? "Writes" : "Reads";
        const char * accessName = RenderCore::GetResourceAccessName( access.myAccess );
        if ( base.myResource->GetSubresources().myNumArrayIndices > 1u ) {
          ImGui::BulletText( "%s %s mips %u-%u planes %u-%u slices %u-%u: %s", verb, base.myName.c_str(), range.myFirstMipLevel,
                             range.myFirstMipLevel + range.myNumMipLevels - 1u, range.myFirstPlane, range.myFirstPlane + range.myNumPlanes - 1u,
                             range.myFirstArrayIndex, range.myFirstArrayIndex + range.myNumArrayIndices - 1u, accessName );
        } else {
          ImGui::BulletText( "%s %s mips %u-%u planes %u-%u: %s", verb, base.myName.c_str(), range.myFirstMipLevel,
                             range.myFirstMipLevel + range.myNumMipLevels - 1u, range.myFirstPlane, range.myFirstPlane + range.myNumPlanes - 1u,
                             accessName );
        }
      }
    }
  }  // namespace Priv_FrameGraphViewer
  //---------------------------------------------------------------------------//
  void FrameGraphViewer::Render( const FrameGraphBuilder & aFrameGraph, bool * anIsOpen ) {
    using namespace Priv_FrameGraphViewer;

    ImGui::SetNextWindowSize( ImVec2( 800, 400 ), ImGuiCond_FirstUseEver );
    if ( !ImGui::Begin( "Frame Graph", anIsOpen ) ) {
      ImGui::End();
      return;
    }

    if ( !aFrameGraph.IsBuilt() ) {
      ImGui::Text( "The frame graph has not been built." );
      ImGui::End();
      return;
    }

    const eastl::fixed_vector< FrameGraphTask, kFgMaxNumTasks > & tasks = aFrameGraph.GetTasks();
    const eastl::fixed_vector< FrameGraphDependency, kFgMaxNumTasks > & dependencies = aFrameGraph.GetDependencies();
    const uint numTasks = ( uint ) tasks.size();

    // Column = length of the longest chain of dependencies leading to the task. The task order is topological, so all
    // predecessors already have their final column when a task is visited.
    eastl::vector< uint > column( numTasks, 0u );
    for ( uint taskIdx : aFrameGraph.GetTaskOrder() ) {
      for ( const FrameGraphDependency & dependency : dependencies ) {
        if ( dependency.myDstTask == taskIdx ) {
          column[ taskIdx ] = std::max( column[ taskIdx ], column[ dependency.mySrcTask ] + 1u );
        }
      }
    }

    eastl::vector< uint > row( numTasks, 0u );
    eastl::vector< uint > numRowsInColumn( numTasks, 0u );
    for ( uint taskIdx : aFrameGraph.GetTaskOrder() ) {
      row[ taskIdx ] = numRowsInColumn[ column[ taskIdx ] ]++;
    }

    ImDrawList * drawList = ImGui::GetWindowDrawList();
    const ImVec2 origin = ImGui::GetCursorScreenPos();

    eastl::vector< ImVec2 > nodeMin( numTasks );
    eastl::vector< ImVec2 > nodeMax( numTasks );
    ImVec2 canvasSize( 0.0f, 0.0f );
    for ( uint taskIdx = 0u; taskIdx < numTasks; ++taskIdx ) {
      const ImVec2 localMin( kCanvasPadding + column[ taskIdx ] * ( kNodeWidth + kColumnSpacing ),
                             kCanvasPadding + row[ taskIdx ] * ( kNodeHeight + kRowSpacing ) );
      nodeMin[ taskIdx ] = ImVec2( origin.x + localMin.x, origin.y + localMin.y );
      nodeMax[ taskIdx ] = ImVec2( nodeMin[ taskIdx ].x + kNodeWidth, nodeMin[ taskIdx ].y + kNodeHeight );
      canvasSize.x = std::max( canvasSize.x, localMin.x + kNodeWidth + kCanvasPadding );
      canvasSize.y = std::max( canvasSize.y, localMin.y + kNodeHeight + kCanvasPadding );
    }

    for ( const FrameGraphDependency & dependency : dependencies ) {
      const ImVec2 from( nodeMax[ dependency.mySrcTask ].x, ( nodeMin[ dependency.mySrcTask ].y + nodeMax[ dependency.mySrcTask ].y ) * 0.5f );
      const ImVec2 to( nodeMin[ dependency.myDstTask ].x, ( nodeMin[ dependency.myDstTask ].y + nodeMax[ dependency.myDstTask ].y ) * 0.5f );
      drawList->AddLine( from, to, IM_COL32( 180, 180, 180, 255 ), 1.5f );
    }

    for ( uint taskIdx = 0u; taskIdx < numTasks; ++taskIdx ) {
      drawList->AddRectFilled( nodeMin[ taskIdx ], nodeMax[ taskIdx ], IM_COL32( 50, 80, 140, 255 ), 4.0f );
      drawList->PushClipRect( nodeMin[ taskIdx ], nodeMax[ taskIdx ], true );
      drawList->AddText( ImVec2( nodeMin[ taskIdx ].x + 6.0f, nodeMin[ taskIdx ].y + 8.0f ), IM_COL32_WHITE, tasks[ taskIdx ].myName.c_str() );
      drawList->PopClipRect();

      if ( ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect( nodeMin[ taskIdx ], nodeMax[ taskIdx ] ) ) {
        ImGui::BeginTooltip();
        ImGui::Text( "%s", tasks[ taskIdx ].myName.c_str() );
        ImGui::Separator();
        ImGui::Text( "Queue: %s", tasks[ taskIdx ].myQueueType == CommandListType::Compute ? "Compute" : "Graphics" );
        locListAccesses( aFrameGraph, tasks[ taskIdx ] );
        ImGui::EndTooltip();
      }
    }

    // Reserve the drawn area so the window can scroll
    ImGui::Dummy( canvasSize );

    ImGui::End();
  }
  //---------------------------------------------------------------------------//
}  // namespace Fancy
