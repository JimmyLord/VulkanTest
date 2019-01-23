//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <assert.h>
#include <stdio.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vk_sdk_platform.h"

#include "Structs.h"
#include "VulkanBuffer.h"
#include "VulkanInterface.h"

VkVertexInputBindingDescription VertexFormat::bindingDescription = {};
VkVertexInputAttributeDescription VertexFormat::attributeDescriptions[2] = {};

VulkanBuffer::VulkanBuffer()
{
    m_Buffer = VK_NULL_HANDLE;
    m_BufferMemory = VK_NULL_HANDLE;

    m_pInterface = nullptr;

    VertexFormat::SetupDescriptions();
}

VulkanBuffer::~VulkanBuffer()
{
}

void VulkanBuffer::Create(VulkanInterface* pInterface, VkBufferUsageFlags usageFlags, const void* pData, unsigned int sizeInBytes)
{
    assert( m_Buffer == VK_NULL_HANDLE );
    assert( pInterface != nullptr );

    m_pInterface = pInterface;

    VkDevice device = m_pInterface->GetDevice();

    // Create a buffer.
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.pNext = nullptr;
        bufferInfo.flags = 0;
        bufferInfo.size = sizeInBytes;
        bufferInfo.usage = usageFlags;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = nullptr;

        VkResult result = vkCreateBuffer( device, &bufferInfo, nullptr, &m_Buffer );
        assert( result == VK_SUCCESS );
    }

    // Allocate memory for buffer.
    {
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements( device, m_Buffer, &memoryRequirements );

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext = nullptr;
        allocInfo.allocationSize = memoryRequirements.size;
        allocInfo.memoryTypeIndex = m_pInterface->FindMemoryType( memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );

        VkResult result = vkAllocateMemory( device, &allocInfo, nullptr, &m_BufferMemory );
        assert( result == VK_SUCCESS );

        vkBindBufferMemory( device, m_Buffer, m_BufferMemory, 0 );
    }

    // Buffer data, if a data pointer was passed in.
    if( pData != nullptr )
    {
        BufferData( pData, sizeInBytes );
    }
}

void VulkanBuffer::Destroy()
{
    vkFreeMemory( m_pInterface->GetDevice(), m_BufferMemory, nullptr );
    vkDestroyBuffer( m_pInterface->GetDevice(), m_Buffer, nullptr );
    
    m_Buffer = VK_NULL_HANDLE;
    m_pInterface = nullptr;
}

void VulkanBuffer::BufferData(const void* pData, unsigned int sizeInBytes)
{
    assert( pData != nullptr );
    assert( m_pInterface != nullptr );

    VkDevice device = m_pInterface->GetDevice();

    void* data;
    vkMapMemory( device, m_BufferMemory, 0, sizeInBytes, 0, &data );
    memcpy( data, pData, sizeInBytes );
    vkUnmapMemory( device, m_BufferMemory );
}
