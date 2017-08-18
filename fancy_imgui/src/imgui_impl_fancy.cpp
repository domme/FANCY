#include "imgui_impl_fancy.h"
#include "imgui.h"

#include <Fancy_Include.h>

#include "GpuProgramPipeline.h"
#include <RenderCore.h>
#include <GeometryVertexLayout.h>
#include <GeometryData.h>
#include <RenderContext.h>
#include <RenderView.h>
#include <RenderOutput.h>
#include <Texture.h>

namespace Fancy { namespace ImGui {
//---------------------------------------------------------------------------//
  Fancy::FancyRuntime* ourFancyRuntime = nullptr;
  Fancy::RenderWindow* ourRenderWindow = nullptr;

  struct CBufferData
  {
    glm::float4x4 myProjectionMatrix;
  };

  const uint32 kMaxNumVertices = 2048u;
  const uint32 kMaxNumIndices = kMaxNumVertices * 3u;

  SharedPtr<Rendering::GpuBuffer> ourCBuffer;
  SharedPtr<Rendering::GpuBuffer> ourVertexBuffer;
  SharedPtr<Rendering::GpuBuffer> ourIndexBuffer;
  SharedPtr<Rendering::Texture> ourFontTexture;
  SharedPtr<Geometry::GeometryData> ourGeometryData;
  SharedPtr<Rendering::GpuProgramPipeline> ourProgramPipeline;
    
  HWND ourHwnd = nullptr;
  INT64 ourTicksPerSecond = 0;
  INT64 ourTime = 0;
  
  bool Init(Fancy::RenderWindow* aRenderWindow, Fancy::FancyRuntime* aRuntime)
  {
    ourRenderWindow = aRenderWindow;
    ourFancyRuntime = aRuntime;
    ourHwnd = aRenderWindow->GetWindowHandle();
  
    ImGuiIO& io = ::ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = VK_TAB;                              // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
    io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
    io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
    io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
    io.KeyMap[ImGuiKey_Home] = VK_HOME;
    io.KeyMap[ImGuiKey_End] = VK_END;
    io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
    io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = 'A';
    io.KeyMap[ImGuiKey_C] = 'C';
    io.KeyMap[ImGuiKey_V] = 'V';
    io.KeyMap[ImGuiKey_X] = 'X';
    io.KeyMap[ImGuiKey_Y] = 'Y';
    io.KeyMap[ImGuiKey_Z] = 'Z';
  
    io.RenderDrawListsFn = RenderDrawLists;
    io.ImeWindowHandle = ourHwnd;
  
    if (!QueryPerformanceFrequency((LARGE_INTEGER *)&ourTicksPerSecond))
      return false;
    if (!QueryPerformanceCounter((LARGE_INTEGER *)&ourTime))
      return false;

    // Load the imgui-shader state
    {
      Rendering::GpuProgramPipelineDesc pipelineDesc;
      Rendering::GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint32)Rendering::ShaderStage::VERTEX];
      shaderDesc->myShaderFileName = "Imgui";
      shaderDesc->myMainFunction = "main";
      shaderDesc->myShaderStage = (uint32)Rendering::ShaderStage::VERTEX;
      shaderDesc = &pipelineDesc.myGpuPrograms[(uint32)Rendering::ShaderStage::FRAGMENT];
      shaderDesc->myShaderFileName = "Imgui";
      shaderDesc->myShaderStage = (uint32)Rendering::ShaderStage::FRAGMENT;
      shaderDesc->myMainFunction = "main";
      ourProgramPipeline = Rendering::RenderCore::CreateGpuProgramPipeline(pipelineDesc);
      ASSERT(ourProgramPipeline != nullptr);
    }

    // Create the cbuffer
    {
      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::CONSTANT_BUFFER);
      bufferParams.uAccessFlags = (uint32)Rendering::GpuResourceAccessFlags::WRITE
                                 | (uint32)Rendering::GpuResourceAccessFlags::COHERENT
                                 | (uint32)Rendering::GpuResourceAccessFlags::DYNAMIC
                                 | (uint32)Rendering::GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uElementSizeBytes = sizeof(CBufferData);
      bufferParams.uNumElements = 1u;

      CBufferData initialData;
      memset(&initialData, 0, sizeof(initialData));
      ourCBuffer = Rendering::RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(ourCBuffer != nullptr);
    }

    // Vertex buffer
    {
      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::VERTEX_BUFFER);
      bufferParams.uAccessFlags = (uint32)Rendering::GpuResourceAccessFlags::WRITE
                                  | (uint32)Rendering::GpuResourceAccessFlags::COHERENT
                                  | (uint32)Rendering::GpuResourceAccessFlags::DYNAMIC
                                  | (uint32)Rendering::GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uElementSizeBytes = sizeof(ImDrawVert);
      bufferParams.uNumElements = kMaxNumVertices;
      ourVertexBuffer = Rendering::RenderCore::CreateBuffer(bufferParams);
    }

    // Index buffer
    {
      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.myUsageFlags = static_cast<uint32>(Rendering::GpuBufferUsage::INDEX_BUFFER);
      bufferParams.uAccessFlags = (uint32)Rendering::GpuResourceAccessFlags::WRITE
        | (uint32)Rendering::GpuResourceAccessFlags::COHERENT
        | (uint32)Rendering::GpuResourceAccessFlags::DYNAMIC
        | (uint32)Rendering::GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uElementSizeBytes = sizeof(ImDrawIdx);
      bufferParams.uNumElements = kMaxNumIndices;
      ourIndexBuffer = Rendering::RenderCore::CreateBuffer(bufferParams);
    }

    // Geometry Data
    {
      Rendering::GeometryVertexLayout vertexLayout;
      uint32 offsetBytes = 0u;

      // pos
      {
        Rendering::GeometryVertexElement elem;
        elem.eFormat = Rendering::DataFormat::RG_32F;
        elem.eSemantics = Rendering::VertexSemantics::POSITION;
        elem.name = "pos";
        elem.u32SizeBytes = sizeof(ImVec2);
        elem.u32OffsetBytes = offsetBytes;
        vertexLayout.addVertexElement(elem);
        offsetBytes += elem.u32SizeBytes;
      }

      // uv
      {
        Rendering::GeometryVertexElement elem;
        elem.eFormat = Rendering::DataFormat::RG_32F;
        elem.eSemantics = Rendering::VertexSemantics::TEXCOORD;
        elem.name = "uv";
        elem.u32SizeBytes = sizeof(ImVec2);
        elem.u32OffsetBytes = offsetBytes;
        vertexLayout.addVertexElement(elem);
        offsetBytes += elem.u32SizeBytes;
      }

      // color
      {
        Rendering::GeometryVertexElement elem;
        elem.eFormat = Rendering::DataFormat::RGBA_8;
        elem.eSemantics = Rendering::VertexSemantics::COLOR;
        elem.name = "col";
        elem.u32SizeBytes = sizeof(ImU32);
        elem.u32OffsetBytes = offsetBytes;
        vertexLayout.addVertexElement(elem);
        offsetBytes += elem.u32SizeBytes;
      }

      ourGeometryData = std::make_shared<Geometry::GeometryData>();
      ourGeometryData->setVertexBuffer(ourVertexBuffer);
      ourGeometryData->setIndexBuffer(ourIndexBuffer);
      ourGeometryData->setVertexLayout(vertexLayout);
    }

    // Load the font texture
    {
      
    }
    
    return true;
  }
//---------------------------------------------------------------------------//
  void NewFrame()
  {
    ImGuiIO& io = ::ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    RECT rect;
    GetClientRect(ourHwnd, &rect);
    io.DisplaySize = ImVec2((float)(rect.right - rect.left), (float)(rect.bottom - rect.top));

    // Setup time step
    INT64 current_time;
    QueryPerformanceCounter((LARGE_INTEGER *)&current_time);
    io.DeltaTime = (float)(current_time - ourTime) / ourTicksPerSecond;
    ourTime = current_time;

    // Read keyboard modifiers inputs
    io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
    io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
    // io.KeysDown : filled by WM_KEYDOWN/WM_KEYUP events
    // io.MousePos : filled by WM_MOUSEMOVE events
    // io.MouseDown : filled by WM_*BUTTON* events
    // io.MouseWheel : filled by WM_MOUSEWHEEL events

    // Hide OS mouse cursor if ImGui is drawing it
    SetCursor(io.MouseDrawCursor ? NULL : LoadCursor(NULL, IDC_ARROW));

    // Start the frame
    ImGui::NewFrame();
  }
//---------------------------------------------------------------------------//
  void RenderDrawLists(ImDrawData* _draw_data)
  {
    // Update the cbuffer data
    {
      float translate = -0.5f * 2.f;
      const float L = 0.f;
      const float R = ::ImGui::GetIO().DisplaySize.x;
      const float B = ::ImGui::GetIO().DisplaySize.y;
      const float T = 0.f;

      CBufferData cbuffer = {};
      cbuffer.myProjectionMatrix = glm::float4x4(
        2.0f / (R - L),     0.0f,               0.0f,       0.0f,
        0.0f,               2.0f / (T - B),     0.0f,       0.0f,
        0.0f,               0.0f,               0.5f,       0.0f,
        (R + L) / (L - R),  (T + B) / (B - T),  0.5f,       1.0f);
      
      Rendering::RenderCore::UpdateBufferData(ourCBuffer.get(), &cbuffer, sizeof(cbuffer));
    }

    // Copy all vertex- and index data
    {
      uint8* mappedVertexData = static_cast<uint8*>(ourVertexBuffer->Lock(Rendering::GpuResoruceLockOption::WRITE));
      ASSERT(mappedVertexData != nullptr);

      uint8* mappedIndexData = static_cast<uint8*>(ourIndexBuffer->Lock(Rendering::GpuResoruceLockOption::WRITE));
      ASSERT(mappedIndexData != nullptr);

      for (int n = 0; n < _draw_data->CmdListsCount; n++)
      {
        const ImDrawList* cmd_list = _draw_data->CmdLists[n];
        size_t verticesCount = cmd_list->VtxBuffer.size();
        size_t indicesCount = cmd_list->IdxBuffer.size();
        size_t verticesSize = verticesCount * sizeof(ImDrawVert);
        size_t indicesSize = indicesCount * sizeof(ImDrawIdx);

        memcpy(mappedVertexData, &cmd_list->VtxBuffer[0], verticesSize);
        mappedVertexData += verticesSize;

        memcpy(mappedIndexData, &cmd_list->IdxBuffer[0], indicesSize);
        mappedIndexData += indicesSize;
      }

      Rendering::RenderContext* context =
        static_cast<Rendering::RenderContext*>(Rendering::RenderCore::AllocateContext(Rendering::CommandListType::Graphics));

      // TODO: Don't hardcode which renderOutput to use...
      Rendering::RenderOutput* renderOutput = ourFancyRuntime->GetMainView()->GetRenderOutput();

      context->SetViewport(glm::uvec4(0, 0, ::ImGui::GetIO().DisplaySize.x, ::ImGui::GetIO().DisplaySize.y));
      context->SetRenderTarget(renderOutput->GetBackbuffer(), 0u);
      context->SetDepthStencilRenderTarget(renderOutput->GetDefaultDepthStencilBuffer());
      
      context->SetDepthStencilState(nullptr);
      context->SetBlendState(nullptr);
      context->SetCullMode(Rendering::CullMode::NONE);
      context->SetFillMode(Rendering::FillMode::SOLID);
      context->SetWindingOrder(Rendering::WindingOrder::CCW);

      context->SetGpuProgramPipeline(ourProgramPipeline);
      context->BindResource(ourCBuffer.get(), Rendering::ResourceBindingType::CONSTANT_BUFFER, 0u);
      context->BindResource(ourFontTexture.get(), Rendering::ResourceBindingType::SIMPLE, 0u);

    }
  }
//---------------------------------------------------------------------------//
} }
