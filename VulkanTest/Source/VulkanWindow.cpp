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
    assert( m_hInstance == 0 );
    assert( m_hWnd == 0 );

    // Create instance of window.
    {
        m_hInstance = GetModuleHandle( 0 ); // Grab an instance for our window.
        if( m_hInstance == 0 )
        {
            MessageBox( 0, "GetModuleHandle failed.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            return VK_NULL_HANDLE;
        }
    }

    // Register a window class.
    {
        WNDCLASS windowClass = {};
        windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Redraw on move, and own DC for window.
        windowClass.lpfnWndProc = (WNDPROC)StaticWndProc;        // WndProc handles messages.
        windowClass.cbClsExtra = 0;                              // No extra window data.
        windowClass.cbWndExtra = 0;                              // No extra window data.
        windowClass.hInstance = m_hInstance;                     // Set the instance.
        windowClass.hIcon = LoadIcon( 0, IDI_WINLOGO );          // Load the default icon.
        windowClass.hCursor = LoadCursor( 0, IDC_ARROW );        // Load the arrow pointer.
        windowClass.hbrBackground = 0;                           // No background required.
        windowClass.lpszMenuName = 0;                            // We don't want a menu.
        windowClass.lpszClassName = "VulkanWindowClass";         // Set the class name.

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

        AdjustWindowRectEx( &WindowRect, dwStyle, false, dwExStyle );  // Adjust window to true requested size.

        m_hWnd = CreateWindowEx( dwExStyle,                            // Extended style for the window.
                                 "VulkanWindowClass",                  // Class name.
                                 windowTitle,                          // Window title.
                                 WS_CLIPSIBLINGS | WS_CLIPCHILDREN |   // Required window style.
                                     dwStyle,                          // Selected window style.
                                 0, 0,                                 // Window position.
                                 WindowRect.right-WindowRect.left,     // Calculate adjusted window width.
                                 WindowRect.bottom-WindowRect.top,     // Calculate adjusted window height.
                                 0,                                    // No parent window.
                                 0,                                    // No menu.
                                 m_hInstance,                          // Instance.
                                 this );                               // Pass a pointer to this object to WM_NCCREATE.

        if( m_hWnd == 0 )
        {
            Destroy();
            MessageBox( 0, "Window creation error.", "ERROR", MB_OK|MB_ICONEXCLAMATION );
            return VK_NULL_HANDLE;
        }

        ShowWindow( m_hWnd, SW_SHOW );   // Show the window.
        SetForegroundWindow( m_hWnd );   // Slightly higher priority.
        SetFocus( m_hWnd );              // Sets keyboard focus to the window.
    }

    // Create Vulkan surface.
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
LRESULT CALLBACK VulkanWindow::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Get a pointer to the VulkanWindow object associated with this window, will be nullptr until WM_NCCREATE is processed.
    VulkanWindow* pVulkanWindow = (VulkanWindow*)GetWindowLongPtr( hWnd, GWLP_USERDATA );

    switch( uMsg )
    {
    case WM_NCCREATE:
        {
            // Set the user data for this hWnd to the VulkanWindow* we passed in, used on first line of this method above.
            CREATESTRUCT* pcs = (CREATESTRUCT*)lParam;
            pVulkanWindow = (VulkanWindow*)pcs->lpCreateParams;
            SetWindowLongPtr( hWnd, GWLP_USERDATA, (uintptr_t)pVulkanWindow );

            pVulkanWindow->m_hWnd = hWnd;
        }
        return 1;
    }

    if( pVulkanWindow == 0 )
        return 0;

    return pVulkanWindow->WndProc( hWnd, uMsg, wParam, lParam );
}
    
LRESULT CALLBACK VulkanWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch( uMsg )
    {
    case WM_DESTROY:
        {
            m_hWnd = nullptr;
        }
        return 0;

    case WM_CLOSE:
        {
            PostQuitMessage( 0 );
        }
        return 0;

    case WM_KEYDOWN:
        {
            if( wParam == VK_ESCAPE )
                PostQuitMessage( 0 );
        }
        return 0;
    }

    // Pass all unhandled messages to DefWindowProc.
    return DefWindowProc( hWnd, uMsg, wParam, lParam );
}
