#include "fancy_core_precompile.h"
#include "CommandListVk.h"
#include "RenderCore.h"
#include "RenderCore_PlatformVk.h"
#include "ShaderPipeline.h"
#include "ShaderVk.h"
#include "ShaderPipelineVk.h"
#include "BlendState.h"
#include "Texture.h"
#include "TextureVk.h"
#include "GpuResourceViewDataVk.h"
#include "GpuBufferVk.h"
#include "ShaderResourceInfoVk.h"
#include "TextureSamplerVk.h"
#include "DebugUtilsVk.h"
#include "GpuQueryHeapVk.h"
#include "TimeManager.h"
#include "PipelineLayoutVk.h"

#if FANCY_ENABLE_VK

namespace Fancy
{
//---------------------------------------------------------------------------//
  namespace Priv_CommandListVk
  {
//---------------------------------------------------------------------------//
    VkImageLayout locResolveImageLayout(VkImageLayout aLayout, const GpuResource* aResource, const SubresourceRange& aSubresourceRange)
    {
      if (aResource->IsBuffer())
        return VK_IMAGE_LAYOUT_UNDEFINED;

      const GpuResourceHazardDataVk& globalHazardData = aResource->GetVkData()->myHazardData;
      for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it)
      {
        const uint subresourceIdx = aResource->GetSubresourceIndex(*it);
        if (globalHazardData.mySubresources[subresourceIdx].myContext == CommandListType::SHARED_READ)
          return VK_IMAGE_LAYOUT_GENERAL;
      }

      return aLayout;
    }
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locAccessMaskGraphics = static_cast<VkAccessFlags>(~0u);
    constexpr VkAccessFlags locAccessMaskCompute = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT
      | VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT
      | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locAccessMaskRead = VK_ACCESS_INDIRECT_COMMAND_READ_BIT | VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT |
      VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_HOST_READ_BIT | VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_READ_BIT_EXT | VK_ACCESS_CONDITIONAL_RENDERING_READ_BIT_EXT |
      VK_ACCESS_COMMAND_PREPROCESS_READ_BIT_NV | VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT | VK_ACCESS_SHADING_RATE_IMAGE_READ_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV | VK_ACCESS_FRAGMENT_DENSITY_MAP_READ_BIT_EXT;
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locAccessMaskWrite = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT | VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_MEMORY_WRITE_BIT |
      VK_ACCESS_TRANSFORM_FEEDBACK_COUNTER_WRITE_BIT_EXT | VK_ACCESS_COMMAND_PREPROCESS_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV;
//---------------------------------------------------------------------------//
    constexpr VkPipelineStageFlags locPipelineMaskGraphics =
      VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT | VK_PIPELINE_STAGE_VERTEX_INPUT_BIT
      | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT
      | VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT | VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT
      | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT
      | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
      | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT
      | VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_TRANSFORM_FEEDBACK_BIT_EXT | VK_PIPELINE_STAGE_CONDITIONAL_RENDERING_BIT_EXT
      | VK_PIPELINE_STAGE_COMMAND_PREPROCESS_BIT_NV | VK_PIPELINE_STAGE_SHADING_RATE_IMAGE_BIT_NV | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV
      | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV
      | VK_PIPELINE_STAGE_FRAGMENT_DENSITY_PROCESS_BIT_EXT
#if  FANCY_RENDERER_SUPPORT_MESH_SHADERS
      | VK_PIPELINE_STAGE_TASK_SHADER_BIT_NV | VK_PIPELINE_STAGE_MESH_SHADER_BIT_NV
#endif  // FANCY_RENDERER_SUPPORT_MESH_SHADERS
      ;
  //---------------------------------------------------------------------------//
    constexpr VkPipelineStageFlags locPipelineMaskCompute = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT |
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT | VK_PIPELINE_STAGE_TRANSFER_BIT | VK_PIPELINE_STAGE_HOST_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
//---------------------------------------------------------------------------//
    constexpr VkAccessFlags locGetContextAccessMask(CommandListType aCommandListType)
    {
      switch (aCommandListType) 
      { 
        case CommandListType::Graphics: return locAccessMaskGraphics;
        case CommandListType::Compute: return locAccessMaskCompute;
        default: ASSERT(false, "Missing implementation"); return 0u;
      }
    }
//---------------------------------------------------------------------------//
    constexpr VkPipelineStageFlags locGetContextPipelineStageMask(CommandListType aCommandListType)
    {
      switch (aCommandListType)
      {
      case CommandListType::Graphics: return locPipelineMaskGraphics;
      case CommandListType::Compute: return locPipelineMaskCompute;
      default: ASSERT(false, "Missing implementation"); return 0u;
      }
    }
//---------------------------------------------------------------------------//
    VkAccessFlags locResolveValidateDstAccessMask(const GpuResource* aResource, CommandListType aCommandListType, VkAccessFlags aAccessFlags)
    {
      const bool accessWas0 = aAccessFlags == 0u;

      const VkAccessFlags contextAccessMask = Priv_CommandListVk::locGetContextAccessMask(aCommandListType);
      VkAccessFlags dstFlags = aAccessFlags & contextAccessMask;
      ASSERT(accessWas0 || dstFlags != 0, "Unsupported access mask for this commandlist type");
      ASSERT((dstFlags & Priv_CommandListVk::locAccessMaskRead) == dstFlags || (dstFlags & Priv_CommandListVk::locAccessMaskWrite) == dstFlags, "Simulataneous read- and write access flags are not allowed");

      const bool dstIsRead = (dstFlags & Priv_CommandListVk::locAccessMaskRead) == dstFlags;
      dstFlags = dstFlags & (dstIsRead ? aResource->GetVkData()->myHazardData.myReadAccessMask : aResource->GetVkData()->myHazardData.myWriteAccessMask);
      ASSERT(accessWas0 || dstFlags != 0, "Dst access flags not supported by resource");

      return dstFlags;
    }
//---------------------------------------------------------------------------//
    bool locValidateDstImageLayout(const GpuResource* aResource, VkImageLayout anImageLayout)
    {
      if (aResource->myType == GpuResourceType::BUFFER)
      {
        ASSERT(anImageLayout == VK_IMAGE_LAYOUT_UNDEFINED);
        return anImageLayout == VK_IMAGE_LAYOUT_UNDEFINED;
      }
      else
      {
        const uint supportedLayoutMask = aResource->GetVkData()->myHazardData.mySupportedImageLayoutMask;
        const bool supportsLayout = (supportedLayoutMask & RenderCore_PlatformVk::ImageLayoutToFlag(anImageLayout)) != 0u;
        ASSERT(supportsLayout);
        return supportsLayout;
      }
    }
//---------------------------------------------------------------------------//
  }

//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
  CommandListVk::CommandListVk(CommandListType aType) 
    : CommandList(aType)
    , myCommandBuffer(nullptr)
    , myRenderPass(nullptr)
    , myFramebuffer(nullptr)
    , myFramebufferRes(0u, 0u)
    , myPendingBarrierSrcStageMask(0u)
    , myPendingBarrierDstStageMask(0u)
  {
    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    myCommandBuffer = platformVk->GetNewCommandBuffer(myCommandListType);

    BeginCommandBuffer();
  }
//---------------------------------------------------------------------------//
  CommandListVk::~CommandListVk()
  {
    CommandListVk::PostExecute(0ull);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearRenderTarget(TextureView* aTextureView, const float* aColor)
  {
    VkClearColorValue clearColor;
    memcpy(clearColor.float32, aColor, sizeof(clearColor.float32));

    VkImageSubresourceRange subRange = RenderCore_PlatformVk::ResolveSubresourceRange(aTextureView->mySubresourceRange, 
      aTextureView->GetTexture()->GetProperties().myFormat);

    const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    TrackSubresourceTransition(aTextureView->GetResource(), aTextureView->GetSubresourceRange(), VK_ACCESS_TRANSFER_WRITE_BIT, imageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();

#if FANCY_RENDERER_DEBUG
    VkFormatProperties formatProperties;
    const VkFormat format = RenderCore_PlatformVk::ResolveFormat(aTextureView->GetTexture()->GetProperties().myFormat);
    vkGetPhysicalDeviceFormatProperties(RenderCore::GetPlatformVk()->myPhysicalDevice, format, &formatProperties);
    ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
#endif

    GpuResourceDataVk* dataVk = static_cast<TextureVk*>(aTextureView->GetTexture())->GetData();
    vkCmdClearColorImage(myCommandBuffer, dataVk->myImage, imageLayout, &clearColor, 1u, &subRange);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearDepthStencilTarget(TextureView* aTextureView, float aDepthClear, uint8 aStencilClear, uint someClearFlags)
  {
    const DataFormat format = aTextureView->GetTexture()->GetProperties().myFormat;
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(format);
    ASSERT(aTextureView->myType == GpuResourceViewType::DSV);
    ASSERT(formatInfo.myIsDepthStencil);

    const bool clearDepth = someClearFlags & (uint)DepthStencilClearFlags::CLEAR_DEPTH;
    const bool clearStencil = someClearFlags & (uint)DepthStencilClearFlags::CLEAR_STENCIL;

    SubresourceRange subresources = aTextureView->GetSubresourceRange();

    if (clearDepth && !clearStencil)
    {
      ASSERT(subresources.myFirstPlane == 0, "The texture view doesn't cover the depth plane");
      subresources.myFirstPlane = 0u;
      subresources.myNumPlanes = 1u;
    }
    else if (clearStencil && !clearDepth)
    {
      ASSERT(formatInfo.myNumPlanes == 2);
      ASSERT(subresources.myFirstPlane + subresources.myNumPlanes >= 2, "The texture view doesn't cover the stencil plane");
      subresources.myFirstPlane = 1u;
      subresources.myNumPlanes = 1u;
    }
    else
    {
      ASSERT(formatInfo.myNumPlanes == 2);
      ASSERT(subresources.myFirstPlane == 0, "The texture view doesn't cover the depth plane");
      ASSERT(subresources.myFirstPlane + subresources.myNumPlanes >= 2, "The texture view doesn't cover the stencil plane");
      subresources.myFirstPlane = 0u;
      subresources.myNumPlanes = 2u;
    }
    
    const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    TrackSubresourceTransition(aTextureView->GetResource(), subresources, VK_ACCESS_TRANSFER_WRITE_BIT, imageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();

#if FANCY_RENDERER_DEBUG
    VkFormatProperties formatProperties;
    const VkFormat formatVk = RenderCore_PlatformVk::ResolveFormat(format);
    vkGetPhysicalDeviceFormatProperties(RenderCore::GetPlatformVk()->myPhysicalDevice, formatVk, &formatProperties);
    ASSERT(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT);
#endif

    VkClearDepthStencilValue clearValue;
    clearValue.depth = aDepthClear;
    clearValue.stencil = aStencilClear;

    VkImageSubresourceRange subRange = RenderCore_PlatformVk::ResolveSubresourceRange(aTextureView->mySubresourceRange, format);
    vkCmdClearDepthStencilImage(myCommandBuffer, aTextureView->GetResource()->GetVkData()->myImage, imageLayout, &clearValue, 1u, &subRange);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyResource(GpuResource* aDstResource, GpuResource* aSrcResource)
  {
    ASSERT(aDstResource != aSrcResource);
    ASSERT(aDstResource->GetType() == aSrcResource->GetType());

    if (aDstResource->GetType() == GpuResourceType::BUFFER)
    {
      GpuBuffer* dstBuffer = static_cast<GpuBuffer*>(aDstResource);
      GpuBuffer* srcBuffer = static_cast<GpuBuffer*>(aSrcResource);
      const uint64 size = srcBuffer->GetByteSize();
      ASSERT(size == dstBuffer->GetByteSize());
      CopyBuffer(dstBuffer, 0u, srcBuffer, 0u, size);
    }
    else
    {
      Texture* dstTexture = static_cast<Texture*>(aDstResource);
      Texture* srcTexture = static_cast<Texture*>(aSrcResource);

      const TextureProperties& dstTexProps = dstTexture->GetProperties();
      const TextureProperties& srcTexProps = srcTexture->GetProperties();

      const DataFormatInfo& dstFormatInfo = DataFormatInfo::GetFormatInfo(dstTexProps.myFormat);
      const DataFormatInfo& srcFormatInfo = DataFormatInfo::GetFormatInfo(srcTexProps.myFormat);

      ASSERT(dstTexProps.myWidth == srcTexProps.myWidth &&
        dstTexProps.myHeight == srcTexProps.myHeight &&
        dstTexProps.myDepthOrArraySize == srcTexProps.myDepthOrArraySize &&
        dstTexProps.myDimension == srcTexProps.myDimension &&
        dstFormatInfo.mySizeBytes == srcFormatInfo.mySizeBytes &&
        dstFormatInfo.myNumPlanes == srcFormatInfo.myNumPlanes);

      const SubresourceRange& subresourceRange = srcTexture->GetSubresources();
      ASSERT(subresourceRange == dstTexture->GetSubresources());

      const uint baseWidth = srcTexProps.myWidth;
      const uint baseHeight = srcTexProps.myHeight;
      const uint baseDepth = srcTexProps.IsArray() ? 1u : srcTexProps.myDepthOrArraySize;
      const uint numArrayLayers = srcTexProps.IsArray() ? srcTexProps.myDepthOrArraySize : 1u;

      const VkImageLayout srcImageLayout = Priv_CommandListVk::locResolveImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, srcTexture, subresourceRange);
      const VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

      TrackResourceTransition(srcTexture, VK_ACCESS_TRANSFER_READ_BIT, srcImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
      TrackResourceTransition(dstTexture, VK_ACCESS_TRANSFER_WRITE_BIT, dstImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
      FlushBarriers();

      eastl::fixed_vector<VkImageCopy, 16> copyRegions;
      for (SubresourceIterator it = subresourceRange.Begin(); it != subresourceRange.End(); ++it)
      {
        const SubresourceLocation subresource = *it;

        TextureRegion texelRegion;
        texelRegion.myPos = glm::uvec3(0);
        texelRegion.mySize.x = baseWidth >> subresource.myMipLevel;
        texelRegion.mySize.y = baseHeight >> subresource.myMipLevel;
        texelRegion.mySize.z = srcTexProps.IsArray() ? 1u : baseDepth >> subresource.myMipLevel;

#if FANCY_RENDERER_USE_VALIDATION
        ValidateTextureCopy(dstTexProps, subresource, texelRegion, srcTexProps, subresource, texelRegion);
#endif

        VkImageAspectFlags aspectMask = RenderCore_PlatformVk::ResolveAspectMask(subresource.myPlaneIndex, 1u, srcTexProps.myFormat);

        VkImageCopy& copyRegion = copyRegions.push_back();
        copyRegion.extent.width = texelRegion.mySize.x;
        copyRegion.extent.height = texelRegion.mySize.y;
        copyRegion.extent.depth = texelRegion.mySize.z;
        copyRegion.dstOffset.x = texelRegion.myPos.x;
        copyRegion.dstOffset.y = texelRegion.myPos.y;
        copyRegion.dstOffset.z = texelRegion.myPos.z;
        copyRegion.srcOffset.x = texelRegion.myPos.x;
        copyRegion.srcOffset.y = texelRegion.myPos.y;
        copyRegion.srcOffset.z = texelRegion.myPos.z;
        copyRegion.srcSubresource.mipLevel = subresource.myMipLevel;
        copyRegion.srcSubresource.baseArrayLayer = subresource.myArrayIndex;
        copyRegion.srcSubresource.layerCount = 1u;
        copyRegion.srcSubresource.aspectMask = aspectMask;
        copyRegion.dstSubresource.mipLevel = subresource.myMipLevel;
        copyRegion.dstSubresource.baseArrayLayer = subresource.myArrayIndex;
        copyRegion.dstSubresource.layerCount = 1u;
        copyRegion.dstSubresource.aspectMask = aspectMask;
      }

      VkImage dstImage = dstTexture->GetVkData()->myImage;
      VkImage srcImage = srcTexture->GetVkData()->myImage;
      vkCmdCopyImage(myCommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, (uint) copyRegions.size(), copyRegions.data());
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset, uint64 aSize)
  {
    ASSERT(aDstBuffer != aSrcBuffer, "Copying within the same buffer is not supported (same subresource)");

#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferCopy(aDstBuffer->GetProperties(), aDstOffset, aSrcBuffer->GetProperties(), aSrcOffset, aSize);
#endif

    TrackResourceTransition(aSrcBuffer, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackResourceTransition(aDstBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT);

    FlushBarriers();

    const VkBuffer srcBuffer = aSrcBuffer->GetVkData()->myBuffer;
    const VkBuffer dstBuffer = aDstBuffer->GetVkData()->myBuffer;

    VkBufferCopy copyInfo;
    copyInfo.size = aSize;
    copyInfo.srcOffset = aSrcOffset;
    copyInfo.dstOffset = aDstOffset;
    vkCmdCopyBuffer(myCommandBuffer, srcBuffer, dstBuffer, 1u, &copyInfo);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTextureToBuffer(const GpuBuffer* aDstBuffer, uint64 aDstOffset, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const
                                          TextureRegion& aSrcRegion)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureToBufferCopy(aDstBuffer->GetProperties(), aDstOffset, aSrcTexture->GetProperties(), aSrcSubresource, aSrcRegion);
#endif  // FANCY_RENDERER_USE_VALIDATION

    VkBufferImageCopy copyRegion;
    copyRegion.bufferOffset = aDstOffset;
    // Using 0 here will make it fall back to imageExtent. 
    // The buffer is expected to have the copied texture region tightly packed in memory
    copyRegion.bufferRowLength = 0;
    copyRegion.bufferImageHeight = 0;
    copyRegion.imageSubresource.baseArrayLayer = aSrcSubresource.myArrayIndex;
    copyRegion.imageSubresource.layerCount = 1u;
    copyRegion.imageSubresource.aspectMask =
      RenderCore_PlatformVk::ResolveAspectMask(aSrcSubresource.myPlaneIndex, 1u, aSrcTexture->GetProperties().myFormat);
    copyRegion.imageSubresource.mipLevel = aSrcSubresource.myMipLevel;
    copyRegion.imageOffset.x = aSrcRegion.myPos.x;
    copyRegion.imageOffset.y = aSrcRegion.myPos.y;
    copyRegion.imageOffset.z = aSrcRegion.myPos.z;
    copyRegion.imageExtent.width = aSrcRegion.mySize.x;
    copyRegion.imageExtent.height = aSrcRegion.mySize.y;
    copyRegion.imageExtent.depth = aSrcRegion.mySize.z;

    VkBuffer dstBuffer = static_cast<const GpuBufferVk*>(aDstBuffer)->GetData()->myBuffer;
    VkImage srcImage = static_cast<const TextureVk*>(aSrcTexture)->GetData()->myImage;

    const VkImageLayout srcImageLayout = Priv_CommandListVk::locResolveImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, aSrcTexture, SubresourceRange(aSrcSubresource));
    TrackSubresourceTransition(aSrcTexture, SubresourceRange(aSrcSubresource), VK_ACCESS_TRANSFER_READ_BIT, srcImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackResourceTransition(aDstBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();
    
    vkCmdCopyImageToBuffer(myCommandBuffer, srcImage, srcImageLayout, dstBuffer, 1u, &copyRegion);   
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource,
                                  const TextureRegion& aDstRegion, const Texture* aSrcTexture, const SubresourceLocation& aSrcSubresource, const
                                  TextureRegion& aSrcRegion)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateTextureCopy(aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcTexture->GetProperties(), aSrcSubresource, aSrcRegion);
#endif

    VkImageCopy copyRegion;
    copyRegion.extent.width = aDstRegion.mySize.x;
    copyRegion.extent.height = aDstRegion.mySize.y;
    copyRegion.extent.depth = aDstRegion.mySize.z;
    copyRegion.dstOffset.x = aDstRegion.myPos.x;
    copyRegion.dstOffset.y = aDstRegion.myPos.y;
    copyRegion.dstOffset.z = aDstRegion.myPos.z;
    copyRegion.srcOffset.x = aSrcRegion.myPos.x;
    copyRegion.srcOffset.y = aSrcRegion.myPos.y;
    copyRegion.srcOffset.z = aSrcRegion.myPos.z;
    copyRegion.srcSubresource.mipLevel = aSrcSubresource.myMipLevel;
    copyRegion.srcSubresource.baseArrayLayer = aSrcSubresource.myArrayIndex;
    copyRegion.srcSubresource.layerCount = 1u;
    copyRegion.srcSubresource.aspectMask = RenderCore_PlatformVk::ResolveAspectMask(aSrcSubresource.myPlaneIndex, 1u, aSrcTexture->GetProperties().myFormat);
    copyRegion.dstSubresource.mipLevel = aDstSubresource.myMipLevel;
    copyRegion.dstSubresource.baseArrayLayer = aDstSubresource.myArrayIndex;
    copyRegion.dstSubresource.layerCount = 1u;
    copyRegion.dstSubresource.aspectMask = RenderCore_PlatformVk::ResolveAspectMask(aDstSubresource.myPlaneIndex, 1u, aDstTexture->GetProperties().myFormat);

    VkImage dstImage = static_cast<const TextureVk*>(aDstTexture)->GetData()->myImage;
    VkImage srcImage = static_cast<const TextureVk*>(aSrcTexture)->GetData()->myImage;

    const VkImageLayout srcImageLayout = Priv_CommandListVk::locResolveImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, aSrcTexture, SubresourceRange(aSrcSubresource));
    const VkImageLayout dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;

    TrackSubresourceTransition(aSrcTexture, SubresourceRange(aSrcSubresource), VK_ACCESS_TRANSFER_READ_BIT, srcImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackSubresourceTransition(aDstTexture, SubresourceRange(aDstSubresource), VK_ACCESS_TRANSFER_WRITE_BIT, dstImageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();

    vkCmdCopyImage(myCommandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, 1u, &copyRegion);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyBufferToTexture(const Texture* aDstTexture, const SubresourceLocation& aDstSubresource,
                                          const TextureRegion& aDstRegion, const GpuBuffer* aSrcBuffer, uint64 aSrcOffset)
  {
#if FANCY_RENDERER_USE_VALIDATION
    ValidateBufferToTextureCopy(aDstTexture->GetProperties(), aDstSubresource, aDstRegion, aSrcBuffer->GetProperties(), aSrcOffset);
#endif

    VkBufferImageCopy copyRegion;
    copyRegion.bufferOffset = aSrcOffset;
    // Using 0 here will make it fall back to imageExtent. 
    // The buffer is expected to have the copied texture region tightly packed in memory
    copyRegion.bufferRowLength = 0u;
    copyRegion.bufferImageHeight = 0u;
    copyRegion.imageSubresource.baseArrayLayer = aDstSubresource.myArrayIndex;
    copyRegion.imageSubresource.layerCount = 1u;
    copyRegion.imageSubresource.aspectMask = 
      RenderCore_PlatformVk::ResolveAspectMask(aDstSubresource.myPlaneIndex, 1u, aDstTexture->GetProperties().myFormat);
    copyRegion.imageSubresource.mipLevel = aDstSubresource.myMipLevel;
    copyRegion.imageOffset.x = aDstRegion.myPos.x;
    copyRegion.imageOffset.y = aDstRegion.myPos.y;
    copyRegion.imageOffset.z = aDstRegion.myPos.z;
    copyRegion.imageExtent.width = aDstRegion.mySize.x;
    copyRegion.imageExtent.height = aDstRegion.mySize.y;
    copyRegion.imageExtent.depth = aDstRegion.mySize.z;

    VkImage dstImage = static_cast<const TextureVk*>(aDstTexture)->GetData()->myImage;
    VkBuffer srcBuffer = static_cast<const GpuBufferVk*>(aSrcBuffer)->GetData()->myBuffer;

    const VkImageLayout imageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;  // Texture is expected in the WRITE_COPY_DST state here
    TrackResourceTransition(aSrcBuffer, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT);
    TrackSubresourceTransition(aDstTexture, SubresourceRange(aDstSubresource), VK_ACCESS_TRANSFER_WRITE_BIT, imageLayout, VK_PIPELINE_STAGE_TRANSFER_BIT);
    FlushBarriers();
    
    vkCmdCopyBufferToImage(myCommandBuffer, srcBuffer, dstImage, imageLayout, 1u, &copyRegion);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::PostExecute(uint64 aFenceVal)
  {
    CommandList::PostExecute(aFenceVal);

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    
    platformVk->ReleaseCommandBuffer(myCommandBuffer, myCommandListType, aFenceVal);
    myCommandBuffer = nullptr;

    for (uint i = 0u; i < (uint) myUsedDescriptorPools.size(); ++i)
      platformVk->FreeDescriptorPool(myUsedDescriptorPools[i], aFenceVal);
    myUsedDescriptorPools.clear();

    for (uint i = 0u; i < (uint) myTempBufferViews.size(); ++i)
      myTempBufferViews[i].second = glm::max(myTempBufferViews[i].second, aFenceVal);
  
    ClearResourceBindings();  // Clear needed here since the descriptor pools are also being freed above

    myLocalHazardData.clear();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::PreBegin()
  {
    CommandList::PreBegin();

    myRenderPass = nullptr;
    myFramebuffer = nullptr;
    myFramebufferRes = glm::uvec2(0u, 0u);
    myPendingBufferBarriers.clear();
    myPendingImageBarriers.clear();
    myLocalHazardData.clear();
    ClearResourceBindings();

    RenderCore_PlatformVk* platformVk = RenderCore::GetPlatformVk();
    myCommandBuffer = platformVk->GetNewCommandBuffer(myCommandListType);
    ASSERT_VK_RESULT(vkResetCommandBuffer(myCommandBuffer, 0u));

    BeginCommandBuffer();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::FlushBarriers()
  {
    if (myPendingBufferBarriers.empty() && myPendingImageBarriers.empty())
      return;

    // ASSERT(myPendingBarrierSrcStageMask != 0u && myPendingBarrierDstStageMask != 0u);

    VkImageMemoryBarrier imageBarriersVk[kNumCachedBarriers];
    VkBufferMemoryBarrier bufferBarriersVk[kNumCachedBarriers];

    for (uint i = 0u, e = (uint) myPendingImageBarriers.size(); i < e; ++i)
    {
      VkImageMemoryBarrier& imageBarrierVk = imageBarriersVk[i];
      const ImageMemoryBarrierData& pendingBarrier = myPendingImageBarriers[i];

      imageBarrierVk.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
      imageBarrierVk.pNext = nullptr;
      imageBarrierVk.subresourceRange = RenderCore_PlatformVk::ResolveSubresourceRange(pendingBarrier.mySubresourceRange, pendingBarrier.myFormat);
      imageBarrierVk.image = pendingBarrier.myImage;
      imageBarrierVk.srcAccessMask = pendingBarrier.mySrcAccessMask;
      imageBarrierVk.dstAccessMask = pendingBarrier.myDstAccessMask;
      imageBarrierVk.oldLayout = pendingBarrier.mySrcLayout;
      imageBarrierVk.newLayout = pendingBarrier.myDstLayout;
      imageBarrierVk.srcQueueFamilyIndex = pendingBarrier.mySrcQueueFamilyIndex;
      imageBarrierVk.dstQueueFamilyIndex = pendingBarrier.myDstQueueFamilyIndex;
    }

    for (uint i = 0u, e = (uint) myPendingBufferBarriers.size(); i < e; ++i)
    {
      VkBufferMemoryBarrier& bufferBarrierVk = bufferBarriersVk[i];
      const BufferMemoryBarrierData& pendingBarrier = myPendingBufferBarriers[i];

      bufferBarrierVk.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
      bufferBarrierVk.pNext = nullptr;
      bufferBarrierVk.buffer = pendingBarrier.myBuffer;
      bufferBarrierVk.offset = 0u;
      bufferBarrierVk.size = pendingBarrier.myBufferSize;
      bufferBarrierVk.srcAccessMask = pendingBarrier.mySrcAccessMask;
      bufferBarrierVk.dstAccessMask = pendingBarrier.myDstAccessMask;
      bufferBarrierVk.srcQueueFamilyIndex = pendingBarrier.mySrcQueueFamilyIndex;
      bufferBarrierVk.dstQueueFamilyIndex = pendingBarrier.myDstQueueFamilyIndex;
    }

    // TODO: Make this more optimial. BOTTOM_OF_PIPE->TOP_OF_PIPE will stall much more than necessary in most cases. But the last writing pipeline stage needs to be tracked per subresource to correctly do this
    VkPipelineStageFlags srcStageMask = myCommandListType == CommandListType::Graphics ? Priv_CommandListVk::locPipelineMaskGraphics : Priv_CommandListVk::locPipelineMaskCompute;
    VkPipelineStageFlags dstStageMask = myCommandListType == CommandListType::Graphics ? Priv_CommandListVk::locPipelineMaskGraphics : Priv_CommandListVk::locPipelineMaskCompute;

    const VkDependencyFlags dependencyFlags = 0u;
    vkCmdPipelineBarrier(myCommandBuffer, srcStageMask, dstStageMask, 
      dependencyFlags, 0u, nullptr, 
      (uint) myPendingBufferBarriers.size(), bufferBarriersVk, 
      (uint) myPendingImageBarriers.size(), imageBarriersVk);

    myPendingBufferBarriers.clear();
    myPendingImageBarriers.clear();
    
    myPendingBarrierSrcStageMask = 0u;
    myPendingBarrierDstStageMask = 0u;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::SetShaderPipelineInternal(const ShaderPipeline* aPipeline, bool& aHasPipelineChangedOut)
  {
    CommandList::SetShaderPipelineInternal(aPipeline, aHasPipelineChangedOut);

    const ShaderPipelineVk* pipelineVk = static_cast<const ShaderPipelineVk*>(aPipeline);
    const PipelineLayoutVk* pipelineLayout = pipelineVk->GetPipelineLayout();

    if (myPipelineLayout != pipelineLayout->myPipelineLayout)
    {
      myPipelineLayout = pipelineLayout->myPipelineLayout;
      myPipelineLayoutBindings.reset(new PipelineLayoutBindingsVk(*pipelineLayout));
    }
  }
//---------------------------------------------------------------------------//
  const ShaderPipelineVk* CommandListVk::GetShaderPipeline() const
  {
    if (myCurrentContext == CommandListType::Graphics)
      return static_cast<const ShaderPipelineVk*>(myGraphicsPipelineState.myShaderPipeline);

    return static_cast<const ShaderPipelineVk*>(myComputePipelineState.myShaderPipeline);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindVertexBuffers(const GpuBuffer** someBuffers, uint64* someOffsets, uint64* /*someSizes*/, uint aNumBuffers)
  {
    const Shader* vertexShader = myGraphicsPipelineState.myShaderPipeline ? myGraphicsPipelineState.myShaderPipeline->GetShader(ShaderStage::VERTEX) : nullptr;
    const VertexInputLayout* shaderInputLayout = vertexShader ? vertexShader->myDefaultVertexInputLayout.get() : nullptr;
    const VertexInputLayout* inputLayout = myGraphicsPipelineState.myVertexInputLayout ? myGraphicsPipelineState.myVertexInputLayout : shaderInputLayout;

    ASSERT(inputLayout && inputLayout->myProperties.myBufferBindings.size() == aNumBuffers);
    
    eastl::fixed_vector<VkBuffer, 4> vkBuffers;
    for (uint i = 0u; i < aNumBuffers; ++i)
    {
      TrackResourceTransition(someBuffers[i], VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
      const GpuResourceDataVk* resourceDataVk = static_cast<const GpuBufferVk*>(someBuffers[i])->GetData();
      vkBuffers.push_back(resourceDataVk->myBuffer);
    }

    vkCmdBindVertexBuffers(myCommandBuffer, 0u, aNumBuffers, vkBuffers.data(), someOffsets);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindIndexBuffer(const GpuBuffer* aBuffer, uint anIndexSize, uint64 anOffset, uint64 /*aSize*/)
  {
    ASSERT(anIndexSize == 2u || anIndexSize == 4u);
    const VkIndexType indexType = anIndexSize == 2u ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;

    TrackResourceTransition(aBuffer, VK_ACCESS_INDEX_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);

    const GpuResourceDataVk* resourceDataVk = static_cast<const GpuBufferVk*>(aBuffer)->GetData();
    vkCmdBindIndexBuffer(myCommandBuffer, resourceDataVk->myBuffer, anOffset, indexType);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Render(uint aNumIndicesPerInstance, uint aNumInstances, uint aStartIndex, uint aBaseVertex, uint aStartInstance)
  {
    FlushBarriers();
    ApplyViewportAndClipRect();
    ApplyRenderTargets();
    ApplyGraphicsPipelineState();
    ApplyResourceState();

    VkRenderPassBeginInfo renderPassBegin;
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.pNext = nullptr;
    renderPassBegin.renderArea.offset = { 0, 0 };
    renderPassBegin.renderArea.extent = { myFramebufferRes.x, myFramebufferRes.y };
    renderPassBegin.clearValueCount = 0u;
    renderPassBegin.pClearValues = nullptr;
    renderPassBegin.renderPass = myRenderPass;
    renderPassBegin.framebuffer = myFramebuffer;

    vkCmdBeginRenderPass(myCommandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdDrawIndexed(myCommandBuffer, aNumIndicesPerInstance, aNumInstances, aStartIndex, aBaseVertex, aStartInstance);

    vkCmdEndRenderPass(myCommandBuffer);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::UpdateTextureData(const Texture* aDstTexture, const SubresourceRange& aSubresourceRange, const TextureSubData* someDatas, uint aNumDatas)
  {
    const uint numSubresources = aSubresourceRange.GetNumSubresources();
    ASSERT(aNumDatas == numSubresources);

    const TextureProperties& texProps = aDstTexture->GetProperties();
    const DataFormatInfo& formatInfo = DataFormatInfo::GetFormatInfo(texProps.myFormat);
    const RenderPlatformCaps& caps = RenderCore::GetPlatformCaps();
    
    uint64* rowSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64* bufferRowSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64* bufferSliceSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint64* bufferSubresourceSizes = (uint64*)alloca(sizeof(uint64) * numSubresources);
    uint* heights = (uint*)alloca(sizeof(uint) * numSubresources);
    uint* depths = (uint*)alloca(sizeof(uint) * numSubresources);

    uint i = 0;
    uint64 requiredBufferSize = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(), end = aSubresourceRange.End(); it != end; ++it)
    {
      const SubresourceLocation subResource = *it;

      uint width, height, depth;
      texProps.GetSize(subResource.myMipLevel, width, height, depth);

      const uint64 rowSize = width * formatInfo.myCopyableSizePerPlane[subResource.myPlaneIndex];
      const uint64 alignedRowSize = MathUtil::Align(rowSize, caps.myTextureRowAlignment);
      const uint64 alignedSliceSize = alignedRowSize * height;
      const uint64 alignedSubresourceSize = MathUtil::Align(alignedSliceSize * depth, caps.myTextureSubresourceBufferAlignment);
      requiredBufferSize += alignedSubresourceSize;
  
      rowSizes[i] = rowSize;
      bufferRowSizes[i] = alignedRowSize;
      bufferSliceSizes[i] = alignedSliceSize;
      bufferSubresourceSizes[i] = alignedSubresourceSize;
      heights[i] = height;
      depths[i] = depth;
      ++i;
    }

    uint64 uploadBufferOffset;
    uint8* uploadBufferData;
    const GpuBuffer* uploadBuffer = GetMappedBuffer(uploadBufferOffset, GpuBufferUsage::STAGING_UPLOAD, &uploadBufferData, requiredBufferSize);
    ASSERT(uploadBuffer != nullptr);
    ASSERT(uploadBufferData != nullptr);

    uint8* dstSubresourceData = uploadBufferData;
    for (i = 0; i < aNumDatas; ++i)
    {
      const TextureSubData& srcData = someDatas[i];
      ASSERT(rowSizes[i] == srcData.myRowSizeBytes);

      const uint depth = depths[i];
      const uint height = heights[i];

      const uint64 dstRowSize = bufferRowSizes[i];
      const uint64 dstSliceSize = bufferSliceSizes[i];
      const uint64 dstSubresourceSize = bufferSubresourceSizes[i];

      uint8* dstSliceData = dstSubresourceData;
      const uint8* srcSliceData = srcData.myData;
      for (uint d = 0; d < depth; ++d)
      {
        uint8* dstRowData = dstSliceData;
        const uint8* srcRowData = srcSliceData;
        for (uint h = 0; h < height; ++h)
        {
          memcpy(dstRowData, srcRowData, srcData.myRowSizeBytes);

          dstRowData += dstRowSize;
          srcRowData += srcData.myRowSizeBytes;
        }

        dstSliceData += dstSliceSize;
        srcSliceData += srcData.mySliceSizeBytes;
      }

      dstSubresourceData += dstSubresourceSize;
    }

    i = 0;
    uint64 bufferOffset = uploadBufferOffset;
    for (SubresourceIterator subIter = aSubresourceRange.Begin(), e = aSubresourceRange.End(); subIter != e; ++subIter)
    {
      const SubresourceLocation dstLocation = *subIter;
      CommandList::CopyBufferToTexture(aDstTexture, dstLocation, uploadBuffer, bufferOffset);

      bufferOffset += bufferSubresourceSizes[i++];
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindResourceView(const GpuResourceView* aView, uint64 aNameHash, uint anArrayIndex /* = 0u */)
  {
    const ShaderResourceInfoVk* resourceInfo = FindShaderResourceInfo(aNameHash);
    if (!resourceInfo)
      return;

    const VkPipelineStageFlags pipelineStage = myCurrentContext == CommandListType::Compute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    const GpuResourceViewDataVk& viewDataVk = eastl::any_cast<const GpuResourceViewDataVk&>(aView->myNativeData);
    const GpuResourceDataVk* resourceDataVk = aView->myResource->GetVkData();

    VkImageView imageView = nullptr;
    VkImageLayout imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    VkBufferView bufferView = nullptr;
    VkBuffer buffer = nullptr;
    uint64 bufferOffset = 0ull;
    uint64 bufferSize = 0ull;
    switch (resourceInfo->myType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER: ASSERT(false); break;  // Needs to be handled in BindSampler()
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break;  // Not supported in HLSL, so this should never happen
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
    {
      ASSERT(aView->myType == GpuResourceViewType::SRV);
      ASSERT(aView->myResource->GetType() == GpuResourceType::TEXTURE);
      imageLayout = Priv_CommandListVk::locResolveImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, aView->GetResource(), aView->GetSubresourceRange());
      imageView = viewDataVk.myView.myImage;
      TrackSubresourceTransition(aView->GetResource(), aView->GetSubresourceRange(), VK_ACCESS_SHADER_READ_BIT, imageLayout, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
    {
      ASSERT(aView->myType == GpuResourceViewType::UAV);
      ASSERT(aView->myResource->GetType() == GpuResourceType::TEXTURE);
      imageLayout = VK_IMAGE_LAYOUT_GENERAL;
      imageView = viewDataVk.myView.myImage;
      TrackSubresourceTransition(aView->GetResource(), aView->GetSubresourceRange(), VK_ACCESS_SHADER_WRITE_BIT, imageLayout, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::UAV);
      ASSERT(aView->myResource->GetType() == GpuResourceType::BUFFER);
      const GpuBufferViewVk* bufferViewVk = static_cast<const GpuBufferViewVk*>(aView);
      ASSERT(bufferViewVk->GetBufferView() != nullptr);
      bufferView = bufferViewVk->GetBufferView();
      TrackResourceTransition(aView->GetResource(), VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::UAV);
      ASSERT(aView->myResource->GetType() == GpuResourceType::BUFFER);
      buffer = resourceDataVk->myBuffer;
      bufferOffset = static_cast<const GpuBufferView*>(aView)->GetProperties().myOffset;
      bufferSize = static_cast<const GpuBufferView*>(aView)->GetProperties().mySize;
      TrackResourceTransition(aView->GetResource(), VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::SRV);
      ASSERT(aView->myResource->GetType() == GpuResourceType::BUFFER);
      const GpuBufferViewVk* bufferViewVk = static_cast<const GpuBufferViewVk*>(aView);
      ASSERT(bufferViewVk->GetBufferView() != nullptr);
      bufferView = bufferViewVk->GetBufferView();
      TrackResourceTransition(aView->GetResource(), VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    {
      ASSERT(aView->myType == GpuResourceViewType::CBV);
      ASSERT(aView->myResource->GetType() == GpuResourceType::BUFFER);
      buffer = resourceDataVk->myBuffer;
      bufferOffset = static_cast<const GpuBufferView*>(aView)->GetProperties().myOffset;
      bufferSize = static_cast<const GpuBufferView*>(aView)->GetProperties().mySize;
      TrackResourceTransition(aView->GetResource(), VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      break;
    default: ASSERT(false);
    }

    BindInternal(*resourceInfo, anArrayIndex, bufferView, buffer, bufferOffset, bufferSize, imageView, imageLayout, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindBuffer(const GpuBuffer* aBuffer, const GpuBufferViewProperties& someViewProperties, uint64 aNameHash, uint anArrayIndex/* = 0u*/)
  {
    const ShaderResourceInfoVk* resourceInfo = FindShaderResourceInfo(aNameHash);
    if (!resourceInfo)
      return;

    const VkPipelineStageFlags pipelineStage = myCurrentContext == CommandListType::Compute ? VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    const GpuBufferProperties& bufferProps = aBuffer->GetProperties();

    bool needsTempBufferView = false;
    switch (resourceInfo->myType)
    {
    case VK_DESCRIPTOR_TYPE_SAMPLER: ASSERT(false); break;  // Needs to be handled in BindSampler()
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break;  // Not supported in HLSL, so this should never happen
    case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE: ASSERT(false); break;
    case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE: ASSERT(false); break;
    case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
    {
      ASSERT(someViewProperties.myIsShaderWritable && bufferProps.myIsShaderWritable && (bufferProps.myBindFlags & (uint) GpuBufferBindFlags::SHADER_BUFFER));
      needsTempBufferView = true;
      TrackResourceTransition(aBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
    {
      ASSERT(someViewProperties.myIsShaderWritable && bufferProps.myIsShaderWritable && (bufferProps.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER));
      TrackResourceTransition(aBuffer, VK_ACCESS_SHADER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      TrackResourceTransition(aBuffer, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
      break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
    {
      ASSERT(!someViewProperties.myIsShaderWritable && (bufferProps.myBindFlags & (uint)GpuBufferBindFlags::SHADER_BUFFER));
      needsTempBufferView = true;
      TrackResourceTransition(aBuffer, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
    {
      ASSERT(!someViewProperties.myIsShaderWritable && someViewProperties.myIsConstantBuffer && (bufferProps.myBindFlags & (uint)GpuBufferBindFlags::CONSTANT_BUFFER));
      TrackResourceTransition(aBuffer, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
      ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
      TrackResourceTransition(aBuffer, VK_ACCESS_UNIFORM_READ_BIT, VK_IMAGE_LAYOUT_UNDEFINED, pipelineStage);
      break;
    default: ASSERT(false);
    }

    VkBufferView bufferView = nullptr;
    if (needsTempBufferView)
    {
      bufferView = GpuBufferViewVk::CreateVkBufferView(aBuffer, someViewProperties);
      myTempBufferViews.push_back({ bufferView, 0u });
    }

    BindInternal(*resourceInfo, anArrayIndex, bufferView, static_cast<const GpuBufferVk*>(aBuffer)->GetData()->myBuffer, 
      someViewProperties.myOffset, someViewProperties.mySize, nullptr, VK_IMAGE_LAYOUT_UNDEFINED, nullptr);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindSampler(const TextureSampler* aSampler, uint64 aNameHash, uint anArrayIndex/* = 0u*/)
  {
    const ShaderResourceInfoVk* resourceInfo = FindShaderResourceInfo(aNameHash);
    if (!resourceInfo)
      return;
  
    ASSERT(resourceInfo->myType == VK_DESCRIPTOR_TYPE_SAMPLER);

    VkSampler sampler = static_cast<const TextureSamplerVk*>(aSampler)->GetVkSampler();
    BindInternal(*resourceInfo, anArrayIndex, nullptr, nullptr, 0ull, 0ull, nullptr, VK_IMAGE_LAYOUT_UNDEFINED, sampler);
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListVk::BeginQuery(GpuQueryType aType)
  {
    ASSERT(aType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp");

    const GpuQuery query = AllocateQuery(aType);
    GpuQueryHeap* heap = RenderCore::GetQueryHeap(aType);
    const GpuQueryHeapVk* queryHeapVk = static_cast<const GpuQueryHeapVk*>(heap);

    VkQueryControlFlags flags = 0u;
    if (aType == GpuQueryType::OCCLUSION)
      flags |= VK_QUERY_CONTROL_PRECISE_BIT;

    vkCmdBeginQuery(myCommandBuffer, queryHeapVk->GetQueryPool(), query.myIndexInHeap, flags);

    return query;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::EndQuery(const GpuQuery& aQuery)
  {
    ASSERT(aQuery.myFrame == Time::ourFrameIdx);
    ASSERT(aQuery.myType != GpuQueryType::TIMESTAMP, "Timestamp-queries should be used with InsertTimestamp");
    ASSERT(aQuery.myIsOpen);

    aQuery.myIsOpen = false;

    const GpuQueryType queryType = aQuery.myType;
    GpuQueryHeap* heap = RenderCore::GetQueryHeap(queryType);
    const GpuQueryHeapVk* queryHeapVk = static_cast<const GpuQueryHeapVk*>(heap);

    vkCmdEndQuery(myCommandBuffer, queryHeapVk->GetQueryPool(), aQuery.myIndexInHeap);
  }
//---------------------------------------------------------------------------//
  GpuQuery CommandListVk::InsertTimestamp()
  {
    const GpuQuery query = AllocateQuery(GpuQueryType::TIMESTAMP);
    query.myIsOpen = false;

    GpuQueryHeap* heap = RenderCore::GetQueryHeap(GpuQueryType::TIMESTAMP);
    const GpuQueryHeapVk* queryHeapVk = static_cast<const GpuQueryHeapVk*>(heap);

    VkPipelineStageFlagBits stageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    vkCmdWriteTimestamp(myCommandBuffer, stageMask, queryHeapVk->GetQueryPool(), query.myIndexInHeap);

    return query;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::CopyQueryDataToBuffer(const GpuQueryHeap* aQueryHeap, const GpuBuffer* aBuffer, uint aFirstQueryIndex, uint aNumQueries, uint64 aBufferOffset)
  {
    const GpuQueryHeapVk* queryHeapVk = static_cast<const GpuQueryHeapVk*>(aQueryHeap);
    GpuResourceDataVk* bufferDataVk = aBuffer->GetVkData();

    TrackResourceTransition(aBuffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_UNDEFINED, VK_PIPELINE_STAGE_TRANSFER_BIT, false);
    FlushBarriers();

    const uint64 stride = RenderCore::GetPlatformVk()->GetQueryTypeDataSize(aQueryHeap->myType);
    const VkQueryResultFlags resultFlags = stride == sizeof(uint64) ? VK_QUERY_RESULT_64_BIT : 0u;
    vkCmdCopyQueryPoolResults(myCommandBuffer, queryHeapVk->GetQueryPool(), aFirstQueryIndex, aNumQueries, bufferDataVk->myBuffer, aBufferOffset, stride, resultFlags);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::TransitionResource(const GpuResource* aResource, const SubresourceRange& aSubresourceRange, ResourceTransition aTransition, uint /* someUsageFlags = 0u*/)
  {
    VkImageLayout newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    VkAccessFlags newAccessFlags = 0u;
    VkPipelineStageFlags newPipelineStageFlags = 0u;
    bool toSharedRead = false;

    switch (aTransition)
    {
    case ResourceTransition::TO_SHARED_CONTEXT_READ:
      newAccessFlags = aResource->GetVkData()->myHazardData.myReadAccessMask;
      newPipelineStageFlags = Priv_CommandListVk::locPipelineMaskGraphics;
      if (aResource->IsTexture())
        newLayout = VK_IMAGE_LAYOUT_GENERAL;

      toSharedRead = true;
      break;
    default: ASSERT(false);
    }

    TrackSubresourceTransition(aResource, aSubresourceRange, newAccessFlags, newLayout, newPipelineStageFlags, toSharedRead);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ResourceUAVbarrier(const GpuResource** someResources, uint aNumResources)
  {
    VK_MISSING_IMPLEMENTATION();
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Close()
  {
    if (myIsOpen)
      ASSERT_VK_RESULT(vkEndCommandBuffer(myCommandBuffer));

    myIsOpen = false;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::Dispatch(const glm::int3& aNumThreads)
  {
    FlushBarriers();

    ApplyComputePipelineState();
    ApplyResourceState();
    ASSERT(myComputePipelineState.myShaderPipeline != nullptr);

    const Shader* shader = myComputePipelineState.myShaderPipeline->GetShader(ShaderStage::COMPUTE);
    ASSERT(shader != nullptr);

    const glm::int3& numGroupThreads = shader->GetProperties().myNumGroupThreads;
    const glm::int3 numGroups = glm::max(glm::int3(1), aNumThreads / numGroupThreads);
    vkCmdDispatch(myCommandBuffer, (uint)numGroups.x, (uint)numGroups.y, (uint)numGroups.z);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::TrackResourceTransition(const GpuResource* aResource, VkAccessFlags aNewAccessFlags, VkImageLayout aNewImageLayout, VkPipelineStageFlags aNewPipelineStageFlags, bool aToSharedReadState)
  {
    TrackSubresourceTransition(aResource, aResource->GetSubresources(), aNewAccessFlags, aNewImageLayout, aNewPipelineStageFlags, aToSharedReadState);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::TrackSubresourceTransition(
    const GpuResource* aResource, 
    const SubresourceRange& aSubresourceRange, 
    VkAccessFlags aNewAccessFlags, 
    VkImageLayout aNewImageLayout, 
    VkPipelineStageFlags aNewPipelineStageFlags, 
    bool aToSharedReadState)
  {
    if (aResource->IsBuffer())
    {
      ASSERT(aSubresourceRange.GetNumSubresources() == 1u);
      ASSERT(aNewImageLayout == VK_IMAGE_LAYOUT_UNDEFINED);
    }

    VkAccessFlags dstAccessFlags = Priv_CommandListVk::locResolveValidateDstAccessMask(aResource, myCommandListType, aNewAccessFlags);

    if (!Priv_CommandListVk::locValidateDstImageLayout(aResource, aNewImageLayout))
      return;

    uint numPossibleSubresourceTransitions = 0u;
    eastl::fixed_vector<bool, 16> subresourceTransitionPossible(aSubresourceRange.GetNumSubresources(), false);
    uint i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it)
    {
      const bool transitionPossible = ValidateSubresourceTransition(aResource, aResource->GetSubresourceIndex(*it), dstAccessFlags, aNewImageLayout);
      if (transitionPossible)
        ++numPossibleSubresourceTransitions;
      subresourceTransitionPossible[i++] = transitionPossible;
    }

    const bool canEarlyOut = !aToSharedReadState;
    if (numPossibleSubresourceTransitions == 0u && canEarlyOut)
      return;

    const bool dstIsRead = (dstAccessFlags & Priv_CommandListVk::locAccessMaskRead) == dstAccessFlags;

    LocalHazardData* localData = nullptr;
    auto it = myLocalHazardData.find(aResource);
    if (it == myLocalHazardData.end())  // We don't have a local record of this resource yet
    {
      localData = &myLocalHazardData[aResource];
      localData->mySubresources.resize(aResource->mySubresources.GetNumSubresources());
    }
    else
    {
      localData = &it->second;
    }

    bool canTransitionAllSubresources = numPossibleSubresourceTransitions == aResource->mySubresources.GetNumSubresources();
    if (canTransitionAllSubresources)
    {
      const VkAccessFlags firstSrcAccessMask = localData->mySubresources[0].myAccessFlags;
      const VkImageLayout firstSrcImageLayout = localData->mySubresources[0].myImageLayout;

      for (uint sub = 0u; canTransitionAllSubresources && sub < (uint) localData->mySubresources.size(); ++sub)
      {
        canTransitionAllSubresources &= localData->mySubresources[sub].myWasUsed
          && localData->mySubresources[sub].myAccessFlags == firstSrcAccessMask
          && localData->mySubresources[sub].myImageLayout == firstSrcImageLayout;
      }
    }

    if (canTransitionAllSubresources)  // The simple case: We can transition all subresources in one barrier
    {
      if (aResource->IsBuffer())
      {
        BufferMemoryBarrierData barrier;
        barrier.myBuffer = eastl::any_cast<const GpuResourceDataVk&>(aResource->myNativeData).myBuffer;
        barrier.myBufferSize = static_cast<const GpuBuffer*>(aResource)->GetByteSize();
        barrier.myDstAccessMask = dstAccessFlags;
        barrier.mySrcAccessMask = localData->mySubresources[0].myAccessFlags;
        AddBarrier(barrier);

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        if (RenderCore::ourDebugLogResourceBarriers)
          LOG_DEBUG("Transition buffer %s: %s -> %s", aResource->GetName(), 
            DebugUtilsVk::AccessMaskToString(barrier.mySrcAccessMask).c_str(),
            DebugUtilsVk::AccessMaskToString(barrier.myDstAccessMask).c_str());
#endif
      }
      else
      {
        const DataFormat format = static_cast<const Texture*>(aResource)->GetProperties().myFormat;

        ImageMemoryBarrierData barrier;
        barrier.myImage = eastl::any_cast<const GpuResourceDataVk&>(aResource->myNativeData).myImage;
        barrier.myFormat = static_cast<const Texture*>(aResource)->GetProperties().myFormat;
        barrier.myDstAccessMask = dstAccessFlags;
        barrier.myDstLayout = aNewImageLayout;
        barrier.mySrcAccessMask = localData->mySubresources[0].myAccessFlags;
        barrier.mySrcLayout = localData->mySubresources[0].myImageLayout;
        barrier.mySubresourceRange = aResource->GetSubresources();
        AddBarrier(barrier);

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        if (RenderCore::ourDebugLogResourceBarriers)
          LOG_DEBUG("Transition image %s (all subresources): %s -> %s / %s -> %s", aResource->GetName(),
            DebugUtilsVk::AccessMaskToString(barrier.mySrcAccessMask).c_str(),
            DebugUtilsVk::AccessMaskToString(barrier.myDstAccessMask).c_str(),
            DebugUtilsVk::ImageLayoutToString(barrier.mySrcLayout).c_str(),
            DebugUtilsVk::ImageLayoutToString(barrier.myDstLayout).c_str());
#endif
      }
    }
    else if (aResource->IsTexture())
    {
      // Create a barrier for each subresource that needs to be transitioned. In a later step, the barriers are merged if possible in order to cover as many subresources as possible with as few barriers as possible

      eastl::fixed_vector<ImageMemoryBarrierData, 16> potentialSubresourceBarriers;

      ImageMemoryBarrierData imageBarrier;
      imageBarrier.myImage = eastl::any_cast<const GpuResourceDataVk&>(aResource->myNativeData).myImage;
      imageBarrier.myFormat = static_cast<const Texture*>(aResource)->GetProperties().myFormat;
      imageBarrier.myDstAccessMask = dstAccessFlags;
      imageBarrier.myDstLayout = aNewImageLayout;

      i = 0u;
      for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i)
      {
        if (!subresourceTransitionPossible[i])
          continue;

        const uint subresourceIndex = aResource->GetSubresourceIndex(*it);
        SubresourceHazardData& subData = localData->mySubresources[subresourceIndex];
        if (subData.myWasUsed)  // We can only add a barrier if we already know the current state within the command list
        {
          imageBarrier.mySubresourceRange = SubresourceRange(*it);
          imageBarrier.mySrcAccessMask = subData.myAccessFlags;
          imageBarrier.mySrcLayout = subData.myImageLayout;
          potentialSubresourceBarriers.push_back(imageBarrier);

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
          if (RenderCore::ourDebugLogResourceBarriers)
            LOG_DEBUG("Transition image %s (subresource %d): %s -> %s / %s -> %s", aResource->GetName(),
              subresourceIndex,
              DebugUtilsVk::AccessMaskToString(imageBarrier.mySrcAccessMask).c_str(),
              DebugUtilsVk::AccessMaskToString(imageBarrier.myDstAccessMask).c_str(),
              DebugUtilsVk::ImageLayoutToString(imageBarrier.mySrcLayout).c_str(),
              DebugUtilsVk::ImageLayoutToString(imageBarrier.myDstLayout).c_str());
#endif
        }
      }

      // Merge the potential subresource barriers and add them
      if (!potentialSubresourceBarriers.empty())
      {
        ImageMemoryBarrierData currBarrier = potentialSubresourceBarriers[0];
        uint currMaxSubresourceIndex = aResource->GetSubresourceIndex(*currBarrier.mySubresourceRange.Begin());
        for (uint iBarrier = 1u; iBarrier < (uint) potentialSubresourceBarriers.size(); ++iBarrier)
        {
          const ImageMemoryBarrierData& nextBarrier = potentialSubresourceBarriers[iBarrier];
          uint nextSubresourceIndex = aResource->GetSubresourceIndex(*nextBarrier.mySubresourceRange.Begin());

          const bool canBatch = nextSubresourceIndex == currMaxSubresourceIndex + 1u &&
            currBarrier.mySrcAccessMask == nextBarrier.mySrcAccessMask &&
            currBarrier.mySrcLayout == nextBarrier.mySrcLayout;

          if (!canBatch)
          {
            AddBarrier(currBarrier);
            currBarrier = nextBarrier;
          }
          currMaxSubresourceIndex = nextSubresourceIndex;
        }

        AddBarrier(currBarrier);
      }
    }

    // Finally, write the new states into the local subresource records
    i = 0u;
    for (SubresourceIterator it = aSubresourceRange.Begin(); it != aSubresourceRange.End(); ++it, ++i)
    {
      const uint subresourceIndex = aResource->GetSubresourceIndex(*it);
      SubresourceHazardData& subData = localData->mySubresources[subresourceIndex];
      if (aToSharedReadState)
      {
        subData.myWasWritten = false;
        subData.myIsSharedReadState = true;
      }

      if (!subresourceTransitionPossible[i])
        continue;

      if (!subData.myWasUsed)
      {
        subData.myFirstDstAccessFlags = dstAccessFlags;
        subData.myFirstDstImageLayout = aNewImageLayout;

#if FANCY_RENDERER_LOG_RESOURCE_BARRIERS
        if (RenderCore::ourDebugLogResourceBarriers)
        {
          if (aResource->IsBuffer())
            LOG_DEBUG("Open transition buffer %s: ? -> %s", aResource->GetName(), DebugUtilsVk::AccessMaskToString(dstAccessFlags).c_str());
          else
            LOG_DEBUG("Open transition image %s (subresource %d): ? -> %s / ? -> %s", 
              aResource->GetName(), subresourceIndex,
              DebugUtilsVk::AccessMaskToString(dstAccessFlags).c_str(),
              DebugUtilsVk::ImageLayoutToString(aNewImageLayout).c_str());
        }
#endif
      }

      subData.myWasUsed = true;
      subData.myAccessFlags = dstAccessFlags;
      subData.myImageLayout = aNewImageLayout;

      if (!dstIsRead)
      {
        subData.myWasWritten = true;
        subData.myIsSharedReadState = false;
      }
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::AddBarrier(const BufferMemoryBarrierData& aBarrier)
  {
    if (myPendingBufferBarriers.full())
      FlushBarriers();

    myPendingBufferBarriers.push_back(aBarrier);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::AddBarrier(const ImageMemoryBarrierData& aBarrier)
  {
    if (myPendingImageBarriers.full())
      FlushBarriers();

    myPendingImageBarriers.push_back(aBarrier);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ClearResourceBindings()
  {
    if (myPipelineLayoutBindings)
      myPipelineLayoutBindings->Clear();

    for (uint i = 0u; i < (uint)myTempBufferViews.size(); ++i)
      RenderCore::GetPlatformVk()->ReleaseTempBufferView(myTempBufferViews[i].first, myTempBufferViews[i].second);
    myTempBufferViews.clear();
  }
//---------------------------------------------------------------------------//
  bool CommandListVk::ValidateSubresourceTransition(const GpuResource* aResource, uint aSubresourceIndex, VkAccessFlags aDstAccess, VkImageLayout aDstImageLayout)
  {
    const GpuResourceHazardDataVk& globalData = aResource->GetVkData()->myHazardData;

    VkAccessFlags currAccess = globalData.mySubresources[aSubresourceIndex].myAccessMask;
    VkImageLayout currImgLayout = (VkImageLayout) globalData.mySubresources[aSubresourceIndex].myImageLayout;

    CommandListType currGlobalContext = globalData.mySubresources[aSubresourceIndex].myContext;

    auto it = myLocalHazardData.find(aResource);
    const bool hasLocalData = it != myLocalHazardData.end();
    if (hasLocalData)
    {
      currAccess = it->second.mySubresources[aSubresourceIndex].myAccessFlags;
      currImgLayout = it->second.mySubresources[aSubresourceIndex].myImageLayout;
    }

    bool currAccessHasAllDstAccessFlags = (currAccess & aDstAccess) == aDstAccess;
    if (aDstAccess == 0u)
      currAccessHasAllDstAccessFlags = currAccess == 0u;

    bool currImgLayoutIsSame = currImgLayout == aDstImageLayout;

    const bool dstIsRead = (aDstAccess & Priv_CommandListVk::locAccessMaskRead) == aDstAccess;
    bool isInSharedReadState = dstIsRead && currGlobalContext == CommandListType::SHARED_READ;
    if (hasLocalData && it->second.mySubresources[aSubresourceIndex].myWasWritten)
    {
      // The subresource left the shared read state in this command list
      isInSharedReadState = false;
    }

    // We can only truly skip this transition if we already have the resource state on the local timeline. 
   // If the subresource is on the shared read context and the destination is a read state, we can also skip since the subresource is not expected to transition at all
    if ((currAccessHasAllDstAccessFlags && currImgLayoutIsSame) && (hasLocalData || isInSharedReadState))
      return false;
    
    // If we reached this point it means that a state is missing in the curr state and we need to add a barrier for this subresource. However, it is an error to transition 
    // To another read-state if the resource is currently being used by multiple queues. A write-state would take ownership of this resource again however
    if (isInSharedReadState)
    {
      ASSERT(false, "No resource transitions allowed on SHARED_READ context. Resource must be transitioned to a state mask that incorporates all future read-states");
      return false;
    }

    return true;
  }
//---------------------------------------------------------------------------//
  const ShaderResourceInfoVk* CommandListVk::FindShaderResourceInfo(uint64 aNameHash) const
  {
    ASSERT(myGraphicsPipelineState.myShaderPipeline != nullptr || myCurrentContext != CommandListType::Graphics);
    ASSERT(myComputePipelineState.myShaderPipeline != nullptr || myCurrentContext != CommandListType::Compute);

    const ShaderPipelineVk* pipeline = GetShaderPipeline();
    const eastl::vector<ShaderResourceInfoVk>& shaderResources = pipeline->GetResourceInfos();

    auto it = std::find_if(shaderResources.begin(), shaderResources.end(), [aNameHash](const ShaderResourceInfoVk& aResourceInfo) {
      return aResourceInfo.myNameHash == aNameHash;
    });

    if (it == shaderResources.end())
    {
      LOG_WARNING("Resource not found in shader");
      return nullptr;
    }

    const ShaderResourceInfoVk& info = *it;
    return &info;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyViewportAndClipRect()
  {
    if (myViewportDirty)
    {
      VkViewport viewport;
      viewport.x = static_cast<float>(myViewportParams.x);
      viewport.y = static_cast<float>(myViewportParams.y);
      viewport.width = static_cast<float>(myViewportParams.z);
      viewport.height = static_cast<float>(myViewportParams.w);
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      vkCmdSetViewport(myCommandBuffer, 0u, 1u, &viewport);

      myClipRectDirty = true;
      myViewportDirty = false;
    }

    if (myClipRectDirty)
    {
      const int left = static_cast<int>(myClipRect.x);
      const int top = static_cast<int>(myClipRect.y);
      const int right = static_cast<int>(myClipRect.z);
      const int bottom = static_cast<int>(myClipRect.w);

      VkRect2D clipRect;
      clipRect.offset.x = left;
      clipRect.offset.y = top;
      clipRect.extent.width = right - left;
      clipRect.extent.height = bottom - top;

      vkCmdSetScissor(myCommandBuffer, 0u, 1u, &clipRect);

      myClipRectDirty = false;
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyRenderTargets()
  {
    ASSERT(myCurrentContext == CommandListType::Graphics);

    if (!myRenderTargetsDirty)
    {
      ASSERT(myRenderPass != nullptr);
      ASSERT(myFramebuffer != nullptr);
      return;
    }

    myRenderTargetsDirty = false;

    const uint numRtsToSet = myGraphicsPipelineState.myNumRenderTargets;
    ASSERT(numRtsToSet <= RenderConstants::kMaxNumRenderTargets);
    
    for (uint i = 0u; i < numRtsToSet; ++i)
    {
      TextureView* renderTarget = myRenderTargets[i];
      ASSERT(renderTarget != nullptr);

      TrackSubresourceTransition(renderTarget->GetResource(), renderTarget->GetSubresourceRange(), 
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    const bool hasDepthStencilTarget = myDepthStencilTarget != nullptr;
    const TextureViewProperties& dsvProps = myDepthStencilTarget->GetProperties();
    if (hasDepthStencilTarget)
    {
      if (dsvProps.myIsDepthReadOnly == dsvProps.myIsStencilReadOnly)
      {
        VkImageLayout layout = dsvProps.myIsDepthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        layout = Priv_CommandListVk::locResolveImageLayout(layout, myDepthStencilTarget->GetResource(), myDepthStencilTarget->GetSubresourceRange());

        TrackSubresourceTransition(myDepthStencilTarget->GetResource(), myDepthStencilTarget->GetSubresourceRange(),
          dsvProps.myIsDepthReadOnly ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, 
          layout, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
      }
      else
      {
        SubresourceRange range = myDepthStencilTarget->GetSubresourceRange();
        range.myFirstPlane = 0u;
        range.myNumPlanes = 1u;

        VkImageLayout layout = dsvProps.myIsDepthReadOnly ? VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        layout = Priv_CommandListVk::locResolveImageLayout(layout, myDepthStencilTarget->GetResource(), myDepthStencilTarget->GetSubresourceRange());

        TrackSubresourceTransition(myDepthStencilTarget->GetResource(), range,
          dsvProps.myIsDepthReadOnly ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
          layout, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);

        range.myFirstPlane = 1u;

        layout = dsvProps.myIsStencilReadOnly ? VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        layout = Priv_CommandListVk::locResolveImageLayout(layout, myDepthStencilTarget->GetResource(), myDepthStencilTarget->GetSubresourceRange());

        TrackSubresourceTransition(myDepthStencilTarget->GetResource(), range,
          dsvProps.myIsStencilReadOnly ? VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT : VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
          layout, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
      }
    }

    RenderCore_PlatformVk* platform = RenderCore::GetPlatformVk();

    VkRenderPass renderPass = platform->GetRenderPassCache().GetCreate(const_cast<const TextureView**>(myRenderTargets), numRtsToSet, myDepthStencilTarget);

    glm::uvec2 framebufferRes(1u, 1u);
    const TextureView* firstRenderTarget = numRtsToSet > 0 ? myRenderTargets[0] : myDepthStencilTarget;
    if (firstRenderTarget != nullptr)
    {
      const TextureProperties& textureProps = firstRenderTarget->GetTexture()->GetProperties();
      framebufferRes.x = textureProps.myWidth;
      framebufferRes.y = textureProps.myHeight;
    }

    VkFramebuffer framebuffer = platform->GetFrameBufferCache().GetCreate(const_cast<const TextureView**>(myRenderTargets), numRtsToSet, myDepthStencilTarget, framebufferRes, renderPass);

    myRenderPass = renderPass;
    myFramebuffer = framebuffer;
    myFramebufferRes = framebufferRes;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyGraphicsPipelineState()
  {
    if (!myGraphicsPipelineState.myIsDirty)
      return;

    myGraphicsPipelineState.myIsDirty = false;

    VkPipeline pipeline = RenderCore::GetPlatformVk()->GetPipelineStateCache().GetCreateGraphicsPipeline(myGraphicsPipelineState, myRenderPass);
    
    vkCmdBindPipeline(myCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyComputePipelineState()
  {
    if (!myComputePipelineState.myIsDirty)
      return;

    myComputePipelineState.myIsDirty = false;

    VkPipeline pipeline = RenderCore::GetPlatformVk()->GetPipelineStateCache().GetCreateComputePipeline(myComputePipelineState);

    vkCmdBindPipeline(myCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
  }
//---------------------------------------------------------------------------//
  void CommandListVk::ApplyResourceState()
  {
    if (myPipelineLayout == nullptr || !myPipelineLayoutBindings->myIsDirty || myPipelineLayoutBindings->myDescriptorSets.empty())
      return;

    myPipelineLayoutBindings->myIsDirty = false;

    ASSERT(myCurrentContext != CommandListType::Graphics || myGraphicsPipelineState.myShaderPipeline != nullptr);
    ASSERT(myCurrentContext != CommandListType::Compute || myComputePipelineState.myShaderPipeline != nullptr);

    const ShaderPipelineVk* pipeline = GetShaderPipeline();

    eastl::fixed_vector<VkWriteDescriptorSet, 64> writeInfos;
    eastl::fixed_vector<VkDescriptorSet, kVkMaxNumBoundDescriptorSets> descriptorSets;
    uint firstSet = UINT_MAX;
    for (uint iSet = 0u; iSet < (uint) myPipelineLayoutBindings->myDescriptorSets.size(); ++iSet)
    {
      const PipelineLayoutBindingsVk::DescriptorSet& set = myPipelineLayoutBindings->myDescriptorSets[iSet];
      if (!set.myIsDirty || !pipeline->HasDescriptorSet(iSet)) 
        continue;

      set.myIsDirty = false;

      ASSERT(!set.myBindings.empty(), "Shader pipeline is using descriptor set %d but nothing is bound", iSet);

      firstSet = glm::min(firstSet, iSet);

      VkDescriptorSet vkSet = CreateDescriptorSet(set.myLayout);
      descriptorSets.push_back(vkSet);

      VkWriteDescriptorSet baseWriteInfo = {};
      baseWriteInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      baseWriteInfo.dstSet = vkSet;
      
      for (uint iRange = 0u; iRange < (uint) set.myBindings.size(); ++iRange)
      {
        const PipelineLayoutBindingsVk::DescriptorBinding& range = set.myBindings[iRange];

        if (range.myType == VK_DESCRIPTOR_TYPE_MAX_ENUM || range.myData.empty())
          continue;

        VkWriteDescriptorSet writeInfo = baseWriteInfo;
        writeInfo.descriptorType = range.myType;
        writeInfo.dstArrayElement = 0u;
        writeInfo.descriptorCount = range.Size();
        writeInfo.dstBinding = iRange;
        writeInfo.pImageInfo = (VkDescriptorImageInfo*) range.myData.data();
        writeInfo.pBufferInfo = (VkDescriptorBufferInfo*)range.myData.data();
        writeInfo.pTexelBufferView = (VkBufferView*) range.myData.data();
        writeInfos.push_back(writeInfo);
      }
    }

    ASSERT(writeInfos.empty() == descriptorSets.empty());

    if (!writeInfos.empty())
    {
      vkUpdateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, (uint) writeInfos.size(), writeInfos.data(), 0u, nullptr);

      VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      switch (myCurrentContext)
      {
      case CommandListType::Graphics:
        bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        break;
      case CommandListType::Compute:
        bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
        break;
      default: ASSERT(false);
      }

      // TODO: Dynamic offsets might be needed for ring-allocated cbuffers
      const uint numDynamicOffsets = 0u;
      const uint* dynamicOffsets = nullptr;
      vkCmdBindDescriptorSets(GetCommandBuffer(), bindPoint, pipeline->GetPipelineLayout()->myPipelineLayout, firstSet, (uint) descriptorSets.size(), descriptorSets.data(), numDynamicOffsets, dynamicOffsets);
    }
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BindInternal(const ShaderResourceInfoVk& aResourceInfo, uint anArrayIndex,  VkBufferView aBufferView,
    VkBuffer aBuffer, uint64 aBufferOffset, uint64 aBufferSize, VkImageView anImageView, VkImageLayout anImageLayout, VkSampler aSampler)
  {
    ASSERT(myPipelineLayoutBindings != nullptr);
    ASSERT(aResourceInfo.myNumDescriptors > anArrayIndex);
    ASSERT(aResourceInfo.myDescriptorSet < (uint) myPipelineLayoutBindings->myDescriptorSets.size());

    const ShaderPipelineVk* shaderPipeline = GetShaderPipeline();
    ASSERT(shaderPipeline != nullptr);

    PipelineLayoutBindingsVk::DescriptorSet& set = myPipelineLayoutBindings->myDescriptorSets[aResourceInfo.myDescriptorSet];

    ASSERT(aResourceInfo.myBindingInSet < (uint)set.myBindings.size());
    PipelineLayoutBindingsVk::DescriptorBinding& range = set.myBindings[aResourceInfo.myBindingInSet];

    if (range.myIsUnbounded)
    {
      ASSERT(anArrayIndex < RenderCore_PlatformVk::MAX_DESCRIPTOR_ARRAY_SIZE);  // Currently we don't have a way to handle true unbounded arrays in Vulkan.
      const uint oldNumDescriptors = range.Size();
      range.ResizeUp(anArrayIndex + 1);
      memset(&range.myData[oldNumDescriptors * range.GetElementSize()], 0u, (range.Size() - oldNumDescriptors) * range.GetElementSize());
    }
    else
    {
      ASSERT(anArrayIndex < range.Size());
    }

    switch (aResourceInfo.myType)
    {
        case VK_DESCRIPTOR_TYPE_SAMPLER:
        {
          ASSERT(aSampler != nullptr);
          VkDescriptorImageInfo info;
          info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
          info.imageView = nullptr;
          info.sampler = aSampler;
          set.myIsDirty |= range.Set(info, anArrayIndex);
        } break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: ASSERT(false); break;  // Not supported in HLSL, so this should never happen
        case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
        case VK_DESCRIPTOR_TYPE_STORAGE_IMAGE:
        {
          ASSERT(anImageView != nullptr);
          VkDescriptorImageInfo info;
          info.imageLayout = anImageLayout;
          info.imageView = anImageView;
          info.sampler = nullptr;
          set.myIsDirty |= range.Set(info, anArrayIndex);
        } break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
        {
          ASSERT(aBufferView != nullptr);
          set.myIsDirty |= range.Set(aBufferView, anArrayIndex);
        } break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
        {
          ASSERT(aBuffer != nullptr && aBufferSize > 0ull);
          VkDescriptorBufferInfo info;
          info.buffer = aBuffer;
          info.offset = aBufferOffset;
          info.range = aBufferSize;
          set.myIsDirty |= range.Set(info, anArrayIndex);
        } break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
          ASSERT(false);  // TODO: Support dynamic uniform and storage buffers. 
          break;
        default: ASSERT(false);
    }

    myPipelineLayoutBindings->myIsDirty |= set.myIsDirty;
  }
//---------------------------------------------------------------------------//
  VkDescriptorSet CommandListVk::CreateDescriptorSet(VkDescriptorSetLayout aLayout)
  {
    if (myUsedDescriptorPools.empty())
      myUsedDescriptorPools.push_back(RenderCore::GetPlatformVk()->AllocateDescriptorPool());
    
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorSetCount = 1u;
    allocInfo.pSetLayouts = &aLayout;
    allocInfo.descriptorPool = myUsedDescriptorPools.back();

    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, &allocInfo, &descriptorSet);

    if (result == VK_SUCCESS)
      return descriptorSet;

    myUsedDescriptorPools.push_back(RenderCore::GetPlatformVk()->AllocateDescriptorPool());
    allocInfo.descriptorPool = myUsedDescriptorPools.back();
    result = vkAllocateDescriptorSets(RenderCore::GetPlatformVk()->myDevice, &allocInfo, &descriptorSet);

    ASSERT(result == VK_SUCCESS);
    return descriptorSet;
  }
//---------------------------------------------------------------------------//
  void CommandListVk::BeginCommandBuffer()
  {
    VkCommandBufferBeginInfo info;
    info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    info.pNext = nullptr;
    info.pInheritanceInfo = nullptr;
    info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    ASSERT_VK_RESULT(vkBeginCommandBuffer(myCommandBuffer, &info));
  }
//---------------------------------------------------------------------------//
}

#endif 
