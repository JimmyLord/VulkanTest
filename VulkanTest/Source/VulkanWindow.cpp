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

#define VK_USE_PLATFORM_WIN32_KHR
#include "vulkan/vulkan.h"
#include "vulkan/vk_sdk_platform.h"
#include "vulkan/vulkan_win32.h"

#include "VulkanInterface.h"
#include "VulkanWindow.h"

VulkanWindow::VulkanWindow()
{
    m_hInstance = 0;
    m_hWnd = 0;
}

VulkanWindow::~VulkanWindow()
{
}

VkSurfaceKHR VulkanWindow::Create(VkInstance vulkanInstance, const char* windowTitle, int width, int height)
{
    // Create instance of window.
    {
        m_hInstance = GetModuleHandle( 0 );                      // Grab An Instance For Our Window
        if( m_hInstance == 0 )
        {
            MessageBox( 0, "GetModuleHandle failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            return VK_NULL_HANDLE;
        }
    }

    // Register a window class.
    {
        WNDCLASS windowClass = {};
        windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Redraw On Move, And Own DC For Window
        windowClass.lpfnWndProc = (WNDPROC)WndProc;              // WndProc Handles Messages
        windowClass.cbClsExtra = 0;                              // No Extra Window Data
        windowClass.cbWndExtra = 0;                              // No Extra Window Data
        windowClass.hInstance = m_hInstance;                     // Set The Instance
        windowClass.hIcon = LoadIcon( 0, IDI_WINLOGO );          // Load The Default Icon
        windowClass.hCursor = LoadCursor( 0, IDC_ARROW );        // Load The Arrow Pointer
        windowClass.hbrBackground = 0;                           // No Background Required For GL
        windowClass.lpszMenuName = 0;                            // We Don't Want A Menu
        windowClass.lpszClassName = "VulkanWindowClass";         // Set The Class Name

        if( !RegisterClass( &windowClass ) )
        {
            Destroy();
            MessageBox( 0, "Failed to register the window class.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            return VK_NULL_HANDLE;
        }
    }

    // Create a visible window.
    {
        RECT WindowRect;
        WindowRect.left = (long)0;
        WindowRect.right = (long)width;
        WindowRect.top = (long)0;
        WindowRect.bottom = (long)height;

        DWORD dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        DWORD dwStyle = WS_OVERLAPPEDWINDOW;

        AdjustWindowRectEx( &WindowRect, dwStyle, false, dwExStyle );  // Adjust Window To True Requested Size

        m_hWnd = CreateWindowEx( dwExStyle,                            // Extended Style For The Window
                                 "VulkanWindowClass",                  // Class Name
                                 windowTitle,                          // Window Title
                                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN |   // Required Window Style
                                     dwStyle,                          // Selected Window Style
                                 0, 0,                                 // Window Position
                                 WindowRect.right-WindowRect.left,     // Calculate Adjusted Window Width
                                 WindowRect.bottom-WindowRect.top,     // Calculate Adjusted Window Height
                                 0,                                    // No Parent Window
                                 0,                                    // No Menu
                                 m_hInstance,                          // Instance
                                 this );                               // Pass a pointer to this object to WM_NCCREATE.

        if( m_hWnd == 0 )
        {
            Destroy();
            MessageBox( 0, "Window creation error.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            return VK_NULL_HANDLE;
        }

        ShowWindow( m_hWnd, SW_SHOW );   // Show The Window
        SetForegroundWindow( m_hWnd );   // Slightly Higher Priority
        SetFocus( m_hWnd );              // Sets Keyboard Focus To The Window
    }

    // Create Vulkan Surface.
    {
        VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.pNext = NULL;
        surfaceCreateInfo.flags = 0;
        surfaceCreateInfo.hinstance = m_hInstance;
        surfaceCreateInfo.hwnd = m_hWnd;

        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkResult result = vkCreateWin32SurfaceKHR( vulkanInstance, &surfaceCreateInfo, nullptr, &surface );
        if( result != VK_SUCCESS )
        {
            MessageBox( 0, "Failed to create vulkan surface.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            assert( false );
        }

        return surface;
    }
}

void VulkanWindow::Destroy()
{
    if( m_hWnd && !DestroyWindow( m_hWnd ) )
    {
        MessageBox( 0, "Could not release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        m_hWnd = 0;
    }

    if( !UnregisterClass( "VulkanWindowClass", m_hInstance ) )
    {
        MessageBox( 0, "Could not unregister class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION );
        m_hInstance = 0;
    }
}

// This is a static method.
LRESULT CALLBACK VulkanWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Get a pointer to the VulkanWindow object associated with this window.
    VulkanWindow* pVulkanWindow = (VulkanWindow*)GetWindowLongPtr( hWnd, GWLP_USERDATA );

    switch( uMsg )
    {
    case WM_NCCREATE:
        {
            // Set the user data for this hWnd to the VulkanWindow* we passed in, used on first line of this method above.
            CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
            VulkanWindow* pVulkanWindow = (VulkanWindow*)pcs->lpCreateParams;
            SetWindowLongPtr( hWnd, GWLP_USERDATA, (LONG)pVulkanWindow );

            pVulkanWindow->m_hWnd = hWnd;
        }
        return 1;
    
    case WM_DESTROY:
        {
            pVulkanWindow->m_hWnd = nullptr;
        }
        return 0;

    case WM_CLOSE:
        {
            PostQuitMessage( 0 );
        }
        return 0;
    }

    // Pass all unhandled messages to DefWindowProc.
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
