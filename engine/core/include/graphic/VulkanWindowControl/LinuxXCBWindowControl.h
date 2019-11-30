#pragma once

#ifdef __linux__

#include <xcb/xcb.h>

#include "graphic/VulkanWindowControl/IVulkanWindowControl.h"

class LinuxXCBWindowControl : public IVulkanWindowControl
{
public:
    LinuxXCBWindowControl();
    ~LinuxXCBWindowControl();

    virtual void Init(uint32_t Width, uint32_t Height);

    virtual VkSurfaceKHR CreateSurface(VkInstance& inst);

private:

    xcb_connection_t* m_pXCBConn;
    xcb_screen_t* m_pXCBScreen;
    xcb_window_t m_xcbWindow;
};

#endif
