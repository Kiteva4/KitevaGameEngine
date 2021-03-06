#ifdef __linux__

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#include "graphic/KGEVulkan.h"
#include "graphic/VulkanWindowControl/LinuxXCBWindowControl.h"
#include "vulkan/vulkan_xcb.h"

LinuxXCBWindowControl::LinuxXCBWindowControl()
{
    m_pXCBConn   = nullptr;
    m_pXCBScreen = nullptr;
}

LinuxXCBWindowControl::~LinuxXCBWindowControl()
{
    if (m_xcbWindow) {
        xcb_destroy_window(m_pXCBConn, m_xcbWindow);
    }

    if (m_pXCBConn) {
        xcb_disconnect(m_pXCBConn);
    }
}

void LinuxXCBWindowControl::Init(uint32_t Width, uint32_t Height)
{
    m_pXCBConn = xcb_connect(nullptr, nullptr);

    int error = xcb_connection_has_error(m_pXCBConn);

    if  (error) {
        throw std::runtime_error("Vulkan: Error opening xcb connection error %d\n");
    }

    printf("XCB connection opened\n");

    const xcb_setup_t* pSetup = xcb_get_setup(m_pXCBConn);
    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(pSetup);

    m_pXCBScreen = iter.data;

    printf("XCB screen %p\n", m_pXCBScreen);

    m_xcbWindow = xcb_generate_id(m_pXCBConn);

    xcb_create_window(m_pXCBConn,             // соединение с сервером XWindow
      XCB_COPY_FROM_PARENT,                   // глубина цвета
      m_xcbWindow,                            // XID нового окна
      m_pXCBScreen->root,                     // родительское окно
      0,                                      // координата X
      0,                                      // координата Y
      Width,                                  // ширина окна
      Height,                                 // высота окна
      0,                                      // ширина границы
      XCB_WINDOW_CLASS_INPUT_OUTPUT,          // класс окна, не смог найти документации
      m_pXCBScreen->root_visual,              // определяет отображения цвета
      0,
      nullptr);

    xcb_map_window(m_pXCBConn, m_xcbWindow);

    xcb_flush (m_pXCBConn);

    printf("Window %x created\n", m_xcbWindow);
}

VkSurfaceKHR LinuxXCBWindowControl::CreateSurface(VkInstance &vkInstance)
{

    VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
    surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    surfaceCreateInfo.connection = m_pXCBConn;
    surfaceCreateInfo.window = m_xcbWindow;

    VkSurfaceKHR surface;

    if(vkCreateXcbSurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &surface) != VK_SUCCESS){
        throw std::runtime_error("Vulkan: Error creating XCB Linux window!");
    }

    return surface;
}

#endif
