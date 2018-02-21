#include "imgui_impl_fancy.h"
#include "imgui.h"

#include "fancy_core/Fancy_Include.h"
#include "fancy_core/GpuProgramPipeline.h"
#include "fancy_core/RenderCore.h"
#include "fancy_core/Window.h"
#include "fancy_core/GeometryData.h"
#include "fancy_core/RenderOutput.h"
#include "fancy_core/Texture.h"
#include "fancy_core/BlendState.h"
#include "fancy_core/DepthStencilState.h"
#include "fancy_core/CommandContext.h"
#include "fancy_core/CommandListType.h"
#include "fancy_core/GpuProgramDesc.h"
#include "fancy_core/GpuProgramPipelineDesc.h"

namespace Fancy { namespace ImGui {
//---------------------------------------------------------------------------//
  FancyRuntime* ourFancyRuntime = nullptr;
  Rendering::RenderOutput* ourRenderOutput = nullptr;

  struct CBufferData
  {
    glm::float4x4 myProjectionMatrix;
  };

  const uint kVertexNumIncrease = 2048u;
  const uint kIndexNumIncrease = kVertexNumIncrease * 3u;

  SharedPtr<Rendering::GpuBuffer> ourCBuffer;
  SharedPtr<Rendering::GpuBuffer> ourVertexBuffer;
  SharedPtr<Rendering::GpuBuffer> ourIndexBuffer;
  SharedPtr<Rendering::Texture> ourFontTexture;
  SharedPtr<Rendering::GpuProgramPipeline> ourProgramPipeline;
  SharedPtr<Rendering::BlendState> ourBlendState;
  SharedPtr<Rendering::DepthStencilState> ourDepthStencilState;
  Rendering::CommandContext* ourRenderContext;
    
  HWND ourHwnd = nullptr;
  INT64 ourTicksPerSecond = 0;
  INT64 ourTime = 0;

  void HandleWindowEvent(UINT msg, WPARAM wParam, LPARAM lParam, bool* aWasHandled)
  {
    ImGuiIO& io = ::ImGui::GetIO();
    switch (msg)
    {
    case WM_LBUTTONDOWN:
      io.MouseDown[0] = true;
      (*aWasHandled) = true;
      return;
    case WM_LBUTTONUP:
      io.MouseDown[0] = false;
      (*aWasHandled) = true;
      return;
    case WM_RBUTTONDOWN:
      io.MouseDown[1] = true;
      (*aWasHandled) = true;
      return;
    case WM_RBUTTONUP:
      io.MouseDown[1] = false;
      (*aWasHandled) = true;
      return;
    case WM_MOUSEWHEEL:
      io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
      (*aWasHandled) = true;
      return;
    case WM_MOUSEMOVE:
      io.MousePos.x = (signed short)(lParam);
      io.MousePos.y = (signed short)(lParam >> 16);
      (*aWasHandled) = true;
      return;
    case WM_KEYDOWN:
      if (wParam < 256)
        io.KeysDown[wParam] = 1;
      (*aWasHandled) = true;
      return;
    case WM_KEYUP:
      if (wParam < 256)
        io.KeysDown[wParam] = 0;
      (*aWasHandled) = true;
      return;
    case WM_CHAR:
      // You can also use ToAscii()+GetKeyboardState() to retrieve characters.
      if (wParam > 0 && wParam < 0x10000)
        io.AddInputCharacter((unsigned short)wParam);
      (*aWasHandled) = true;
      return;
    }

    (*aWasHandled) = false;
  }
  
  namespace {
    SharedPtr<Rendering::GpuBuffer> locCreateVertexBuffer(uint aNumRequiredVertices)
    {
      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.myUsageFlags = static_cast<uint>(Rendering::GpuBufferUsage::VERTEX_BUFFER);
      bufferParams.uAccessFlags = (uint)Rendering::GpuResourceAccessFlags::WRITE
                                | (uint)Rendering::GpuResourceAccessFlags::COHERENT
                                | (uint)Rendering::GpuResourceAccessFlags::DYNAMIC
                                | (uint)Rendering::GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uElementSizeBytes = sizeof(ImDrawVert);
      bufferParams.uNumElements = aNumRequiredVertices;
      return Rendering::RenderCore::CreateBuffer(bufferParams);
    }

    SharedPtr<Rendering::GpuBuffer> locCreateIndexBuffer(uint aNumRequiredIndices)
    {
      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.myUsageFlags = static_cast<uint>(Rendering::GpuBufferUsage::INDEX_BUFFER);
      bufferParams.uAccessFlags = (uint)Rendering::GpuResourceAccessFlags::WRITE
        | (uint)Rendering::GpuResourceAccessFlags::COHERENT
        | (uint)Rendering::GpuResourceAccessFlags::DYNAMIC
        | (uint)Rendering::GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uElementSizeBytes = sizeof(ImDrawIdx);
      bufferParams.uNumElements = aNumRequiredIndices;
      return Rendering::RenderCore::CreateBuffer(bufferParams);
    }
  }

  bool Init(Fancy::Rendering::RenderOutput* aRenderOutput, Fancy::FancyRuntime* aRuntime)
  {
    ourRenderOutput = aRenderOutput;
    std::function<void(UINT, WPARAM, LPARAM, bool*)> fnWindowHandler = &HandleWindowEvent;
    ourRenderOutput->GetWindow()->myWindowEventHandler.Connect(fnWindowHandler);

    ourFancyRuntime = aRuntime;
    ourHwnd = ourRenderOutput->GetWindow()->GetWindowHandle();

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
      Rendering::GpuProgramDesc* shaderDesc = &pipelineDesc.myGpuPrograms[(uint)Rendering::ShaderStage::VERTEX];
      shaderDesc->myShaderFileName = "Imgui";
      shaderDesc->myMainFunction = "main";
      shaderDesc->myShaderStage = (uint)Rendering::ShaderStage::VERTEX;
      shaderDesc = &pipelineDesc.myGpuPrograms[(uint)Rendering::ShaderStage::FRAGMENT];
      shaderDesc->myShaderFileName = "Imgui";
      shaderDesc->myShaderStage = (uint)Rendering::ShaderStage::FRAGMENT;
      shaderDesc->myMainFunction = "main";
      ourProgramPipeline = Rendering::RenderCore::CreateGpuProgramPipeline(pipelineDesc);
      ASSERT(ourProgramPipeline != nullptr);
    }

    // Create the cbuffer
    {
      Rendering::GpuBufferCreationParams bufferParams;
      bufferParams.myUsageFlags = static_cast<uint>(Rendering::GpuBufferUsage::CONSTANT_BUFFER);
      bufferParams.uAccessFlags = (uint)Rendering::GpuResourceAccessFlags::WRITE
                                 | (uint)Rendering::GpuResourceAccessFlags::COHERENT
                                 | (uint)Rendering::GpuResourceAccessFlags::DYNAMIC
                                 | (uint)Rendering::GpuResourceAccessFlags::PERSISTENT_LOCKABLE;
      bufferParams.uElementSizeBytes = sizeof(CBufferData);
      bufferParams.uNumElements = 1u;

      CBufferData initialData;
      memset(&initialData, 0, sizeof(initialData));
      ourCBuffer = Rendering::RenderCore::CreateBuffer(bufferParams, &initialData);
      ASSERT(ourCBuffer != nullptr);
    }
  
    ourVertexBuffer = locCreateVertexBuffer(kVertexNumIncrease);
    ASSERT(ourVertexBuffer != nullptr);
   
    ourIndexBuffer = locCreateIndexBuffer(kIndexNumIncrease);
    ASSERT(ourIndexBuffer != nullptr);
   
    // Create the font texture
    {
      uint8* fontPixelData = nullptr;
      int width, height, pixelSizeBytes;
      ImGuiIO& io = ::ImGui::GetIO();
      io.Fonts->GetTexDataAsRGBA32(&fontPixelData, &width, &height, &pixelSizeBytes);
      ASSERT(fontPixelData != nullptr);

      Rendering::TextureParams params;
      params.u16Width = width;
      params.u16Height = height;
      params.eFormat = Rendering::DataFormat::RGBA_8;
      const Rendering::DataFormatInfo& formatInfo = Rendering::DataFormatInfo::GetFormatInfo(params.eFormat);
      ASSERT(formatInfo.mySizeBytes == pixelSizeBytes);
      
      Rendering::TextureUploadData uploadData;
      uploadData.myData = fontPixelData;
      uploadData.myPixelSizeBytes = pixelSizeBytes;
      uploadData.myRowSizeBytes = width * uploadData.myPixelSizeBytes;
      uploadData.mySliceSizeBytes = height * uploadData.myRowSizeBytes;
      uploadData.myTotalSizeBytes = uploadData.mySliceSizeBytes;
      
      ourFontTexture = Rendering::RenderCore::CreateTexture(params, &uploadData, 1u);
      ASSERT(ourFontTexture != nullptr);
    }

    // Blend state (alpha blending)
    {
      Rendering::BlendStateDesc desc;
      desc.myBlendEnabled[0] = true;
      desc.mySrcBlend[0] = static_cast<uint>(Rendering::BlendInput::SRC_ALPHA);
      desc.myDestBlend[0] = static_cast<uint>(Rendering::BlendInput::INV_SRC_ALPHA);
      desc.myBlendOp[0] = static_cast<uint>(Rendering::BlendOp::ADD);
      ourBlendState = Rendering::RenderCore::CreateBlendState(desc);
      ASSERT(ourBlendState != nullptr);
    }

    // Depth-stencil state (no depth testing)
    {
      Rendering::DepthStencilStateDesc desc;
      desc.myStencilEnabled = false;
      desc.myDepthTestEnabled = false;
      desc.myDepthWriteEnabled = false;
      ourDepthStencilState = Rendering::RenderCore::CreateDepthStencilState(desc);
      ASSERT(ourDepthStencilState != nullptr);
    }

    ourRenderContext = Rendering::RenderCore::AllocateContext(Rendering::CommandListType::Graphics);
    
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
    ::ImGui::NewFrame();
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

    // Calculate the required number of vertices and indices and grow the buffers if neccessary
    {
      uint numRequiredVertices = 0u;
      uint numRequiredIndices = 0u;
      for (int n = 0; n < _draw_data->CmdListsCount; n++)
      {
        const ImDrawList* cmd_list = _draw_data->CmdLists[n];
        numRequiredVertices += cmd_list->VtxBuffer.size();
        numRequiredIndices += cmd_list->IdxBuffer.size();
      }

      if (numRequiredVertices > ourVertexBuffer->GetNumElements())
      {
        const uint newBufferSize = static_cast<uint>(Fancy::MathUtil::Align(numRequiredVertices, kVertexNumIncrease));
        ourVertexBuffer = locCreateVertexBuffer(newBufferSize);
        ASSERT(ourVertexBuffer != nullptr);
      }

      if (numRequiredIndices > ourIndexBuffer->GetNumElements())
      {
        const uint newBufferSize = (uint) Fancy::MathUtil::Align(numRequiredIndices, kIndexNumIncrease);
        ourIndexBuffer = locCreateIndexBuffer(newBufferSize);
        ASSERT(ourIndexBuffer != nullptr);
      }
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
        uint verticesCount = cmd_list->VtxBuffer.size();
        uint indicesCount = cmd_list->IdxBuffer.size();
        uint verticesSize = verticesCount * sizeof(ImDrawVert);
        uint indicesSize = indicesCount * sizeof(ImDrawIdx);

        memcpy(mappedVertexData, &cmd_list->VtxBuffer[0], verticesSize);
        mappedVertexData += verticesSize;

        memcpy(mappedIndexData, &cmd_list->IdxBuffer[0], indicesSize);
        mappedIndexData += indicesSize;
      }

      ourVertexBuffer->Unlock();
      ourIndexBuffer->Unlock();
    }

    Rendering::RenderOutput* renderOutput = ourRenderOutput;

    ourRenderContext->SetViewport(glm::uvec4(0, 0, ::ImGui::GetIO().DisplaySize.x, ::ImGui::GetIO().DisplaySize.y));
    ourRenderContext->SetRenderTarget(renderOutput->GetBackbuffer(), 0u);
    ourRenderContext->SetDepthStencilRenderTarget(renderOutput->GetDefaultDepthStencilBuffer());
    
    ourRenderContext->SetDepthStencilState(ourDepthStencilState);
    ourRenderContext->SetBlendState(ourBlendState);
    ourRenderContext->SetCullMode(Rendering::CullMode::NONE);
    ourRenderContext->SetFillMode(Rendering::FillMode::SOLID);
    ourRenderContext->SetWindingOrder(Rendering::WindingOrder::CCW);

    ourRenderContext->SetGpuProgramPipeline(ourProgramPipeline);
    ourRenderContext->BindResource(ourCBuffer.get(), Rendering::DescriptorType::CONSTANT_BUFFER, 0u);

    uint cmdListVertexOffset = 0u;
    uint cmdListIndexOffset = 0u;
    for (uint iCmdList = 0u; iCmdList < static_cast<uint>(_draw_data->CmdListsCount); ++iCmdList)
    {
      const ImDrawList* cmd_list = _draw_data->CmdLists[iCmdList];
      uint verticesCount = cmd_list->VtxBuffer.size();
      uint indicesCount = cmd_list->IdxBuffer.size();
      
      ourRenderContext->SetVertexIndexBuffers(ourVertexBuffer.get(), ourIndexBuffer.get(), cmdListVertexOffset, verticesCount, cmdListIndexOffset, indicesCount);
      
      cmdListVertexOffset += verticesCount;
      cmdListIndexOffset += indicesCount;

      uint cmdIndexOffset = 0;
      for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++)
      {
        const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
        if (pcmd->UserCallback)
        {
          pcmd->UserCallback(cmd_list, pcmd);
        }
        else
        {
          const Rendering::Descriptor* descriptors[] = { ourFontTexture->GetDescriptor(Rendering::DescriptorType::DEFAULT_READ) };

          ImTextureID textureId = pcmd->TextureId;
          if (textureId != nullptr)
            descriptors[0] = static_cast<const Rendering::Descriptor*>(textureId);

          ourRenderContext->BindDescriptorSet(descriptors, 1u, 1u);

          const glm::uvec4 clipRect( (uint) pcmd->ClipRect.x, (uint) pcmd->ClipRect.y, (uint) pcmd->ClipRect.z, (uint) pcmd->ClipRect.w);
          ourRenderContext->SetClipRect(clipRect);

          ourRenderContext->Render(pcmd->ElemCount, 1u, cmdIndexOffset, 0u, 0u);
        }
        cmdIndexOffset += pcmd->ElemCount;
      }
    }

    ourRenderContext->ExecuteAndReset(true);
  }

  void Shutdown()
  {
    Rendering::RenderCore::FreeContext(ourRenderContext);
    ourRenderContext = nullptr;

    ourFancyRuntime = nullptr;
    ourRenderOutput = nullptr;
    
    ourCBuffer.reset();
    ourVertexBuffer.reset();
    ourIndexBuffer.reset();
    ourFontTexture.reset();
    ourProgramPipeline.reset();
    ourBlendState.reset();
    ourDepthStencilState.reset();
    
    ourHwnd = nullptr;
    ourTicksPerSecond = 0;
    ourTime = 0;
  }
//---------------------------------------------------------------------------//
} }
