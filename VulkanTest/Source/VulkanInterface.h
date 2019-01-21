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

class VulkanInterface
{
protected:
    VulkanWindow* m_Window;

    VkInstance m_VulkanInstance;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR m_Surface;
    VkQueue m_Queue;
    uint32_t m_GraphicsQueueFamilyIndex;
    //uint32_t m_PresentQueueFamilyIndex;

    VkCommandPool m_CommandBufferPool;

    VkSwapchainKHR m_Swapchain;
    VkImage m_SwapchainImages[3];
    VkImageView m_SwapchainImageViews[3];
    VkCommandBuffer m_SwapchainCommandBuffers[3];

    VkSemaphore m_ImageAcquiredSemaphore;
    VkSemaphore m_DrawCompleteSemaphore;

    uint32_t m_CurrentSwapchainImageIndex;

protected:
    void CreateInterface();
    void CreateSurface(const char* windowName, int width, int height);
    void CreateSwapchain();
    void CreateCommandBufferPool();
    void CreateSemaphores();

    VkCommandBuffer CreateCommandBuffer();
    void SetupClearScreenCommands();

public:
    VulkanInterface();
    ~VulkanInterface();

    void Create(const char* windowName, int width, int height);
    void Destroy();

    void Render();
    void Present();
};

#endif //__VulkanInterface_H__
