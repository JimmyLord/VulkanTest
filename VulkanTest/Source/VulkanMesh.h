//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VulkanMesh_H__
#define __VulkanMesh_H__

#include "vulkan/vulkan.h"

class VulkanInterface;
class VulkanBuffer;

class VulkanMesh
{
    friend class VulkanInterface;

protected:
    VulkanBuffer* m_VertexBuffer;
    VulkanBuffer* m_IndexBuffer;

    uint32 m_VertexCount;
    uint32 m_IndexCount;

public:
    VulkanMesh();
    virtual ~VulkanMesh();

    void Create(VulkanInterface* pInterface, const void* vertices, uint32 vertexCount, const void* indices, uint32 indexCount);
    void CreateCube(VulkanInterface* pInterface);
    void Destroy();

    VulkanBuffer* GetVertexBuffer() { return m_VertexBuffer; }
    VulkanBuffer* GetIndexBuffer() { return m_IndexBuffer; }
    uint32 GetVertexCount() { return m_VertexCount; }
    uint32 GetIndexCount() { return m_IndexCount; }
};

#endif //__VulkanMesh_H__
