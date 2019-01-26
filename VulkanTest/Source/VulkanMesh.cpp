//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "vulkan/vulkan.h"
#include "Math/MyTypes.h"

#include "VulkanMesh.h"
#include "VulkanBuffer.h"
#include "Structs.h"

VulkanMesh::VulkanMesh()
{
    m_VertexBuffer = nullptr;
    m_IndexBuffer = nullptr;
}

VulkanMesh::~VulkanMesh()
{
    assert( m_VertexBuffer == nullptr );
    assert( m_IndexBuffer == nullptr );
}

void VulkanMesh::Create(VulkanInterface* pInterface, const void* vertices, uint32 vertexCount, const void* indices, uint32 indexCount)
{
    // Create a vertex and index buffer.
    m_VertexBuffer = new VulkanBuffer();
    m_IndexBuffer = new VulkanBuffer();

    // Copy all data into buffers.
    m_VertexBuffer->Create( pInterface, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vertices, sizeof( VertexFormat ) * m_VertexCount );
    m_IndexBuffer->Create( pInterface, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, indices, sizeof( unsigned short ) * m_IndexCount );
}

void VulkanMesh::CreateCube(VulkanInterface* pInterface)
{
    // Copy cube verts into a buffer.
    m_VertexCount = 24;
    m_IndexCount = 36;

    const VertexFormat vertices[] =
    {
        { {-1.0f, -1.0f, -1.0f}, {   0,   0, 255, 255 } }, // Front // BL
        { {-1.0f,  1.0f, -1.0f}, {   0,   0, 255, 255 } },          // TL
        { { 1.0f,  1.0f, -1.0f}, {   0,   0, 255, 255 } },          // TR
        { { 1.0f, -1.0f, -1.0f}, {   0,   0, 255, 255 } },          // BR

        { { 1.0f, -1.0f, -1.0f}, { 255,   0,   0, 255 } }, // Right Side // BL
        { { 1.0f,  1.0f, -1.0f}, { 255,   0,   0, 255 } },               // TL
        { { 1.0f,  1.0f,  1.0f}, { 255,   0,   0, 255 } },               // TR
        { { 1.0f, -1.0f,  1.0f}, { 255,   0,   0, 255 } },               // BR

        { { 1.0f, -1.0f,  1.0f}, {   0,   0, 128, 255 } }, // Back // BL
        { { 1.0f,  1.0f,  1.0f}, {   0,   0, 128, 255 } },         // TL
        { {-1.0f,  1.0f,  1.0f}, {   0,   0, 128, 255 } },         // TR
        { {-1.0f, -1.0f,  1.0f}, {   0,   0, 128, 255 } },         // BR

        { {-1.0f, -1.0f,  1.0f}, { 128,   0,   0, 255 } }, // Left Side // BL
        { {-1.0f,  1.0f,  1.0f}, { 128,   0,   0, 255 } },              // TL
        { {-1.0f,  1.0f, -1.0f}, { 128,   0,   0, 255 } },              // TR
        { {-1.0f, -1.0f, -1.0f}, { 128,   0,   0, 255 } },              // BR

        { {-1.0f,  1.0f, -1.0f}, {   0, 255,   0, 255 } }, // Top // BL
        { {-1.0f,  1.0f,  1.0f}, {   0, 255,   0, 255 } },        // TL
        { { 1.0f,  1.0f,  1.0f}, {   0, 255,   0, 255 } },        // TR
        { { 1.0f,  1.0f, -1.0f}, {   0, 255,   0, 255 } },        // BR

        { {-1.0f, -1.0f,  1.0f}, {   0, 128,   0, 255 } }, // Bottom // BL
        { {-1.0f, -1.0f, -1.0f}, {   0, 128,   0, 255 } },           // TL
        { { 1.0f, -1.0f, -1.0f}, {   0, 128,   0, 255 } },           // TR
        { { 1.0f, -1.0f,  1.0f}, {   0, 128,   0, 255 } },           // BR
    };

    const unsigned short indices[] =
    {
        0, 1, 2, 0, 2, 3,
        4, 5, 6, 4, 6, 7,
        8, 9,10, 8,10,11,
        12,13,14,12,14,15,
        16,17,18,16,18,19,
        20,21,22,20,22,23,
    };

    Create( pInterface, vertices, m_VertexCount, indices, m_IndexCount );
}

void VulkanMesh::Destroy()
{
    delete m_VertexBuffer;
    delete m_IndexBuffer;

    m_VertexBuffer = nullptr;
    m_IndexBuffer = nullptr;
}
