//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VulkanBuffer_H__
#define __VulkanBuffer_H__

#include "vulkan/vulkan.h"
#include "VulkanBuffer.h"

class VulkanInterface;

struct VertexFormat
{
    float pos[2];
    unsigned char color[4];

    static VkVertexInputBindingDescription bindingDescription;
    static VkVertexInputAttributeDescription attributeDescriptions[2];

    static void SetupDescriptions()
    {
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof( VertexFormat );
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof( VertexFormat, pos );

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R8G8B8A8_UNORM;
        attributeDescriptions[1].offset = offsetof( VertexFormat, color );
    }

    static int GetBindingDescriptionCount() { return 1; }
    static VkVertexInputBindingDescription* getBindingDescription()
    {
        return &bindingDescription;
    }

    static int GetAttributeDescriptionCount() { return 2; }
    static VkVertexInputAttributeDescription* getAttributeDescriptions()
    {
        return attributeDescriptions;
    }
};

class VulkanBuffer
{
    friend class VulkanInterface;

protected:
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;

    VulkanInterface* m_pInterface;

protected:
    void BufferData(const void* pData, unsigned int sizeInBytes);

public:
    VulkanBuffer();
    virtual ~VulkanBuffer();

    void Create(VulkanInterface* pInterface);
    void Destroy();
};

#endif //__VulkanBuffer_H__
