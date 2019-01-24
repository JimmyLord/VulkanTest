//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#ifndef __VulkanSwapchainObject_H__
#define __VulkanSwapchainObject_H__

#include "vulkan/vulkan.h"
class VulkanBuffer;

static const int MAX_SWAP_IMAGES = 3;

class SwapchainStuff
{
    friend class VulkanInterface;

protected:
    VkImage m_Images;
    VkImageView m_ImageViews;
    VkCommandBuffer m_CommandBuffers;
    VkFramebuffer m_Framebuffers;
    VulkanBuffer* m_UBO_Matrices;
    VkDescriptorSet m_DescriptorSets;

protected:
    void NullEverything();

public:
    SwapchainStuff();
    ~SwapchainStuff();
};

#endif //__VulkanSwapchainObject_H__