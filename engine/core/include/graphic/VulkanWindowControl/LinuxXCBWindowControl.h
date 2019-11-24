#pragma once

#ifdef __linux__

#include <xcb/xcb.h>

#include "graphic/VulkanWindowControl/IVulkanWindowControl.h"

class LinuxXCBWindowControl : public IVulkanWindowControl
{
public:
    LinuxXCBWindowControl();
    ~LinuxXCBWindowControl();

    virtual void Init(unsigned int Width, unsigned int Height);

    virtual VkSurfaceKHR CreateSurface(VkInstance& inst);

private:

    xcb_connection_t* m_pXCBConn;
    xcb_screen_t* m_pXCBScreen;
    xcb_window_t m_xcbWindow;
};

#endif
