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
#include "vulkan/vulkan.h"

#include "VulkanShader.h"

char* LoadCompleteFile(const char* filename, long* length)
{
    char* filecontents = 0;

    FILE* filehandle;
    errno_t error = fopen_s( &filehandle, filename, "rb" );

    if( filehandle )
    {
        fseek( filehandle, 0, SEEK_END );
        long size = ftell( filehandle );
        rewind( filehandle );

        filecontents = new char[size+1];
        fread( filecontents, size, 1, filehandle );
        filecontents[size] = 0;

        if( length )
            *length = size;

        fclose( filehandle );
    }

    return filecontents;
}

VulkanShader::VulkanShader()
{
    m_VertexShader = VK_NULL_HANDLE;
    m_FragmentShader = VK_NULL_HANDLE;

    m_Device = VK_NULL_HANDLE;
}

VulkanShader::~VulkanShader()
{
}

void VulkanShader::Create(VkDevice device, const char* vertexShaderFilename, const char* fragmentShaderFilename)
{
    assert( m_Device == VK_NULL_HANDLE );

    m_Device = device;

    long vertStringLength;
    const char* vertexShaderString = LoadCompleteFile( vertexShaderFilename, &vertStringLength );
    assert( (uintptr_t)vertexShaderString % 4 == 0 );

    long fragStringLength;
    const char* fragmentShaderString = LoadCompleteFile( fragmentShaderFilename, &fragStringLength );
    assert( (uintptr_t)fragmentShaderString % 4 == 0 );

    m_VertexShader = CreateShader( vertexShaderString, vertStringLength );
    m_FragmentShader = CreateShader( fragmentShaderString, fragStringLength );
}

void VulkanShader::Destroy()
{
    vkDestroyShaderModule( m_Device, m_VertexShader, nullptr );
    vkDestroyShaderModule( m_Device, m_FragmentShader, nullptr );

    m_VertexShader = VK_NULL_HANDLE;
    m_FragmentShader = VK_NULL_HANDLE;

    m_Device = VK_NULL_HANDLE;
}

VkShaderModule VulkanShader::CreateShader(const char* string, const size_t stringLength)
{
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pNext = nullptr;
    shaderModuleCreateInfo.flags = 0;
    shaderModuleCreateInfo.codeSize = stringLength;
    shaderModuleCreateInfo.pCode = (const uint32_t*)string;

    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule( m_Device, &shaderModuleCreateInfo, nullptr, &shaderModule );
    assert( result == VK_SUCCESS );

    return shaderModule;
}
