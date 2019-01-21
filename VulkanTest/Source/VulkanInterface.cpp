//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <assert.h>
#include <limits.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vk_sdk_platform.h"

#include "VulkanInterface.h"

VulkanInterface::VulkanInterface()
{
    m_Window = nullptr;

    m_VulkanInstance = VK_NULL_HANDLE;
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
    m_Surface = VK_NULL_HANDLE;
    m_Queue = VK_NULL_HANDLE;
    m_GraphicsQueueFamilyIndex = UINT_MAX;
    //m_PresentQueueFamilyIndex = UINT_MAX;

    m_CommandBufferPool = VK_NULL_HANDLE;

    m_Swapchain = VK_NULL_HANDLE;
    for( int i=0; i<3; i++ )
    {
        m_SwapchainImages[i] = VK_NULL_HANDLE;
        m_SwapchainImageViews[i] = VK_NULL_HANDLE;
        m_SwapchainCommandBuffers[i] = VK_NULL_HANDLE;
    }

    m_ImageAcquiredSemaphore = VK_NULL_HANDLE;
    m_DrawCompleteSemaphore = VK_NULL_HANDLE;

    m_CurrentSwapchainImageIndex = UINT_MAX;
}

VulkanInterface::~VulkanInterface()
{
}

void VulkanInterface::Create(const char* windowName, int width, int height)
{
    assert( m_VulkanInstance == VK_NULL_HANDLE );
    assert( m_Window == nullptr );

    CreateInterface();
    CreateSurface( windowName, width, height );
    CreateSwapchain();
    CreateCommandBufferPool();
    CreateSemaphores();

    SetupClearScreenCommands();
}

void VulkanInterface::Destroy()
{
    // Destroy Vulkan objects.
    vkDestroySemaphore( m_Device, m_ImageAcquiredSemaphore, nullptr );
    vkDestroySemaphore( m_Device, m_DrawCompleteSemaphore, nullptr );

    vkDestroySwapchainKHR( m_Device, m_Swapchain, nullptr );

    vkDestroyCommandPool( m_Device, m_CommandBufferPool, nullptr );

    vkDestroyDevice( m_Device, nullptr );

    m_Window->Destroy();
    delete m_Window;

    vkDestroyInstance( m_VulkanInstance, nullptr );

    // Null everything.
    m_Window = nullptr;

    m_VulkanInstance = VK_NULL_HANDLE;
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
    m_Surface = VK_NULL_HANDLE;
    m_Queue = VK_NULL_HANDLE;
    m_GraphicsQueueFamilyIndex = UINT_MAX;
    //m_PresentQueueFamilyIndex = UINT_MAX;

    m_CommandBufferPool = VK_NULL_HANDLE;

    m_Swapchain = VK_NULL_HANDLE;
    for( int i=0; i<3; i++ )
    {
        m_SwapchainImages[i] = VK_NULL_HANDLE;
        m_SwapchainImageViews[i] = VK_NULL_HANDLE;
        m_SwapchainCommandBuffers[i] = VK_NULL_HANDLE;
    }

    m_ImageAcquiredSemaphore = VK_NULL_HANDLE;
    m_DrawCompleteSemaphore = VK_NULL_HANDLE;

    m_CurrentSwapchainImageIndex = UINT_MAX;
}

void VulkanInterface::CreateInterface()
{
    VkResult result;

    // Create instance.
    {
        // Setup application info struct.
        VkApplicationInfo applicationInfo = {};       
        applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        applicationInfo.pNext = nullptr;
        applicationInfo.pApplicationName = "My Vulkan Test Project";
        applicationInfo.applicationVersion = 1;
        applicationInfo.pEngineName = nullptr;
        applicationInfo.engineVersion = 1;
        applicationInfo.apiVersion = VK_API_VERSION_1_0;

        // Setup extensions.
        const int extensionCount = 3;
        const char* extensionList[extensionCount] =
        {
            VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
        };

        // Setup validation layer.
        const int layerCount = 1;
        const char* layerList[layerCount] =
        {
            "VK_LAYER_LUNARG_standard_validation"
        };

        // Setup instance creation info struct, using structs/lists setup above.
        VkInstanceCreateInfo instanceCreateInfo = {};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pNext = nullptr;
        instanceCreateInfo.flags = 0;
        instanceCreateInfo.pApplicationInfo = &applicationInfo;
        instanceCreateInfo.enabledLayerCount = layerCount;
        instanceCreateInfo.ppEnabledLayerNames = layerList;
        instanceCreateInfo.enabledExtensionCount = extensionCount;
        instanceCreateInfo.ppEnabledExtensionNames = extensionList;  

        result = vkCreateInstance( &instanceCreateInfo, nullptr, &m_VulkanInstance );
        assert( result == VK_SUCCESS );
    }

    // Enumerate physical devices.
    {
        // Get count.
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices( m_VulkanInstance, &deviceCount, nullptr );
        assert( deviceCount > 0 );

        // Limit count to 10, for now.
        if( deviceCount > 10 )
            deviceCount = 10;
        VkPhysicalDevice devices[10];

        // Get list.
        vkEnumeratePhysicalDevices( m_VulkanInstance, &deviceCount, devices );

        // Default to the first physical device for now.
        m_PhysicalDevice = devices[0];
    }

    // Get physical device properties and features, not used for anything yet.
    {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties( m_PhysicalDevice, &deviceProperties );

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures( m_PhysicalDevice, &deviceFeatures );
    }

    // Enumerate queue families.
    {
        // Get count.
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties( m_PhysicalDevice, &queueFamilyCount, nullptr );

        // Limit count to 128, for now.
        if( queueFamilyCount > 128 )
            queueFamilyCount = 128;
        VkQueueFamilyProperties queueFamilyProperties[128];

        // Get list.
        vkGetPhysicalDeviceQueueFamilyProperties( m_PhysicalDevice, &queueFamilyCount, queueFamilyProperties );

        // Default to the first queue family for now.
        m_GraphicsQueueFamilyIndex = 0;
        //m_PresentQueueFamilyIndex = 0;
    }

    // Create logical device.
    {
        float queuePriorities = 0.0f;
        VkDeviceQueueCreateInfo queueInfo = {};
        queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueInfo.pNext = nullptr;
        queueInfo.flags = 0;
        queueInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
        queueInfo.queueCount = 1;
        queueInfo.pQueuePriorities = &queuePriorities;
    
        const int deviceExtensionCount = 1;
        const char* pDeviceExtensions[deviceExtensionCount] =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };
    
        VkDeviceCreateInfo deviceCreateInfo = {};
        deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        deviceCreateInfo.pNext = nullptr;
        deviceCreateInfo.flags = 0;
        deviceCreateInfo.queueCreateInfoCount = 1;
        deviceCreateInfo.pQueueCreateInfos = &queueInfo;
        deviceCreateInfo.enabledLayerCount = 0;
        deviceCreateInfo.ppEnabledLayerNames = nullptr;
        deviceCreateInfo.enabledExtensionCount = deviceExtensionCount;
        deviceCreateInfo.ppEnabledExtensionNames = pDeviceExtensions;
        deviceCreateInfo.pEnabledFeatures = nullptr;
       
        VkResult result = vkCreateDevice( m_PhysicalDevice, &deviceCreateInfo, nullptr, &m_Device );
        assert( result == VK_SUCCESS );
    }

    // Get the Device Queue for selected family.
    {
        uint32_t deviceQueueIndex = 0;
        vkGetDeviceQueue( m_Device, m_GraphicsQueueFamilyIndex, deviceQueueIndex, &m_Queue );
    }
}

void VulkanInterface::CreateSurface(const char* windowName, int width, int height)
{
    assert( m_Window == nullptr );

    // Create window.
    m_Window = new VulkanWindow();
    m_Surface = m_Window->Create( m_VulkanInstance, windowName, width, height );
    assert( m_Surface != VK_NULL_HANDLE );

    // Verify surface is valid.
    VkBool32 supported;
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR( m_PhysicalDevice, m_GraphicsQueueFamilyIndex, m_Surface, &supported );
    assert( result == VK_SUCCESS );
    assert( supported == 1 );
}

void VulkanInterface::CreateSwapchain()
{
    assert( m_Window != nullptr );

    VkResult result;

    // Get Surface caps.
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_PhysicalDevice, m_Surface, &surfaceCapabilities );

    // Get list of surface formats.
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_PhysicalDevice, m_Surface, &formatCount, nullptr );
    if( formatCount > 128 )
        formatCount = 128;
    VkSurfaceFormatKHR surfaceFormats[128];
    vkGetPhysicalDeviceSurfaceFormatsKHR( m_PhysicalDevice, m_Surface, &formatCount, surfaceFormats );

    // Default to the first surface format for now.
    int surfaceFormatIndex = 0;

    // Create the swapchain.
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = m_Surface;
    swapchainCreateInfo.minImageCount = 3; // Triple buffer.
    swapchainCreateInfo.imageFormat = surfaceFormats[surfaceFormatIndex].format; //VK_FORMAT_R8G8B8A8_UNORM;
    swapchainCreateInfo.imageColorSpace = surfaceFormats[surfaceFormatIndex].colorSpace; //VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageExtent = surfaceCapabilities.currentExtent;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = nullptr;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform; //VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.clipped = true;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    result = vkCreateSwapchainKHR( m_Device, &swapchainCreateInfo, nullptr, &m_Swapchain );
    assert( result == VK_SUCCESS );

    // Get swapchain images.
    uint32_t swapchainImageCount = 3;
    result = vkGetSwapchainImagesKHR( m_Device, m_Swapchain, &swapchainImageCount, m_SwapchainImages );
    assert( result == VK_SUCCESS );

    // Create image views.
    for( int i=0; i<3; i++ )
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = m_SwapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = surfaceFormats[0].format; //VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView( m_Device, &imageViewCreateInfo, nullptr, &m_SwapchainImageViews[i] );
        assert( result == VK_SUCCESS );
    }
}

void VulkanInterface::CreateCommandBufferPool()
{
    VkCommandPoolCreateInfo commandPoolCreateInfo = {};
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = m_GraphicsQueueFamilyIndex;
    
    VkResult result = vkCreateCommandPool( m_Device, &commandPoolCreateInfo, nullptr, &m_CommandBufferPool );    
    assert( result == VK_SUCCESS );

    for( int i=0; i<3; i++ )
    {
        m_SwapchainCommandBuffers[i] = CreateCommandBuffer();
    }
}

void VulkanInterface::CreateSemaphores()
{
    VkResult result;

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreInfo.pNext = nullptr;
    semaphoreInfo.flags = 0;

    result = vkCreateSemaphore( m_Device, &semaphoreInfo, nullptr, &m_ImageAcquiredSemaphore );
    assert( result == VK_SUCCESS );

    result = vkCreateSemaphore( m_Device, &semaphoreInfo, nullptr, &m_DrawCompleteSemaphore );
    assert( result == VK_SUCCESS );
}

VkCommandBuffer VulkanInterface::CreateCommandBuffer()
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};

    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = nullptr;
    commandBufferAllocateInfo.commandPool = m_CommandBufferPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;

    VkResult result = vkAllocateCommandBuffers( m_Device, &commandBufferAllocateInfo, &commandBuffer );            
    assert( result == VK_SUCCESS );

    return commandBuffer;
}

void VulkanInterface::SetupClearScreenCommands()
{
    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.pNext = nullptr;
    bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    bufferBeginInfo.pInheritanceInfo = nullptr;

    VkClearColorValue clearColor = { 0.0f, 0.0f, 0.3f, 1.0f };
    //VkClearDepthStencilValue clearDepth = { 1.0f, 0 };
    //VkClearValue clearValue = {};
    //clearValue.color = clearColor;
    //clearValue.depthStencil = clearDepth;

    VkImageSubresourceRange imageRange = {};
    imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageRange.baseMipLevel = 0;
    imageRange.levelCount = 1;
    imageRange.baseArrayLayer = 0;
    imageRange.layerCount = 1;

    for( int i=0; i<3; i++ )
    {
        VkResult result = vkBeginCommandBuffer( m_SwapchainCommandBuffers[i], &bufferBeginInfo );
        assert( result == VK_SUCCESS );

        vkCmdClearColorImage( m_SwapchainCommandBuffers[i], m_SwapchainImages[i], VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &imageRange);

        result = vkEndCommandBuffer( m_SwapchainCommandBuffers[i] );
        assert( result == VK_SUCCESS );
    }
}

void VulkanInterface::Render()
{
    VkResult result = vkAcquireNextImageKHR( m_Device, m_Swapchain, UINT64_MAX, m_ImageAcquiredSemaphore, VK_NULL_HANDLE, &m_CurrentSwapchainImageIndex );
    assert( result == VK_SUCCESS );

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_SwapchainCommandBuffers[m_CurrentSwapchainImageIndex];
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    
    result = vkQueueSubmit( m_Queue, 1, &submitInfo, VK_NULL_HANDLE );    
    assert( result == VK_SUCCESS );
}

void VulkanInterface::Present()
{
    VkSemaphore waitSemaphores[1] = { m_ImageAcquiredSemaphore };

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = waitSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &m_Swapchain;
    presentInfo.pImageIndices = &m_CurrentSwapchainImageIndex;
    presentInfo.pResults = nullptr;
    
    VkResult result = vkQueuePresentKHR( m_Queue, &presentInfo );    
    assert( result == VK_SUCCESS );
}
