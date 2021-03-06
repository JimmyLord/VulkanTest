//
// Copyright (c) 2019 Jimmy Lord http://www.flatheadgames.com
//
// This software is provided 'as-is', without any express or implied warranty.  In no event will the authors be held liable for any damages arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include <Windows.h>
#include <assert.h>

#include "VulkanInterface.h"
#include "VulkanMesh.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    VulkanInterface* vulkanInterface = new VulkanInterface();
    vulkanInterface->Create( "Vulkan Test", 480, 270 );

    VulkanMesh* cube = new VulkanMesh();
    cube->CreateCube( vulkanInterface );

    vulkanInterface->SetupCommandBuffers( cube );

    MSG msg;
    bool running = true;
    while( running )
    {
        if( PeekMessage( &msg, nullptr, 0, 0, PM_REMOVE ) )
        {
            if( msg.message == WM_QUIT )
            {
                running = false;
            }
            else
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }
        else
        {
            vulkanInterface->Render();
            vulkanInterface->Present();
        }
    }

    cube->Destroy();
    delete cube;

    vulkanInterface->Destroy();
    delete vulkanInterface;
}
