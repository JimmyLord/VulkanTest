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

class VulkanBuffer
{
    friend class VulkanInterface;

protected:
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;

    VulkanInterface* m_pInterface;

protected:

public:
    VulkanBuffer();
    virtual ~VulkanBuffer();

    void Create(VulkanInterface* pInterface, VkBufferUsageFlags usageFlags, const void* pData, unsigned int sizeInBytes);
    void Destroy();

    void BufferData(const void* pData, unsigned int sizeInBytes);

    VkBuffer GetBuffer() { return m_Buffer; }
};

#endif //__VulkanBuffer_H__
