//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VulkanInterface_H__
#define __VulkanInterface_H__

#include "vulkan/vulkan.h"
#include "VulkanWindow.h"

class VulkanShader;
class VulkanBuffer;

class VulkanInterface
{
    friend class VulkanBuffer;

protected:
    VulkanWindow* m_Window;
    VulkanShader* m_TempShader;
    VulkanBuffer* m_TriangleBuffer;
    VkDescriptorSetLayout m_UBODescriptorSet;
    VulkanBuffer* m_UniformBuffer_Matrices[3];

    VkInstance m_VulkanInstance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VkFormat m_SurfaceFormat;
    VkFormat m_DepthFormat;
    VkQueue m_Queue;
    uint32_t m_GraphicsQueueFamilyIndex;
    //uint32_t m_PresentQueueFamilyIndex;

    VkCommandPool m_CommandBufferPool;

    uint32_t m_SurfaceWidth;
    uint32_t m_SurfaceHeight;

    VkSwapchainKHR m_Swapchain;
    VkImage m_SwapchainImages[3];
    VkImageView m_SwapchainImageViews[3];
    VkCommandBuffer m_SwapchainCommandBuffers[3];
    VkFramebuffer m_Framebuffers[3];
    uint32_t m_CurrentSwapchainImageIndex;

    VkSemaphore m_ImageAcquiredSemaphore;
    VkSemaphore m_DrawCompleteSemaphore;

    VkRenderPass m_RenderPass;
    VkPipeline m_Pipeline;
    VkPipelineLayout m_PipelineLayout;

protected:
    virtual int ChooseDevice(int deviceCount, VkPhysicalDevice* devices);
    virtual int ChooseGraphicsQueueFamily(int queueFamilyCount, VkQueueFamilyProperties* queueFamilyProperties);
    virtual int ChooseSurfaceFormat(int formatCount, VkSurfaceFormatKHR* surfaceFormats);

    void NullEverything();

    void CreateInterface();
    void CreateSurface(const char* windowName, int width, int height);
    void CreateSwapchain();
    void CreateCommandBufferPool();
    void CreateSemaphores();
    void CreateRenderPassAndPipeline(VkDescriptorSetLayout uboLayout);

    VkDescriptorSetLayout CreateUBODescriptorSetLayout();
    VkCommandBuffer CreateCommandBuffer();
    void SetupCommandBuffers();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkDevice GetDevice() { return m_Device; }

public:
    VulkanInterface();
    virtual ~VulkanInterface();

    void Create(const char* windowName, int width, int height);
    void Destroy();

    void Render();
    void Present();
};

#endif //__VulkanInterface_H__
