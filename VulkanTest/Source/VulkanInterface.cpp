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

#include "VulkanBuffer.h"
#include "VulkanInterface.h"
#include "VulkanMesh.h"
#include "VulkanShader.h"
#include "VulkanSwapchainObject.h"
#include "Structs.h"

VulkanInterface::VulkanInterface()
{
    m_SwapchainImageCount = 3;

    NullEverything();
}

VulkanInterface::~VulkanInterface()
{
}

void VulkanInterface::NullEverything()
{
    m_Window = nullptr;
    m_TempShader = nullptr;
    m_UBODescriptorSetLayout = VK_NULL_HANDLE;

    m_VulkanInstance = VK_NULL_HANDLE;
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
    m_Surface = VK_NULL_HANDLE;
    m_SurfaceFormat = VK_FORMAT_UNDEFINED;
    m_DepthFormat = VK_FORMAT_UNDEFINED;
    m_Queue = VK_NULL_HANDLE;
    m_GraphicsQueueFamilyIndex = UINT_MAX;
    //m_PresentQueueFamilyIndex = UINT_MAX;

    m_CommandBufferPool = VK_NULL_HANDLE;
    m_DescriptorPool = VK_NULL_HANDLE;

    m_SurfaceWidth = 0;
    m_SurfaceHeight = 0;

    m_Swapchain = VK_NULL_HANDLE;
    for( uint32 i=0; i<MAX_SWAP_IMAGES; i++ )
    {
        m_SwapchainStuff[i].NullEverything();
    }
    m_CurrentSwapchainImageIndex = UINT_MAX;

    m_ImageAcquiredSemaphore = VK_NULL_HANDLE;
    m_DrawCompleteSemaphore = VK_NULL_HANDLE;

    m_RenderPass = VK_NULL_HANDLE;
    m_Pipeline = VK_NULL_HANDLE;
    m_PipelineLayout = VK_NULL_HANDLE;
}

void VulkanInterface::Create(const char* windowName, int width, int height)
{
    assert( m_VulkanInstance == VK_NULL_HANDLE );
    assert( m_Window == nullptr );

    CreateInterface();
    CreateSurface( windowName, width, height );
    CreateSwapchain();
    CreateCommandBufferPool();
    CreateDescriptorPool();
    CreateSemaphores();


    m_UBODescriptorSetLayout = CreateUBODescriptorSetLayout();

    // Create UBOs for matrices.  One per swapchain image.
    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        m_SwapchainStuff[i].m_UBO_Matrices = new VulkanBuffer();
        m_SwapchainStuff[i].m_UBO_Matrices->Create( this, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, nullptr, sizeof( UniformBufferObject_Matrices ) );
    }

    CreateDescriptorSets();

    CreateRenderPassAndPipeline( m_UBODescriptorSetLayout );
}

void VulkanInterface::Destroy()
{
    // Destroy Vulkan objects.
    vkDestroyDescriptorSetLayout( m_Device, m_UBODescriptorSetLayout, nullptr );

    vkDestroyPipelineLayout( m_Device, m_PipelineLayout, nullptr );
    vkDestroyPipeline( m_Device, m_Pipeline, nullptr );
    vkDestroyRenderPass( m_Device, m_RenderPass, nullptr );

    vkDestroySemaphore( m_Device, m_ImageAcquiredSemaphore, nullptr );
    vkDestroySemaphore( m_Device, m_DrawCompleteSemaphore, nullptr );

    vkDestroySwapchainKHR( m_Device, m_Swapchain, nullptr );

    vkDestroyCommandPool( m_Device, m_CommandBufferPool, nullptr );
    vkDestroyDescriptorPool( m_Device, m_DescriptorPool, nullptr );

    for( uint32 i=0; i<MAX_SWAP_IMAGES; i++ )
    {
        vkDestroyImageView( m_Device, m_SwapchainStuff[i].m_ImageViews, nullptr );
        vkDestroyFramebuffer( m_Device, m_SwapchainStuff[i].m_Framebuffers, nullptr );
    }

    delete m_TempShader;
    for( uint32 i=0; i<MAX_SWAP_IMAGES; i++ )
    {
        delete m_SwapchainStuff[i].m_UBO_Matrices;
    }

    vkDestroyDevice( m_Device, nullptr );

    m_Window->Destroy();
    delete m_Window;

    vkDestroyInstance( m_VulkanInstance, nullptr );

    NullEverything();
}

int VulkanInterface::ChooseDevice(int deviceCount, VkPhysicalDevice* devices)
{
    // Default to the first physical device for now.
    return 0;
}

int VulkanInterface::ChooseGraphicsQueueFamily(int queueFamilyCount, VkQueueFamilyProperties* queueFamilyProperties)
{
    // Default to the first queue family for now.
    return 0;
}

int VulkanInterface::ChooseSurfaceFormat(int formatCount, VkSurfaceFormatKHR* surfaceFormats)
{
    // Default to the first surface format for now.
    return 0;
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

        // Choose a physical device.
        int deviceIndex = ChooseDevice( deviceCount, devices );
        m_PhysicalDevice = devices[deviceIndex];
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

        // Choose a queue family.
        m_GraphicsQueueFamilyIndex = ChooseGraphicsQueueFamily( queueFamilyCount, queueFamilyProperties );
        //m_PresentQueueFamilyIndex = m_GraphicsQueueFamilyIndex;
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

    m_SurfaceWidth = width;
    m_SurfaceHeight = height;

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

    // Choose a surface format.
    int surfaceFormatIndex = ChooseSurfaceFormat( formatCount, surfaceFormats );
    m_SurfaceFormat = surfaceFormats[surfaceFormatIndex].format;

    m_SurfaceWidth = surfaceCapabilities.currentExtent.width;
    m_SurfaceHeight = surfaceCapabilities.currentExtent.height;

    // Create the swapchain.
    VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = nullptr;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = m_Surface;
    swapchainCreateInfo.minImageCount = m_SwapchainImageCount; // Triple buffer.
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
    VkImage images[MAX_SWAP_IMAGES];
    result = vkGetSwapchainImagesKHR( m_Device, m_Swapchain, &m_SwapchainImageCount, images );
    assert( result == VK_SUCCESS );

    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        m_SwapchainStuff[i].m_Images = images[i];
    }

    // Create image views.
    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        VkImageViewCreateInfo imageViewCreateInfo = {};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = nullptr;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = m_SwapchainStuff[i].m_Images;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = surfaceFormats[surfaceFormatIndex].format; //VK_FORMAT_R8G8B8A8_UNORM;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView( m_Device, &imageViewCreateInfo, nullptr, &m_SwapchainStuff[i].m_ImageViews );
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

    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        m_SwapchainStuff[i].m_CommandBuffers = CreateCommandBuffer();
    }
}

void VulkanInterface::CreateDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = m_SwapchainImageCount;

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pNext = nullptr;
    poolInfo.flags = 0;
    poolInfo.maxSets = m_SwapchainImageCount;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    VkResult result = vkCreateDescriptorPool( m_Device, &poolInfo, nullptr, &m_DescriptorPool );
    assert( result == VK_SUCCESS );
}

void VulkanInterface::CreateDescriptorSets()
{
    VkDescriptorSetLayout layouts[3];
    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        layouts[i] = m_UBODescriptorSetLayout;
    }

    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = m_SwapchainImageCount;
    allocInfo.pSetLayouts = layouts;

    VkDescriptorSet descriptorSets[MAX_SWAP_IMAGES];
    VkResult result = vkAllocateDescriptorSets( m_Device, &allocInfo, descriptorSets );
    assert( result == VK_SUCCESS );

    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        m_SwapchainStuff[i].m_DescriptorSets = descriptorSets[i];
    }

    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_SwapchainStuff[i].m_UBO_Matrices->GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof( UniformBufferObject_Matrices );

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.pNext = nullptr;
        descriptorWrite.dstSet = m_SwapchainStuff[i].m_DescriptorSets;
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets( m_Device, 1, &descriptorWrite, 0, nullptr );
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

VkDescriptorSetLayout VulkanInterface::CreateUBODescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding layoutBinding = {};
    layoutBinding.binding = 0;
    layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layoutBinding.descriptorCount = 1;
    layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = nullptr;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = 1;
    layoutCreateInfo.pBindings = &layoutBinding;

    VkDescriptorSetLayout layout;
    VkResult result = vkCreateDescriptorSetLayout( m_Device, &layoutCreateInfo, nullptr, &layout );
    assert( result == VK_SUCCESS );

    return layout;
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

void VulkanInterface::CreateRenderPassAndPipeline(VkDescriptorSetLayout uboLayout)
{
    VkResult result;

    VkAttachmentReference colorAttachmentReference = {};
    VkAttachmentDescription colorAttachmentDescription = {};

    // Fill in color attachment reference and description.
    {
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentDescription.flags = 0;
        colorAttachmentDescription.format = m_SurfaceFormat;
        colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }

    //VkAttachmentReference depthAttachmentReference = {};
    //VkAttachmentDescription depthAttachmentDescription = {};

    //// Fill in depth attachment reference and description.
    //{
    //    depthAttachmentReference.attachment = 0;
    //    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //    depthAttachmentDescription.flags = 0;
    //    depthAttachmentDescription.format = m_DepthFormat;
    //    depthAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    //    depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    //    depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    //    depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    //    depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    //    depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    //    depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    //}

    // Create renderPass.
    {
        VkSubpassDescription subpassDescription = {};
        subpassDescription.flags = 0;
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.pResolveAttachments = nullptr;
        subpassDescription.pDepthStencilAttachment = nullptr; //&depthAttachmentReference;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;

        VkSubpassDependency subpassDependency = {};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependency.dependencyFlags = 0;

        VkRenderPassCreateInfo renderPassCreateInfo = {};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.pNext = nullptr;
        renderPassCreateInfo.flags = 0;
        renderPassCreateInfo.attachmentCount = 1;
        renderPassCreateInfo.pAttachments = &colorAttachmentDescription;
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;

        result = vkCreateRenderPass( m_Device, &renderPassCreateInfo, nullptr, &m_RenderPass );
        assert( result == VK_SUCCESS );
    }

    // Create framebuffers.
    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        VkFramebufferCreateInfo framebufferCreateInfo = {};
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext = nullptr;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.renderPass = m_RenderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &m_SwapchainStuff[i].m_ImageViews;
        framebufferCreateInfo.width = m_SurfaceWidth;
        framebufferCreateInfo.height = m_SurfaceHeight;
        framebufferCreateInfo.layers = 1;

        result = vkCreateFramebuffer( m_Device, &framebufferCreateInfo, nullptr, &m_SwapchainStuff[i].m_Framebuffers );
        assert( result == VK_SUCCESS );
    }

    // Create a temporary shader.
    {
        m_TempShader = new VulkanShader();
        m_TempShader->Create( m_Device, "Data/Shaders/spv.test.vs", "Data/Shaders/spv.test.fs" );
    }

    // Create a pipeline.
    {
        VkPipelineShaderStageCreateInfo shaderStageCreateInfoArray[2] = {};

        shaderStageCreateInfoArray[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfoArray[0].pNext = nullptr;
        shaderStageCreateInfoArray[0].flags = 0;
        shaderStageCreateInfoArray[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStageCreateInfoArray[0].module = m_TempShader->GetVertexShader();
        shaderStageCreateInfoArray[0].pName = "main";
        shaderStageCreateInfoArray[0].pSpecializationInfo = nullptr;

        shaderStageCreateInfoArray[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfoArray[1].pNext = nullptr;
        shaderStageCreateInfoArray[1].flags = 0;
        shaderStageCreateInfoArray[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStageCreateInfoArray[1].module = m_TempShader->GetFragmentShader();
        shaderStageCreateInfoArray[1].pName = "main";
        shaderStageCreateInfoArray[1].pSpecializationInfo = nullptr;

        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCreateInfo.pNext = nullptr;
        vertexInputStateCreateInfo.flags = 0;;
        //vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
        //vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr;
        //vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
        //vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr;
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = VertexFormat::GetBindingDescriptionCount();
        vertexInputStateCreateInfo.pVertexBindingDescriptions = VertexFormat::getBindingDescription();
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = VertexFormat::GetAttributeDescriptionCount();
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = VertexFormat::getAttributeDescriptions();

        VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.pNext = nullptr;
        inputAssemblyStateCreateInfo.flags = 0;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = false;

        VkViewport viewport = {};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width  = (float)m_SurfaceWidth;
        viewport.height = (float)m_SurfaceHeight;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissorRect = {};
        scissorRect.offset.x = 0;
        scissorRect.offset.y = 0;
        scissorRect.extent.width = m_SurfaceWidth;
        scissorRect.extent.height = m_SurfaceHeight;

        VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.pNext = nullptr;
        viewportStateCreateInfo.flags = 0;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &viewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &scissorRect;

        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.pNext = nullptr;
        rasterizationStateCreateInfo.flags = 0;
        rasterizationStateCreateInfo.depthClampEnable = false;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = false;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable = false;
        rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
        rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
        rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
        rasterizationStateCreateInfo.lineWidth = 1.0f;

        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.pNext = nullptr;
        multisampleStateCreateInfo.flags = 0;
        multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleStateCreateInfo.sampleShadingEnable = false;
        multisampleStateCreateInfo.minSampleShading = 1.0f;
        multisampleStateCreateInfo.pSampleMask = nullptr;
        multisampleStateCreateInfo.alphaToCoverageEnable = false;
        multisampleStateCreateInfo.alphaToOneEnable = false;

        VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {};
        depthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencilStateCreateInfo.pNext = nullptr;
        depthStencilStateCreateInfo.flags = 0;
        depthStencilStateCreateInfo.depthTestEnable = false;
        depthStencilStateCreateInfo.depthWriteEnable = false;
        depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencilStateCreateInfo.depthBoundsTestEnable = false;
        depthStencilStateCreateInfo.stencilTestEnable = false;
        depthStencilStateCreateInfo.front.failOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCreateInfo.front.passOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCreateInfo.front.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
        depthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
        depthStencilStateCreateInfo.minDepthBounds = 0.0f;
        depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

        VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
        colorBlendAttachmentState.blendEnable = false;
        colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.pNext = nullptr;
        colorBlendStateCreateInfo.flags = 0;
        colorBlendStateCreateInfo.logicOpEnable = false;
        colorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
        colorBlendStateCreateInfo.blendConstants[0] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[1] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[2] = 0.0f;
        colorBlendStateCreateInfo.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.pNext = nullptr;
        pipelineLayoutCreateInfo.flags = 0;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &uboLayout;
        pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
        pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

        result = vkCreatePipelineLayout( m_Device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayout );
        assert( result == VK_SUCCESS );

        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.pNext = nullptr;
        graphicsPipelineCreateInfo.flags = 0;
        graphicsPipelineCreateInfo.stageCount = 2;
        graphicsPipelineCreateInfo.pStages = shaderStageCreateInfoArray;
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
        graphicsPipelineCreateInfo.pTessellationState = nullptr;
        graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
        graphicsPipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
        graphicsPipelineCreateInfo.pDynamicState = nullptr;
        graphicsPipelineCreateInfo.layout = m_PipelineLayout;
        graphicsPipelineCreateInfo.renderPass = m_RenderPass;
        graphicsPipelineCreateInfo.subpass = VK_NULL_HANDLE;
        graphicsPipelineCreateInfo.basePipelineHandle;
        graphicsPipelineCreateInfo.basePipelineIndex = -1;

        result = vkCreateGraphicsPipelines( m_Device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &m_Pipeline );
        assert( result == VK_SUCCESS );
    }

    // Delete the shader.
    {
        m_TempShader->Destroy();
        delete m_TempShader;
        m_TempShader = nullptr;
    }
}

void VulkanInterface::SetupCommandBuffers(VulkanMesh* pMesh)
{
    VkCommandBufferBeginInfo bufferBeginInfo = {};
    bufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bufferBeginInfo.pNext = nullptr;
    bufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    bufferBeginInfo.pInheritanceInfo = nullptr;

    VkClearColorValue clearColor = { 0.0f, 0.0f, 0.3f, 1.0f };
    //VkClearDepthStencilValue clearDepth = { 1.0f, 0 };
    VkClearValue clearValue = {};
    clearValue.color = clearColor;
    //clearValue.depthStencil = clearDepth;

    VkImageSubresourceRange imageRange = {};
    imageRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    imageRange.baseMipLevel = 0;
    imageRange.levelCount = 1;
    imageRange.baseArrayLayer = 0;
    imageRange.layerCount = 1;

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.renderPass = m_RenderPass;
    //renderPassInfo.framebuffer = m_SwapchainStuff[i].m_Framebuffers; // Set in loop below.
    renderPassInfo.renderArea.offset.x = 0;
    renderPassInfo.renderArea.offset.y = 0;
    renderPassInfo.renderArea.extent.width = m_SurfaceWidth;
    renderPassInfo.renderArea.extent.height = m_SurfaceHeight;
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    for( uint32 i=0; i<m_SwapchainImageCount; i++ )
    {
        renderPassInfo.framebuffer = m_SwapchainStuff[i].m_Framebuffers;

        VkResult result = vkBeginCommandBuffer( m_SwapchainStuff[i].m_CommandBuffers, &bufferBeginInfo );
        assert( result == VK_SUCCESS );

        vkCmdBeginRenderPass( m_SwapchainStuff[i].m_CommandBuffers, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

        vkCmdBindPipeline( m_SwapchainStuff[i].m_CommandBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline );

        VkBuffer vertexBuffers[] = { pMesh->GetVertexBuffer()->m_Buffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers( m_SwapchainStuff[i].m_CommandBuffers, 0, 1, vertexBuffers, offsets );
        vkCmdBindIndexBuffer( m_SwapchainStuff[i].m_CommandBuffers, pMesh->GetIndexBuffer()->m_Buffer, 0, VK_INDEX_TYPE_UINT16 );

        vkCmdBindDescriptorSets( m_SwapchainStuff[i].m_CommandBuffers, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_SwapchainStuff[i].m_DescriptorSets, 0, nullptr );

        //vkCmdDraw( m_SwapchainStuff[i].m_CommandBuffers, drawCount, 1, 0, 0 );
        vkCmdDrawIndexed( m_SwapchainStuff[i].m_CommandBuffers, pMesh->GetIndexCount(), 1, 0, 0, 0 );

        vkCmdEndRenderPass( m_SwapchainStuff[i].m_CommandBuffers );
	
        result = vkEndCommandBuffer( m_SwapchainStuff[i].m_CommandBuffers );
        assert( result == VK_SUCCESS );
    }
}

uint32_t VulkanInterface::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties( m_PhysicalDevice, &memoryProperties );

    for( uint32_t i=0; i<memoryProperties.memoryTypeCount; i++ )
    {
        if( (typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties )
        {
            return i;
        }
    }

    assert( false );
    return UINT_MAX;
}

void VulkanInterface::Render()
{
    VkResult result = vkAcquireNextImageKHR( m_Device, m_Swapchain, UINT64_MAX, m_ImageAcquiredSemaphore, VK_NULL_HANDLE, &m_CurrentSwapchainImageIndex );
    assert( result == VK_SUCCESS );

    // Update our UBO.
    {
        static float frameCount = 0.0f;
        UniformBufferObject_Matrices matrices;
        matrices.m_World.CreateSRT( Vector3(1,1,1), Vector3(0,frameCount,frameCount/1.5f), Vector3(0,0,0) );
        matrices.m_View.CreateLookAtView( Vector3(0,0,-5), Vector3(0,1,0), Vector3(0,0,0) );
        matrices.m_Proj.CreatePerspectiveVFoV( 45.0f, (float)m_SurfaceWidth/m_SurfaceHeight, 0.01f, 100.0f );
        matrices.m_Proj.m22 *= -1; // Hack for vulkan clip-space being upside down. (-1,-1) at top left.
        frameCount += 1.0f;

        m_SwapchainStuff[m_CurrentSwapchainImageIndex].m_UBO_Matrices->BufferData( &matrices, sizeof( UniformBufferObject_Matrices ) );
    }

    VkSemaphore waitSemaphores[] = { m_ImageAcquiredSemaphore };
    VkSemaphore signalSemaphores[] = { m_DrawCompleteSemaphore };

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_SwapchainStuff[m_CurrentSwapchainImageIndex].m_CommandBuffers;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    
    result = vkQueueSubmit( m_Queue, 1, &submitInfo, VK_NULL_HANDLE );    
    assert( result == VK_SUCCESS );
}

void VulkanInterface::Present()
{
    VkSemaphore waitSemaphores[1] = { m_DrawCompleteSemaphore };

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
