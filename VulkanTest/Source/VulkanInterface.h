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
#include "VulkanSwapchainObject.h"

#include "Math/MyTypes.h"

class VulkanShader;
class VulkanBuffer;
class VulkanMesh;

class VulkanInterface
{
    friend class VulkanBuffer;

protected:
    VulkanWindow* m_Window;
    VulkanShader* m_TempShader;
    VkDescriptorSetLayout m_UBODescriptorSetLayout;

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
    VkDescriptorPool m_DescriptorPool;

    uint32_t m_SurfaceWidth;
    uint32_t m_SurfaceHeight;

    VkSwapchainKHR m_Swapchain;
    uint32 m_SwapchainImageCount; // Currently hardcoded to 3.
    SwapchainStuff m_SwapchainStuff[3];
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

    void CreateDescriptorPool();
    void CreateDescriptorSets();

    void CreateSemaphores();
    void CreateRenderPassAndPipeline(VkDescriptorSetLayout uboLayout);

    VkDescriptorSetLayout CreateUBODescriptorSetLayout();
    VkCommandBuffer CreateCommandBuffer();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    VkDevice GetDevice() { return m_Device; }

public:
    VulkanInterface();
    virtual ~VulkanInterface();

    void Create(const char* windowName, int width, int height);
    void Destroy();

    void SetupCommandBuffers(VulkanMesh* pMesh);

    void Render();
    void Present();
};

#endif //__VulkanInterface_H__
